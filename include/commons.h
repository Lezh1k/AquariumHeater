/*
 * commons.h
 *
 * Created: 24.10.2016 23:28:16
 *  Author: Lezh1k
 */ 

#ifndef COMMONS_H_
#define COMMONS_H_

#include <stdint.h>
#include <avr/io.h>

#define F_CPU       1000000UL

#define enable_adc_int() (ADCSRA |= (1 << ADIE))
#define disable_adc_int() (ADCSRA &= ~(1 << ADIE))  

#define enable_pcie_int() (GIMSK |= (1 << PCIE))
#define disable_pcie_int() (GIMSK &= ~(1 << PCIE))

#define enable_tim0_ovf_int() (TIMSK |= (1 << TOIE0))
#define disable_tim0_ovf_int() (TIMSK &= ~(1 << TOIE0))

#define enable_tim0_compa_int() (TIMSK |= (1 << OCIE0A))
#define disable_tim0_compa_int() (TIMSK &= ~(1 << OCIE0A))

#define enable_tim1_ovf_int() (TIMSK |= (1 << TOIE1))
#define disable_tim1_ovf_int() (TIMSK &= ~(1 << TOIE1))



#define nop() asm volatile("nop" "\n\t")

#define TERMISTOR_B		3380
#define TERMISTOR_RREF		10000
#define TERMISTOR_R0		10000
#define TERMISTOR_T0		25

#endif /* COMMONS_H_ */
