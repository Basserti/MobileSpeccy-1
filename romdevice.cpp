#include "romdevice.h"
#include <QFile>
#include <QDebug>
#include <QErrorMessage>
#include <QApplication>

ROMDevice::ROMDevice(const QString &filename)
{
    QFile romfile(filename);

    if (romfile.open(QIODevice::ReadOnly))
    {
        _data = romfile.readAll();
        Q_ASSERT(_data.size() > 0);
    }
    else
    {
        QErrorMessage em;
        qDebug() << "A";
        em.setModal(true);
        em.showMessage(QString("Can't load a ROM file:")+ filename);
        em.exec();
        qDebug() << "B";
        exit(1); //FIXME переделать для qApp->exit()
    }
}

uint8_t ROMDevice::read8(uint32_t address)
{
    return _data[address % _data.size()];
}

void ROMDevice::write8(uint32_t address, uint8_t value)
{
    Q_UNUSED(address);
    Q_UNUSED(value);
}
