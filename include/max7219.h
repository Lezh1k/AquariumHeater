/*
 * max7219.h
 *
 * Created: 25.10.2016 18:05:17
 *  Author: Lezh1k
 */ 


#ifndef MAX7219_H_
#define MAX7219_H_           

#include <stdint.h>

typedef enum max7219_screen {
  MS_0 = 0x01,
  MS_1 = 0x03
} max7219_screen_t;

void max7219_init();
void max7219_set_symbol(max7219_screen_t pos, int16_t sym);

typedef enum max7219_heater_state {
  OFF = 0, ON = 1
} max7219_heater_state_t;

void
max7219_turn_heater(max7219_heater_state_t state);

#endif /* MAX7219_H_ */
