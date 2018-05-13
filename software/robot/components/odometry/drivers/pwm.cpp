#include "pwm.h"

Pwm::Pwm(Pwm::Channel chan)
{
     m_channel = chan;
     m_dutyCycle = 0.0;
}

void Pwm::SetDutyCycle(float duty)
{
     m_dutyCycle = duty;
}
