#ifndef __GPIO_H__
#define __GPIO_H__

class Gpio
{

public:
    typedef enum {
        PORTA,
        PORTB,
        PORTC,
        PORTD,
        PORTF,
        PORTE,
    } Port;

    typedef unsigned char Pin;

    typedef enum {
        OUTPUT = 0,
        INPUT = 1,
    } Direction;

    typedef enum {
        LOW = 0,
        HIGH = 1,
    } State;

    Gpio(Gpio::Port port, Gpio::Pin pin, Gpio::Direction direction);

    Gpio::State Read();

    void Write(Gpio::State state);

    void Toggle();

private:

    Gpio::Pin       m_pin;
    Gpio::State     m_state;

};

#endif /* __GPIO_H__ */
