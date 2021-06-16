#include "portfe.h"

PortFE::PortFE()
{

}

uint8_t PortFE::read8(uint32_t address)
{
    uint8_t col = (address >> 8) & 0xff;
    uint8_t col_mask = 1;
    uint8_t result = 0b11111111;
    for (int i = 0; i < 8; ++i)
    {
        if ( not (col & col_mask))
        {
            result &=_key_matrix[i];
        }
        col_mask <<= 1;
    }

    return result;
}

void PortFE::write8(uint32_t address, uint8_t value)
{
    Q_UNUSED(address);
    _fe_data = value;
}

void PortFE::press_key(int row, int col)
{
    _key_matrix[col - 8] &= ~(1 << row);
}

void PortFE::release_key(int row, int col)
{
    _key_matrix[col - 8] |= (1 << row);
}
