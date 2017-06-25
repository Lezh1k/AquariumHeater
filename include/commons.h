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

#define enable_tim1_ovf_int() (TIMSK |= (1 << TOIE1))
#define disable_tim1_ovf_int() (TIMSK &= ~(1 << TOIE1))

#define enable_tim1_compA_int() (TIMSK |= (1 << OCIE1A))
#define enable_tim1_compB_int() (TIMSK |= (1 << OCIE1B))

#define disable_tim1_compA_int() (TIMSK &= ~(1 << OCIE1A))
#define disable_tim1_compB_int() (TIMSK &= ~(1 << OCIE1B))

#define nop() asm volatile("nop" "\n\t")

#endif /* COMMONS_H_ */
