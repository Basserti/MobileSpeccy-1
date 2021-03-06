#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "businterface.h"
#include "emulation/CPU/Z80.h"
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void load_sna(const QString& filename);
    void load_z80(const QString& filename);

    void keyPressed(int sc);

    void keyRelease(int sc);

public slots:
    void upPressed();

    void downPressed();

    void leftPressed();

    void rightPressed();

    void firePressed();

    void upRelease();

    void downRelease();

    void leftRelease();

    void fireRelease();

    void rightRelease();

protected:
    virtual bool eventFilter(QObject *object, QEvent *event) override;

private slots:
    void frameRefresh();

    void on_cbShowControls_stateChanged(int state);

    void reset();

    void on_key_pressed(int row, int col);
    void on_key_released(int row, int col);

    void on_cbCaptureKeyboard_stateChanged(int arg1);

    void on_buttonTEST_clicked();

    void on_actionSpectrum_48k_triggered();

    void on_actionSpectrum_128k_triggered();

    void on_action_Load_a_snapshot_triggered();

    void on_action_Exit_triggered();

    void on_action_Reset_triggered();

    void on_action_NMI_triggered();

    void on_action_About_triggered();

    void on_action_color1_triggered();

    void on_action_color2_triggered();

    void on_actionLoad_a_SCR_file_triggered();

    void on_actionSave_a_SCR_file_triggered();

    void on_actionLoad_a_z80_file_triggered();

    void on_actionSave_a_z80_file_triggered();

private:
    Ui::MainWindow *ui;

    BusInterface * bus { nullptr };

    // redcode/Z80
    Z80 cpustate {};
    QTimer *frame_timer;
    QTimer *flash_timer;
};
#endif // MAINWINDOW_H
