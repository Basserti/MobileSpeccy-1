#ifndef BUSINTERFACE128_H
#define BUSINTERFACE128_H

#include "businterface.h"
#include "port7ffd.h"

class BusInterface128 : public BusInterface
{
    Q_OBJECT
public:
    BusInterface128();

    virtual uint8_t mem_read8(uint32_t addr) override;
    virtual void mem_write8(uint32_t addr, uint8_t value) override;

    virtual uint8_t io_read8(uint32_t addr) override;
    virtual void io_write8(uint32_t addr, uint8_t value) override;


    virtual const uint8_t * framebuffer() const override
    {
        return ram.getBuffer(0x4000 * mapper.vram_page());//fixME:128
    }

    virtual void reset() override { mapper.reset(); }

protected:
#if defined(WIN32)
    ROMDevice rom {"rom/128.rom"};
#endif
#if defined (Q_OS_ANDROID)
    ROMDevice rom {"assets:/rom/128.rom"};
#endif
    RAMDevice ram { 17 };
    Port7FFD mapper;


};

#endif // BUSINTERFACE128_H
