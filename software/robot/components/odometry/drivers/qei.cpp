#include "qei.h"
#include <driver/pcnt.h>


Qei::Qei(Qei::Channel channel, Gpio::Pin pin) :
     m_channel(channel)
{
     pcnt_config_t pcnt_config;

     
     pcnt_config.pulse_gpio_num = pin;
     pcnt_config.ctrl_gpio_num = -1;
     pcnt_config.channel = PCNT_CHANNEL_0;
     pcnt_config.unit = PCNT_UNIT_0;

     pcnt_unit_config(&pcnt_config);

}

uint16_t Qei::GetStepsCount()
{
     int16_t val = 0;
     pcnt_get_counter_value(PCNT_UNIT_0, &val);
     return (uint16_t)val;
}
