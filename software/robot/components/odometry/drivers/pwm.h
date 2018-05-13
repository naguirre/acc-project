#ifndef __PWM_H__
#define __PWM_H__

class Pwm
{

public:

    typedef enum {
        CHANNEL0,
        CHANNEL1,
        CHANNEL2,
        CHANNEL3,
    } Channel;

    Pwm(Pwm::Channel channel);

    void SetDutyCycle(float duty);

private:

    Pwm::Channel m_channel;

    float m_dutyCycle;

};

#endif /* __PWM_H__ */
