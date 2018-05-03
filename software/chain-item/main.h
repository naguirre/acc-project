#ifndef _main_h_
#define _main_h_


#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "mydefs.h"


#define	XTAL	8000000
#define	BAUD	57600

#define F_CPU   XTAL
#include <util/delay.h>

#define	STXD		SBIT( PORTB, PB1 )	// = OC1A
#define	STXD_DDR	SBIT( DDRB,  PB1 )

#define	SRXD_PIN	SBIT( PINB,  PB2 )	// = ICP


#endif
