/*
* AquariumHeater.c
*
* Created: 13.10.2016 19:05:09
* Author : Lezh1k
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>

#include "commons.h"
#include "max7219.h"

/*be very carefull with these 4 register variables. there are -ffixed-reg option in compiler settings. 
  but who knows which library procedure could use these registers*/
register uint8_t SINTERRUPTS asm("r3");
register uint8_t DST_T asm("r4");
register uint8_t CUR_T asm("r5");
register uint8_t CUR_LT asm("r6");

#define PIN_BTN_UP (1 << PINB1)
#define PIN_BTN_DOWN (1 << PINB0)
#define PORT_BTN_UP (1 << PB1)
#define PORT_BTN_DOWN (1 << PB0)
//////////////////////////////////////////////////////////////////////////

static void handle_sint_adc();
static inline void print_dest_temperature();
static inline void print_current_temperature();

ISR(ADC_vect, ISR_NAKED) {
	asm("push r24" "\n\t");
  SINTERRUPTS |= sinterrupt_adc_finished;
	asm("pop r24" "\n\t");
	reti();
}
//////////////////////////////////////////////////////////////////////////

ISR(PCINT0_vect, ISR_NAKED) {
	asm("push r24" "\n\t");
	disable_pcie_interrupt();	
	do {
		if ((PINB & PIN_BTN_UP) == 0) {
			++DST_T;
			SINTERRUPTS |= sinterrupt_dst_temperature_changed;
			break;
		}

		if ((PINB & PIN_BTN_DOWN) == 0) {
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

static inline void start_adc() {
	ADCSRA |= (1 << ADSC); 
}
//////////////////////////////////////////////////////////////////////////

int
main(void) {  
	SINTERRUPTS = CUR_T = CUR_LT = 0x00;
	DST_T = 25;
  //configure adc
  ADCSRA |= (1 << ADPS1) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADEN); //use 64 prescaler. in our program it's 8000000/64 = 125kHz
	enable_adc_int(); //enable adc interrupt
  
  max7219_init();
	print_dest_temperature();
	
	PCMSK = PORT_BTN_DOWN | PORT_BTN_UP;
	enable_pcie_int();
  sei();	
	start_adc();
	
  while (1) {
	  if (SINTERRUPTS & sinterrupt_adc_finished) {
		  handle_sint_adc();
			if (CUR_LT != CUR_T) {
				CUR_LT = CUR_T;
				print_current_temperature();
			}
			max7219_turn_heater(CUR_T < DST_T);
	  }

		if (SINTERRUPTS & sinterrupt_dst_temperature_changed) {
			SINTERRUPTS &= ~sinterrupt_dst_temperature_changed;
			print_dest_temperature();
			max7219_turn_heater(CUR_T < DST_T);
		}
  }
}
//////////////////////////////////////////////////////////////////////////

//~0.5C . 
#define ADC_DIFF 6
void
handle_sint_adc() {
	enum {ADC_CNT = 64}; //(0xffff / 0x03ff)
	static int8_t adc_cnt = ADC_CNT+1;
	static uint16_t adc_avg = 0;
	static uint16_t adc_val = 0;
	float r, st;
	do {
		SINTERRUPTS &= ~sinterrupt_adc_finished;
		
		if (--adc_cnt) {
			adc_avg += ADCW;
			break;
		}

		adc_avg >>= 6;
		++adc_avg; //because we don't want adc == 0
		adc_cnt = ADC_CNT+1;
		
		if ((adc_avg ^ adc_val) < ADC_DIFF) { //if (abs(adx_avg-adc_val) < ADC_DIFF)
			adc_avg = 0;
			break;
		}
		
		adc_val = adc_avg;
		adc_avg = 0;

		r = TERMISTOR_RREF / (1024.0f / adc_val - 1);
		st = r / TERMISTOR_R0;
		st = logf(st);
		st /= TERMISTOR_B;
		st += 1.0f / (TERMISTOR_T0 + 273.15f);
		st = 1.0f / st;
		st -= 273.15f;
		CUR_T = (uint8_t)st;
	} while(0);

	start_adc();
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