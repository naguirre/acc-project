#ifndef __QEI_H__
#define __QEI_H__

#include <types.h>
#include "gpio.h"
class Qei
{

public:

    typedef enum {
        CHANNEL0,
        CHANNEL1,
        CHANNEL2,
        CHANNEL3,
    } Channel;

    Qei(Qei::Channel channel, Gpio::Pin pin);

    uint16_t GetStepsCount();

private:

    Qei::Channel m_channel;
};

#endif /* __QEI_H__ */
