/************************************************************************/
/*                                                                      */
/*                      Several helpful definitions                     */
/*                                                                      */
/*              Author: Peter Dannegger                                 */
/*                                                                      */
/************************************************************************/
#ifndef _mydefs_h_
#define _mydefs_h_


//			Easier type writing:

typedef unsigned char  u8;
typedef   signed char  s8;
typedef unsigned short u16;
typedef   signed short s16;
typedef unsigned long  u32;
typedef   signed long  s32;


// 			Access bits like variables:

struct bits {
  u8 b0:1;
  u8 b1:1;
  u8 b2:1;
  u8 b3:1;
  u8 b4:1;
  u8 b5:1;
  u8 b6:1;
  u8 b7:1;
} __attribute__((__packed__));

#define SBIT_(port,pin) ((*(volatile struct bits*)&port).b##pin)
#define	SBIT(x,y)	SBIT_(x,y)

//			Optimization improvements
#define	OPTR18 __asm__ volatile (""::);		// it helps, but why ?
						// remove useless R18/R19

// volatile access (reject unwanted removing access):

#define vu8(x)  (*(volatile u8*)&(x))



#endif
