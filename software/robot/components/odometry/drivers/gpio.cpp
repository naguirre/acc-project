#include <driver/gpio.h>
#include "gpio.h"

Gpio::Gpio(Gpio::Port port, Gpio::Pin pin, Gpio::Direction direction)
{
    gpio_config_t io_conf;
    io_conf.intr_type = (gpio_int_type_t)GPIO_PIN_INTR_DISABLE;
    if (direction == Gpio::OUTPUT)
        io_conf.mode =  GPIO_MODE_OUTPUT;
    else
        io_conf.mode = GPIO_MODE_INPUT;

    io_conf.pin_bit_mask = 1ULL << pin;
    io_conf.pull_down_en = (gpio_pulldown_t)0;
    io_conf.pull_up_en = (gpio_pullup_t)0;
    m_pin = pin;
    m_state = Gpio::LOW;
    gpio_config(&io_conf);
}

Gpio::State Gpio::Read()
{
	 return m_state;
}

void Gpio::Write(Gpio::State state)
{
    gpio_set_level((gpio_num_t)m_pin, (uint32_t)state);
    m_state = state;
}

void Gpio::Toggle(void)
{
	if (m_state == Gpio::LOW)
    {
        m_state = Gpio::HIGH;
        gpio_set_level((gpio_num_t)m_pin, m_state);
    }
    else
    {
        m_state = Gpio::HIGH;
        gpio_set_level((gpio_num_t)m_pin, m_state);
    }
}
