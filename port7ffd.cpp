#include "port7ffd.h"

Port7FFD::Port7FFD()
{

}

uint8_t Port7FFD::read8(uint32_t address)
{
    Q_UNUSED(address);
    return 0;
}

void Port7FFD::write8(uint32_t address, uint8_t value)
{
    Q_UNUSED(address);
    if (!locked())
        _port_7ffd_data = value;
}
