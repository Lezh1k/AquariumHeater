/*
 * commons.h
 *
 * Created: 24.10.2016 23:28:16
 *  Author: Lezh1k
 */ 

#ifndef COMMONS_H_
#define COMMONS_H_

#include <stdint.h>

#define BAUD_RATE 9600
#define F_CPU 8000000UL
#define F_TIMER 1000000UL
#define UART_PERIOD (F_TIMER / BAUD_RATE)
#define UART_HALF_PERIOD (F_TIMER / (BAUD_RATE*2))

#define enable_adc_int() (ADCSRA |= (1 << ADIE))
#define disable_adc_int() (ADCSRA &= ~(1 << ADIE))  
#define enable_pcie_int() (GIMSK |= (1 << PCIE))
#define disable_pcie_int() (GIMSK &= ~(1 << PCIE))
#define enable_tim0_ovf_int() (TIMSK |= (1 << TOIE0))
#define disable_tim0_ovf_int() (TIMSK &= ~(1 << TOIE0))

#define nop() asm("nop" "\n\t")

enum sinterrupt {
	sinterrupt_adc_finished = (1 << 1),
	sinterrupt_dst_temperature_changed = (1 << 2),
	sinterrupt_tim0_ovf = (1 << 3)
};

#define TERMISTOR_B				3380
#define TERMISTOR_RREF		10000
#define TERMISTOR_R0			10000
#define TERMISTOR_T0			25

#endif /* COMMONS_H_ */