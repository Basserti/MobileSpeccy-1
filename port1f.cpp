#include "port1f.h"

Port1F::Port1F()
{

}

uint8_t Port1F::read8(uint32_t address)
{
    Q_UNUSED(address);
    return _port_1f_data;
}

void Port1F::write8(uint32_t address, uint8_t value)
{
    Q_UNUSED(address);
    Q_UNUSED(value);
}

void Port1F::press_button(int btn)
{
    _port_1f_data |= (1 << btn);
}

void Port1F::release_button(int btn)
{
    _port_1f_data &= ~(1 << btn);
}
