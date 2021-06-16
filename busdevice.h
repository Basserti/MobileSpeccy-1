#ifndef BUSDEVICE_H
#define BUSDEVICE_H

#include <QObject>

class BusDevice : public QObject
{
    Q_OBJECT
public:
    explicit BusDevice(QObject *parent = nullptr);
    virtual ~BusDevice() = default;

    virtual uint8_t read8(uint32_t address) = 0; //pure virtual (обстрактный метод)
    virtual void write8(uint32_t address, uint8_t value) = 0;

signals:

};

#endif // BUSDEVICE_H
