#ifndef BUSINTERFACE48_H
#define BUSINTERFACE48_H

#include "businterface.h"

class BusInterface48 : public BusInterface
{
    Q_OBJECT
public:
    BusInterface48();

    virtual uint8_t mem_read8(uint32_t addr) override;
    virtual void mem_write8(uint32_t addr, uint8_t value) override;

    virtual uint8_t io_read8(uint32_t addr) override;
    virtual void io_write8(uint32_t addr, uint8_t value) override;


    virtual const uint8_t * framebuffer() const override
    {return ram.getBuffer(16384);}

protected:
#if defined(WIN32)
    ROMDevice rom {"rom/48.rom"};
#endif
#if defined (Q_OS_ANDROID)
    ROMDevice rom {"assets:/rom/48.rom"};
#endif

    RAMDevice ram { 16 };


};

#endif // BUSINTERFACE48_H
