#include "mydefs.h"
#include "suart.h"
#include <avr/io.h>
#include <avr/interrupt.h>

static u8 analog_val = 0;
static u8 analog_ok = 0;

void init( void )
{
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Set ADC prescaler to 128 - 125KHz sample rate @ 16MHz

  ADMUX = 0x00; // Set ADC to PIN PB0 (ADC0)
  ADCSRB = 0x00; // Free running mode
  ADCSRA |= (1 << ADEN);  // Enable ADC
  ADCSRA |= (1 << ADIE);  // Enable ADC Interrupt
  ADCSRA |= (1 << ADSC);  // Start ADC Conversions
  while(!analog_ok); // Wait for first conversion to be done
  suart_init();
}

ISR(ADC_vect)
{
    analog_val = ADCL;
    analog_ok = 1;
}

int main( void )
{
    int nb = 0;
    init();
    sei();

    for (;;)
    {
        while(kbhit())
        {
            nb = ugetchar() + 1;
            uputchar(nb);
            while(nb--)
                uputchar(ugetchar());
            uputchar(analog_val);
        }
    }
}
