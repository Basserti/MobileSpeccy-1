#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QKeyEvent>
#include <QMap>
#include <cstdint>
#include <stdint.h>

#include "screenwidget.h"
#include "businterface48.h"
#include "businterface128.h"


enum {
    CURSOR_IF = 0,
    KEMPSTON_IF = 1,
    SINCLAIR_IF2 =2,
    SINCLAIR_IF2_2 =3,
};

static constexpr int PAIR(int a, int b) { return a * 100 + b;}
static constexpr int FIRST(int v) { return v / 100;}
static constexpr int SECOND(int v) { return v % 100;}

static constexpr int ESC_SCANCODE = 1;
static constexpr int F12_SCANCODE = 88;

static constexpr int UP_SCANCODE = 328;
static constexpr int DOWN_SCANCODE = 336;
static constexpr int LEFT_SCANCODE = 331;
static constexpr int RIGHT_SCANCODE = 333;
static constexpr int UP2_SCANCODE = 72;
static constexpr int DOWN2_SCANCODE = 76;
static constexpr int LEFT2_SCANCODE = 75;
static constexpr int RIGHT2_SCANCODE = 77;
static constexpr int LCTRL_SCANCODE = 29;
static constexpr int RCTRL_SCANCODE = 285;





static const QMap<int, int> s_key_mapping
{
    { 2,  PAIR(0, 11) },
    { 3,  PAIR(1, 11) },
    { 4,  PAIR(2, 11) },
    { 5,  PAIR(3, 11) },
    { 6,  PAIR(4, 11) },
    { 7,  PAIR(4, 12) },
    { 8,  PAIR(3, 12) },
    { 9,  PAIR(2, 12) },
    { 10, PAIR(1, 12) },
    { 11, PAIR(0, 12) },

    { 16, PAIR(0, 10) },
    { 17, PAIR(1, 10) },
    { 18, PAIR(2, 10) },
    { 19, PAIR(3, 10) },
    { 20, PAIR(4, 10) },
    { 21, PAIR(4, 13) },
    { 22, PAIR(3, 13) },
    { 23, PAIR(2, 13) },
    { 24, PAIR(1, 13) },
    { 25, PAIR(0, 13) },

    { 30, PAIR(0, 9) },
    { 31, PAIR(1, 9) },
    { 32, PAIR(2, 9) },
    { 33, PAIR(3, 9) },
    { 34, PAIR(4, 9) },
    { 35, PAIR(4, 14) },
    { 36, PAIR(3, 14) },
    { 37, PAIR(2, 14) },
    { 38, PAIR(1, 14) },
    { 28, PAIR(0, 14) },

    { 42, PAIR(0, 8) },
    { 44, PAIR(1, 8) },
    { 45, PAIR(2, 8) },
    { 46, PAIR(3, 8) },
    { 47, PAIR(4, 8) },
    { 48, PAIR(4, 15) },
    { 49, PAIR(3, 15) },
    { 50, PAIR(2, 15) },
    { 54, PAIR(1, 15) },
    { 57, PAIR(0, 15) },

    {331, PAIR(4, 11) },
    {336, PAIR(4, 12) },
    {328, PAIR(3, 12) },
    {333, PAIR(2, 12) },
    { 29, PAIR(0, 12) },
};

static uint8_t s_mem_read(void *context, uint16_t address)
{
    BusInterface * bi =reinterpret_cast<BusInterface*>(context);
    return bi->mem_read8(address);
}

static void s_mem_write(void *context, uint16_t address, uint8_t value)
{
    BusInterface * bi =reinterpret_cast<BusInterface*>(context);
    bi->mem_write8(address, value);
}

static uint8_t s_port_read(void *context, uint16_t address)
{
    BusInterface * bi =reinterpret_cast<BusInterface*>(context);
    return bi->io_read8(address);
}

static void s_port_write(void *context, uint16_t address, uint8_t value)
{
    BusInterface * bi =reinterpret_cast<BusInterface*>(context);
    bi->io_write8(address, value);
}

static uint32_t s_int_data(void *context)
{
    Q_UNUSED(context);
    return 0xC3000000; // JP #0
}

static void s_halt(void *context, uint8_t state)
{
    Q_UNUSED(context);
    Q_UNUSED(state);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);

    ui->cbShowControls->setChecked(false);
    ui->twControls->setVisible(false);
    bus = new BusInterface128();
    ui->screen->setBusInterface(bus);

    connect(ui->keyboard,
            SIGNAL(key_pressed(int,int)),
            this,
            SLOT(on_key_pressed(int,int)));
    connect(ui->keyboard,
            SIGNAL(key_released(int,int)),
            this,
            SLOT(on_key_released(int,int)));
    connect(ui->btn_Reset,
            SIGNAL(clicked()),
            this,
            SLOT(reset()));

    connect(ui->pbUp,  SIGNAL(pressed()), this, SLOT(upPressed()));
    connect(ui->pbDown, SIGNAL(pressed()), this, SLOT(downPressed()));
    connect(ui->pbLeft, SIGNAL(pressed()), this, SLOT(leftPressed()));
    connect(ui->pbRight, SIGNAL(pressed()), this, SLOT(rightPressed()));
    connect(ui->pbFire,  SIGNAL(pressed()), this, SLOT(firePressed()));

    connect(ui->pbUp,  SIGNAL(released()), this, SLOT(upRelease()));
    connect(ui->pbDown, SIGNAL(released()), this, SLOT(downRelease()));
    connect(ui->pbLeft, SIGNAL(released()), this, SLOT(leftRelease()));
    connect(ui->pbRight, SIGNAL(released()), this, SLOT(rightRelease()));
    connect(ui->pbFire, SIGNAL(released()), this, SLOT(fireRelease()));


    bus->io_write8(0xfe, 1);

    cpustate.context = bus;
    cpustate.read = s_mem_read;
    cpustate.write = s_mem_write;
    cpustate.in = s_port_read;
    cpustate.out = s_port_write;
    cpustate.int_data = s_int_data;
    cpustate.halt = s_halt;

    reset();


    frame_timer = new QTimer();

    connect(    frame_timer,
                SIGNAL(timeout()),
                this,
                SLOT(frameRefresh()));
    frame_timer->start(1000/50);

    flash_timer = new QTimer();
    connect(    flash_timer,
                SIGNAL(timeout()),
                ui->screen,
                SLOT(toggleFlash()));
    flash_timer->start(320);
}

MainWindow::~MainWindow()
{
    delete flash_timer;
    delete frame_timer;
    delete ui;
}
#pragma pack(push, 1)
struct SNAHeader
{
    uint8_t I;
    uint16_t HL_, DE_,
             BC_, AF_;
    uint16_t HL, DE, BC , IY, IX;
    uint8_t IFF2;
    uint8_t R;
    uint16_t AF, SP;
    uint8_t IM;
    uint8_t BRD;
};
#pragma pack(pop)

void MainWindow::load_sna(const QString &filename)
{
    QFile sna(filename);

    if (sna.open(QIODevice::ReadOnly)) {
        QByteArray buffer;
        buffer = sna.readAll();
        SNAHeader * sna_hdr = reinterpret_cast<SNAHeader *>(buffer.data());
        uint8_t * sna_memory = reinterpret_cast<uint8_t *>(buffer.data()) +
                                sizeof(SNAHeader);
        cpustate.state.i = sna_hdr->I;
        cpustate.state.hl_.value_uint16 = sna_hdr->HL_;
        cpustate.state.de_.value_uint16 = sna_hdr->DE_;
        cpustate.state.bc_.value_uint16 = sna_hdr->BC_;
        cpustate.state.af_.value_uint16 = sna_hdr->AF_;
        cpustate.state.hl.value_uint16  = sna_hdr->HL;
        cpustate.state.de.value_uint16  = sna_hdr->DE;
        cpustate.state.bc.value_uint16  = sna_hdr->BC;
        cpustate.state.iy.value_uint16  = sna_hdr->IY;
        cpustate.state.ix.value_uint16  = sna_hdr->IX;
        cpustate.state.internal.iff2    = sna_hdr->IFF2 >> 2;
        cpustate.state.r                = sna_hdr->R;
        cpustate.state.af.value_uint16  = sna_hdr->AF;
        cpustate.state.sp               = sna_hdr->SP;
        cpustate.state.internal.im      = sna_hdr->IM;
        bus->io_write8(0xfe, sna_hdr->BRD);
        for (int off = 0; off < 49152; off++)
        {
            bus->mem_write8(16384 + off, sna_memory[off]);
        }
    }
}
#pragma pack(push, 1)
struct Z80Header
{
    uint16_t AF;
    uint16_t BC;
    uint16_t HL;
    uint16_t PC;
    uint16_t SP;
    uint8_t I, R;
    uint8_t BDR;
    uint16_t DE;
    uint16_t BC_;
    uint16_t DE_;
    uint16_t HL_;
    uint16_t AF_;
    uint16_t IY;
    uint16_t IX;
    uint8_t IFF1;
    uint8_t IFF2;
    uint8_t IM;
};
#pragma pack(pop)

void MainWindow::load_z80(const QString &filename)
{
    QFile z80(filename);

    if (z80.open(QIODevice::ReadOnly))
    {
        QByteArray buffer;
        buffer = z80.readAll();
        Z80Header * z80_hdr = reinterpret_cast<Z80Header *>(buffer.data());
        uint8_t * z80_memory = reinterpret_cast<uint8_t *>(buffer.data()) + sizeof(Z80Header);

        if (z80_hdr->BDR == 0xff) z80_hdr->BDR =0x01;
        cpustate.state.af.value_uint16 = z80_hdr->AF;
        cpustate.state.bc.value_uint16 = z80_hdr->BC;
        cpustate.state.hl.value_uint16 = z80_hdr->HL;
        cpustate.state.pc = z80_hdr->PC;
        cpustate.state.sp = z80_hdr->SP;
        cpustate.state.i = z80_hdr->I;
        cpustate.state.r = (z80_hdr->R &0x7f) | ((z80_hdr->BDR & 0x01) << 7);
        cpustate.state.de.value_uint16 = z80_hdr->DE;
        cpustate.state.bc_.value_uint16 = z80_hdr->BC_;
        cpustate.state.de_.value_uint16 = z80_hdr->DE_;
        cpustate.state.hl_.value_uint16 = z80_hdr->HL_;
        cpustate.state.af_.value_uint16 = z80_hdr->AF_;
        cpustate.state.iy.value_uint16 = z80_hdr->IY;
        cpustate.state.ix.value_uint16 = z80_hdr->IX;
        cpustate.state.internal.iff1 = z80_hdr->IFF1;
        cpustate.state.internal.iff2 = z80_hdr->IFF2;
        cpustate.state.internal.im = z80_hdr->IM & 0x03;
        bus->io_write8(0xfe, (z80_hdr->BDR >> 1) & 0x07);

        if ( z80_hdr->BDR & 0x20)
        {
            QByteArray data;
            int state = 0;
            uint8_t *ptr = z80_memory;
            int count = buffer.size() - sizeof(Z80Header);
            unsigned reps = 0;

            while (count--) {
                uint8_t byte = *(ptr++);
                if (state == 0 and byte == 0xed) { state = 1; continue; }
                if (state == 0) { data.append(byte); continue; }
                if (state == 1 and byte == 0xed) { state = 2; continue; }
                if (state == 1) { data.append(0xed); data.append(byte); state = 0; continue; }
                if (state == 2 and byte == 0x00) { break; }
                if (state == 2) { reps = byte; state = 3; continue; }
                while (reps--) data.append(byte);
                state = 0;
            }
            for (int off=0; off <49152; off++) {
                if (off >= data.size()) break;
                bus->mem_write8(16384 + off, data[off]);
            }
        }
        else
        {
            for (int off = 0; off < 49152; off++)
            {
                bus->mem_write8(16384 + off, z80_memory[off]);
            }
        }

    }
}

void MainWindow::upPressed()
{
    switch (ui->cbJoystickInterface->currentIndex()) {
    case CURSOR_IF: bus->key_press(3, 12);break;
    case KEMPSTON_IF: bus->kj_button_press(Port1F::KJ_UP);break;
    case SINCLAIR_IF2: bus->key_press(1, 12);break;
        case SINCLAIR_IF2_2: bus->key_press(3, 11);break;
    }
}

void MainWindow::downPressed()
{
    switch (ui->cbJoystickInterface->currentIndex()) {
    case CURSOR_IF: bus->key_press(4, 12);break;
    case KEMPSTON_IF: bus->kj_button_press(Port1F::KJ_DOWN);break;
    case SINCLAIR_IF2: bus->key_press(2, 12);break;
        case SINCLAIR_IF2_2: bus->key_press(2, 11);break;
    }
}

void MainWindow::leftPressed()
{
    switch (ui->cbJoystickInterface->currentIndex()) {
    case CURSOR_IF: bus->key_press(4, 11);break;
    case KEMPSTON_IF: bus->kj_button_press(Port1F::KJ_LEFT);break;
        case SINCLAIR_IF2: bus->key_press(4, 12);break;
        case SINCLAIR_IF2_2: bus->key_press(0, 11);break;
    }
}

void MainWindow::rightPressed()
{
    switch (ui->cbJoystickInterface->currentIndex()) {
    case CURSOR_IF: bus->key_press(2, 12);break;
    case KEMPSTON_IF: bus->kj_button_press(Port1F::KJ_RIGHT);break;
        case SINCLAIR_IF2: bus->key_press(3, 12);break;
        case SINCLAIR_IF2_2: bus->key_press(1, 11);break;
    }
}

void MainWindow::firePressed()
{
    switch (ui->cbJoystickInterface->currentIndex()) {
    case CURSOR_IF: bus->key_press(0, 12);break;
    case KEMPSTON_IF: bus->kj_button_press(Port1F::KJ_FIRE);break;
        case SINCLAIR_IF2: bus->key_press(0, 12);break;
        case SINCLAIR_IF2_2: bus->key_press(4, 11);break;
    }
}

void MainWindow::keyPressed(int sc)
{
    auto elem =s_key_mapping.find(sc);
    if (elem != s_key_mapping.end())
    {
        int row =FIRST(elem.value());
        int col = SECOND(elem.value());
        bus->key_press(row, col);
    }
}

void MainWindow::upRelease()
{
    switch (ui->cbJoystickInterface->currentIndex()) {
    case CURSOR_IF: bus->key_release(3, 12);break;
    case KEMPSTON_IF: bus->kj_button_release(Port1F::KJ_UP);break;
        case SINCLAIR_IF2: bus->key_release(1, 12);break;
        case SINCLAIR_IF2_2: bus->key_release(3, 11);break;
    }
}

void MainWindow::downRelease()
{
    switch (ui->cbJoystickInterface->currentIndex()) {
    case CURSOR_IF: bus->key_release(4, 12);break;
    case KEMPSTON_IF: bus->kj_button_release(Port1F::KJ_DOWN);break;
        case SINCLAIR_IF2: bus->key_release(2, 12);break;
        case SINCLAIR_IF2_2: bus->key_release(2, 11);break;
    }
}

void MainWindow::leftRelease()
{
    switch (ui->cbJoystickInterface->currentIndex()) {
    case CURSOR_IF: bus->key_release(4, 11);break;
    case KEMPSTON_IF: bus->kj_button_release(Port1F::KJ_LEFT);break;
        case SINCLAIR_IF2: bus->key_release(4, 12);break;
        case SINCLAIR_IF2_2: bus->key_release(0, 11);break;
    }
}

void MainWindow::fireRelease()
{
    switch (ui->cbJoystickInterface->currentIndex()) {
    case CURSOR_IF: bus->key_release(0, 12);break;
    case KEMPSTON_IF: bus->kj_button_release(Port1F::KJ_FIRE);break;
        case SINCLAIR_IF2: bus->key_release(0, 12);break;
        case SINCLAIR_IF2_2: bus->key_release(4, 11);break;
    }
}

void MainWindow::rightRelease()
{
    switch (ui->cbJoystickInterface->currentIndex()) {
    case CURSOR_IF: bus->key_release(2, 12);break;
    case KEMPSTON_IF: bus->kj_button_release(Port1F::KJ_RIGHT);break;
        case SINCLAIR_IF2: bus->key_release(3, 12);break;
        case SINCLAIR_IF2_2: bus->key_release(1, 11);break;
    }
}

void MainWindow::keyRelease(int sc)
{
    auto elem =s_key_mapping.find(sc);
    if (elem != s_key_mapping.end())
    {
        int row =FIRST(elem.value());
        int col = SECOND(elem.value());
        bus->key_release(row, col);
    }
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object);

/*    if (event->type() ==QEvent::FocusOut and
            object == ui->screen and
            ui->cbCaptureKeyboard->isChecked()) {
            ui->screen->setFocus();
    }*/


    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *ke = static_cast<QKeyEvent*>(event);
        qDebug() << "Key pressed: " << ke->nativeScanCode() << ke->text();
        auto sc = ke->nativeScanCode();
        switch (sc) {
            case ESC_SCANCODE: reset();break;
            case F12_SCANCODE: z80_nmi(&cpustate);break;
            case UP_SCANCODE: upPressed();break;
            case DOWN_SCANCODE: downPressed();break;
            case LEFT_SCANCODE: leftPressed();break;
            case RIGHT_SCANCODE: rightPressed();break;
            case UP2_SCANCODE: upPressed();break;
            case DOWN2_SCANCODE: downPressed();break;
            case LEFT2_SCANCODE: leftPressed();break;
            case RIGHT2_SCANCODE: rightPressed();break;
            case LCTRL_SCANCODE: firePressed();break;
            case RCTRL_SCANCODE: firePressed();break;
            default :
            {
                keyPressed(sc);
            }

        }
    }
    if (event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *ke = static_cast<QKeyEvent*>(event);
        qDebug() << "Key release: " << ke->nativeScanCode() << ke->text();
        auto sc = ke->nativeScanCode();

        switch (sc) {

            case UP_SCANCODE:       upRelease();break;
            case DOWN_SCANCODE:   downRelease();break;
            case LEFT_SCANCODE:   leftRelease();break;
            case RIGHT_SCANCODE: rightRelease();break;
            case UP2_SCANCODE:      upRelease();break;
            case DOWN2_SCANCODE:  downRelease();break;
            case LEFT2_SCANCODE:  leftRelease();break;
            case RIGHT2_SCANCODE:rightRelease();break;
            case LCTRL_SCANCODE:  fireRelease();break;
            case RCTRL_SCANCODE:  fireRelease();break;
            default :
            {
            keyRelease(sc);
            }

        }
    }

    return false;
}

void MainWindow::frameRefresh()
{
    // 3 500 000 / 50
    // 70 000
    //
    z80_run(&cpustate,70000 - 28);
    z80_int(&cpustate, 1);
    z80_run(&cpustate, 28);
    z80_int(&cpustate, 0);
    ui->screen->repaint();
}

void MainWindow::on_cbShowControls_stateChanged(int state)
{
    if (state == Qt::Checked)
        ui->twControls->setVisible(true);
    else
        ui->twControls->setVisible(false);
}

void MainWindow::reset()
{
    z80_reset(&cpustate);
    bus->reset();
}

void MainWindow::on_key_pressed(int row, int col)
{
    qDebug() << "Key pressed: " << row << " " << col;
    bus->key_press(row, col);

}

void MainWindow::on_key_released(int row, int col)
{
    qDebug() << "Key released: " << row << " " << col;
    bus->key_release(row, col);
}

void MainWindow::on_cbCaptureKeyboard_stateChanged(int state)
{
    if (state == Qt::Checked)
    {
        installEventFilter(this);
    }
    else
    {
        installEventFilter(nullptr);
    }

}

void MainWindow::on_buttonTEST_clicked()
{
    load_sna("sna/river.sna");
}

void MainWindow::on_actionSpectrum_48k_triggered()
{
    ui->actionSpectrum_48k->setChecked(true);
    ui->actionSpectrum_128k->setChecked(false);
    auto new_bi = new BusInterface48();
    auto old_bi = bus;
    ui->screen->setBusInterface(new_bi);
    cpustate.context = new_bi;
    bus = new_bi;
    delete old_bi;
}

void MainWindow::on_actionSpectrum_128k_triggered()
{
    ui->actionSpectrum_48k->setChecked(false);
    ui->actionSpectrum_128k->setChecked(true);
    auto new_bi = new BusInterface128();
    auto old_bi = bus;
    ui->screen->setBusInterface(new_bi);
    cpustate.context = new_bi;
    bus = new_bi;
    delete old_bi;
}

void MainWindow::on_action_Load_a_snapshot_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),"sna/","*.sna");
    load_sna(fileName);
}

void MainWindow::on_action_Exit_triggered()
{
    exit(0);
}

void MainWindow::on_action_Reset_triggered()
{
    reset();
}

void MainWindow::on_action_NMI_triggered()
{
    z80_nmi(&cpustate);
}

void MainWindow::on_action_About_triggered()
{
    QMessageBox aboutBox;
    aboutBox.setWindowTitle("About");
    aboutBox.setText("Emulator ZX spectrum\nby Basserti\n2021");
    aboutBox.exec();
}

void MainWindow::on_action_color1_triggered()
{
    ui->action_color1->setChecked(true);
    ui->action_color2->setChecked(false);
    bus->_color_pal = 1;
}

void MainWindow::on_action_color2_triggered()
{
    ui->action_color1->setChecked(false);
    ui->action_color2->setChecked(true);
    bus->_color_pal = 2;
}

void MainWindow::on_actionLoad_a_SCR_file_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),"scr/","*.scr");
    QFile scr_file(fileName);
    if (scr_file.open(QIODevice::ReadOnly)) {
        QByteArray buffer;
        buffer =scr_file.readAll();
        uint8_t * scr_memory =reinterpret_cast<uint8_t *>(buffer.data());
        for (int off = 0; off < 6912; off++)
        {
            bus->mem_write8(16384 + off, scr_memory[off]);
        }
    }
}

void MainWindow::on_actionSave_a_SCR_file_triggered()
{
    QByteArray buffer;
    for (int off = 0; off < 6912; off++)
    {
        buffer[off] = bus->mem_read8(16384 + off);
    }
    //qDebug() << buffer;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),"scr/","*.scr");
        QFile scr_file(fileName);
        if (scr_file.open(QIODevice::WriteOnly))
        {
            scr_file.write(buffer);
            scr_file.close();
        }

}

void MainWindow::on_actionLoad_a_z80_file_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),"sna/","*.z80");
    load_z80(fileName);
}

void MainWindow::on_actionSave_a_z80_file_triggered()
{
    Z80Header header;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),"sna/","*.z80");
        QFile z80_file(fileName);
        if (z80_file.open(QIODevice::WriteOnly))
        {
            header.AF = cpustate.state.af.value_uint16;
            header.BC = cpustate.state.bc.value_uint16;
            header.HL = cpustate.state.hl.value_uint16;
            header.PC = cpustate.state.pc;
            header.SP = cpustate.state.sp;
            header.I = cpustate.state.i;
            header.R = (cpustate.state.r &0x7f);
            header.BDR = (bus->border() << 1);
            header.DE = cpustate.state.de.value_uint16;
            header.BC_ = cpustate.state.bc_.value_uint16;
            header.DE_ = cpustate.state.de_.value_uint16;
            header.HL_ = cpustate.state.hl_.value_uint16;
            header.AF_ = cpustate.state.af_.value_uint16;
            header.IY = cpustate.state.iy.value_uint16;
            header.IX = cpustate.state.ix.value_uint16;
            header.IFF1 = cpustate.state.internal.iff1;
            header.IFF2 = cpustate.state.internal.iff2;
            header.IM = (cpustate.state.internal.im & 0x03);
            z80_file.write(reinterpret_cast<const char *>(&header), sizeof(header));
            QByteArray buffer;
            for (int off = 0; off < 49152; off++)
            {
                buffer[off] = bus->mem_read8(16384 + off);
                //z80_file.write(buffer);
            }
            z80_file.write(buffer);
            z80_file.close();
        }
}
/*struct Z80Header
{
    uint16_t AF;
    uint16_t BC;
    uint16_t HL;
    uint16_t PC;
    uint16_t SP;
    uint8_t I, R;
    uint8_t BDR;
    uint16_t DE;
    uint16_t BC_;
    uint16_t DE_;
    uint16_t HL_;
    uint16_t AF_;
    uint16_t IY;
    uint16_t IX;
    uint8_t IFF1;
    uint8_t IFF2;
    uint8_t IM;
};*/
