/*
* AquariumHeater.c
*
* Created: 13.10.2016 19:05:09
* Author : Lezh1k
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>
#include <avr/pgmspace.h>

#include "commons.h"
#include "max7219.h"

volatile uint8_t SINTERRUPTS = 0; 
volatile uint8_t DST_T = 35;
volatile uint8_t CUR_T = 0;
volatile uint8_t CUR_LT = 0;

#define PIN_BTN_UP (1 << PINB1)
#define PIN_BTN_DOWN (1 << PINB0)
#define PORT_BTN_UP (1 << PB1)
#define PORT_BTN_DOWN (1 << PB0)
//////////////////////////////////////////////////////////////////////////

static void handle_sint_adc();
static inline void print_dest_temperature();
static inline void print_current_temperature();
uint8_t adcw_to_temperature(uint16_t adc_avg);

static inline void start_adc() {
	ADCSRA |= (1 << ADSC);
}
//////////////////////////////////////////////////////////////////////////

ISR(TIMER0_OVF_vect, ISR_NAKED) {
	asm("push r24" "\n\t");
	SINTERRUPTS |= sinterrupt_tim0_ovf;
	asm("pop r24" "\n\t");
	reti();
}
//////////////////////////////////////////////////////////////////////////

ISR(ADC_vect, ISR_NAKED) {
	asm("push r24" "\n\t");
  SINTERRUPTS |= sinterrupt_adc_finished;
	asm("pop r24" "\n\t");
	reti();
}
//////////////////////////////////////////////////////////////////////////

ISR(PCINT0_vect, ISR_NAKED) {
	asm("push r24" "\n\t");
	disable_pcie_int();	
	do {
		if (!(PINB & PIN_BTN_UP)) {
			++DST_T;
			SINTERRUPTS |= sinterrupt_dst_temperature_changed;
			break;
		}

		if (!(PINB & PIN_BTN_DOWN)) {
			--DST_T;
			SINTERRUPTS |= sinterrupt_dst_temperature_changed;
			break;
		}
	} while(0);	
	enable_pcie_int();
	asm("pop r24" "\n\t");
	reti();
}
//////////////////////////////////////////////////////////////////////////

enum {TIM0_OVF_CNT = 3};
int
main(void) {  
	register int8_t tim0_ovf_cnt = TIM0_OVF_CNT;
	
  //configure adc
  ADCSRA = (1 << ADPS2) | (1 << ADEN) ; //use 16 prescaler. in our program it's 1000000/16 ~ 65kHz
	enable_adc_int(); //enable adc interrupt
  
  max7219_init();
	print_dest_temperature();
	
	PCMSK = PORT_BTN_DOWN | PORT_BTN_UP;
	enable_pcie_int();

	TCCR0B = (1 << CS02) | (1 << CS00); //1024 prescalser ~0.3 sec.
	enable_tim0_ovf_int();
  
	sei();	
	start_adc();	
	
  while (1) {
	  if (SINTERRUPTS & sinterrupt_adc_finished) {
		  handle_sint_adc();
		  max7219_turn_heater(CUR_T < DST_T);
			start_adc();
	  }

	  if (SINTERRUPTS & sinterrupt_dst_temperature_changed) {
		  SINTERRUPTS &= ~sinterrupt_dst_temperature_changed;
		  print_dest_temperature();
		  max7219_turn_heater(CUR_T < DST_T);
	  }

	  if (SINTERRUPTS & sinterrupt_tim0_ovf) {
		  SINTERRUPTS &= ~sinterrupt_tim0_ovf;	
		  if (!(--tim0_ovf_cnt)) {
			  tim0_ovf_cnt = TIM0_OVF_CNT;
				print_current_temperature();
		  }			
	  }
  }
}
//////////////////////////////////////////////////////////////////////////

void
handle_sint_adc() {
	enum {ADC_CNT = 64}; //(0xffff / 0x03ff)
	static int8_t adc_cnt = ADC_CNT+1;
	static uint16_t adc_avg = 0;
	do {
		SINTERRUPTS &= ~sinterrupt_adc_finished;
		if (--adc_cnt) {
			adc_avg += ADCW;
			break;
		}		

		adc_cnt = ADC_CNT+1;			
		adc_avg >>= 6;
		
		CUR_T = adcw_to_temperature(adc_avg);

		adc_avg = 0;
	} while(0);	
}
//////////////////////////////////////////////////////////////////////////							 

uint8_t adcw_to_temperature(uint16_t adc_avg) {
	//change this table if TERMISTOR_B, TERMISTOR_RREF, TERMISTOR_R0 or TERMISTOR_T0 is changed
	static const int16_t adc_temperature[] PROGMEM= {
		493, 340, 288, 258, 238, 222, 210, 201,
		192, 185, 179, 173, 168, 164, 160, 156,
		152, 149, 146, 143, 141, 138, 136, 133,
		131, 129, 127, 126, 124, 122, 120, 119,
		117, 116, 114, 113, 112, 110, 109, 108,
		107, 106, 105, 104, 103, 102, 101, 100,
		99, 98, 97, 96, 95, 94, 93, 93,
		92, 91, 90, 89, 89, 88, 87, 87,
		86, 85, 85, 84, 83, 83, 82, 81,
		81, 80, 80, 79, 79, 78, 77, 77,
		76, 76, 75, 75, 74, 74, 73, 73,
		72, 72, 71, 71, 70, 70, 69, 69,
		69, 68, 68, 67, 67, 66, 66, 66,
		65, 65, 64, 64, 64, 63, 63, 62,
		62, 62, 61, 61, 61, 60, 60, 59,
		59, 59, 58, 58, 58, 57, 57, 57,
		56, 56, 56, 55, 55, 55, 54, 54,
		54, 53, 53, 53, 52, 52, 52, 52,
		51, 51, 51, 50, 50, 50, 49, 49,
		49, 49, 48, 48, 48, 47, 47, 47,
		47, 46, 46, 46, 46, 45, 45, 45,
		44, 44, 44, 44, 43, 43, 43, 43,
		42, 42, 42, 42, 41, 41, 41, 41,
		40, 40, 40, 40, 39, 39, 39, 39,
		38, 38, 38, 38, 37, 37, 37, 37,
		37, 36, 36, 36, 36, 35, 35, 35,
		35, 34, 34, 34, 34, 34, 33, 33,
		33, 33, 32, 32, 32, 32, 32, 31,
		31, 31, 31, 30, 30, 30, 30, 30,
		29, 29, 29, 29, 29, 28, 28, 28,
		28, 28, 27, 27, 27, 27, 26, 26,
		26, 26, 26, 25, 25, 25, 25, 25,
		24, 24, 24, 24, 24, 23, 23, 23,
		23, 23, 22, 22, 22, 22, 22, 21,
		21, 21, 21, 21, 20, 20, 20, 20,
		20, 19, 19, 19, 19, 19, 18, 18,
		18, 18, 18, 17, 17, 17, 17, 17,
		16, 16, 16, 16, 16, 15, 15, 15,
		15, 15, 14, 14, 14, 14, 14, 13,
		13, 13, 13, 13, 12, 12, 12, 12,
		12, 11, 11, 11, 11, 11, 10, 10,
		10, 10, 10, 9, 9, 9, 9, 9,
		8, 8, 8, 8, 7, 7, 7, 7,
		7, 6, 6, 6, 6, 6, 5, 5,
		5, 5, 5, 4, 4, 4, 4, 4,
		3, 3, 3, 3, 2, 2, 2, 2,
		2, 1, 1, 1, 1, 1, 0, 0,
		0, 0, 0, 0, 0, 0, -1, -1,
		-1, -1, -1, -2, -2, -2, -2, -3,
		-3, -3, -3, -4, -4, -4, -4, -4,
		-5, -5, -5, -5, -6, -6, -6, -6,
		-7, -7, -7, -7, -8, -8, -8, -9,
		-9, -9, -9, -10, -10, -10, -10, -11,
		-11, -11, -12, -12, -12, -12, -13, -13,
		-13, -14, -14, -14, -14, -15, -15, -15,
		-16, -16, -16, -17, -17, -17, -18, -18,
		-18, -19, -19, -19, -20, -20, -20, -21,
		-21, -22, -22, -22, -23, -23, -24, -24,
		-24, -25, -25, -26, -26, -27, -27, -28,
		-28, -29, -29, -30, -30, -31, -31, -32,
		-32, -33, -33, -34, -35, -35, -36, -37,
		-37, -38, -39, -40, -41, -41, -42, -43,
		-44, -45, -46, -48, -49, -50, -52, -53,
		-55, -57, -60, -62, -65, -70, -76, -88,
	};

	return pgm_read_byte(&adc_temperature[adc_avg / 2]);
}
//////////////////////////////////////////////////////////////////////////

void
print_current_temperature() {
	max7219_set_symbol(MS_0, CUR_T);
}
//////////////////////////////////////////////////////////////////////////

void 
print_dest_temperature() {
	max7219_set_symbol(MS_1, DST_T);
}
//////////////////////////////////////////////////////////////////////////