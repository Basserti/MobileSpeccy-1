#include "ramdevice.h"

RAMDevice::RAMDevice(int width)
{
    Q_ASSERT(width >= 0);
    _data.resize(1 << width);

    //TODO: заполнить нач знач
    for (int addr = 0; addr < _data.size(); addr++)
        _data[addr] = 0;
}

uint8_t RAMDevice::read8(uint32_t address)
{
    return _data[address % _data.size()];
}

void RAMDevice::write8(uint32_t address, uint8_t value)
{
    _data.data()[address % _data.size()] = value;
}

const uint8_t *RAMDevice::getBuffer(uint32_t address) const
{
    return &_data.data()[address % _data.size()];
}
