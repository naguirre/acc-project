
#define F_CPU 800000
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>


static uint8_t analog_val = 0;
static uint8_t analog_ok = 0;


static void _analog_init(void)
{
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Set ADC prescaler to 128 - 125KHz sample rate @ 16MHz

  	ADMUX = 0x00; // Set ADC to PIN PB0 (ADC0)
    ADCSRB = 0x00; // Free running mode
  	ADCSRA |= (1 << ADEN);  // Enable ADC
  	ADCSRA |= (1 << ADIE);  // Enable ADC Interrupt
  	sei();	// Enable Global Interrupts
  	ADCSRA |= (1 << ADSC);  // Start A2D Conversions
    while(!analog_ok); // Wait for first conversion to be done
}

int main(void)
{
    int i = 0;
    _analog_init();

    for(;;)
    {
        ++i;

    }

    return 0;
}

ISR(ADC_vect)
{
    analog_val = ADCH;
    analog_ok = 1;
}
