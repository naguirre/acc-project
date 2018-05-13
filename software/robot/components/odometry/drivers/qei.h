#ifndef __QEI_H__
#define __QEI_H__

#include <types.h>

class Qei
{

public:

    typedef enum {
        CHANNEL0,
        CHANNEL1,
        CHANNEL2,
        CHANNEL3,
    } Channel;

    Qei(Qei::Channel channel);

    uint16_t GetStepsCount();

private:

    Qei::Channel m_channel;
};

#endif /* __QEI_H__ */
