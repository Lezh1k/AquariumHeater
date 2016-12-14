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
#include <util/delay.h>

#include "max7219.h"

static volatile uint8_t int_tim0_ovf = 0;
static volatile uint8_t int_adc = 0;
static volatile uint8_t int_pcint0 = 0;

static volatile uint8_t DST_T = 35;
static volatile uint8_t CUR_T = 0;

#define PIN_BTN_UP (1 << PINB3)
#define PIN_BTN_DOWN (1 << PINB4)
#define PORT_BTN_UP (1 << PB3)
#define PORT_BTN_DOWN (1 << PB4)
//////////////////////////////////////////////////////////////////////////

static void handle_sint_adc();
static inline void print_dest_temperature();
static inline void print_current_temperature();
uint8_t adcw_to_temperature(uint16_t adc_avg);

static inline void start_adc() {
  ADCSRA |= (1 << ADSC);
}
//////////////////////////////////////////////////////////////////////////

ISR(TIMER0_OVF_vect) {
  int_tim0_ovf = 1;
  nop();
}
//////////////////////////////////////////////////////////////////////////

ISR(ADC_vect) {
  int_adc = 1;
  nop();
}
//////////////////////////////////////////////////////////////////////////

ISR(PCINT0_vect) {
  disable_pcie_int();
  int_pcint0 = 1;
  nop();
}
//////////////////////////////////////////////////////////////////////////


int
main(void) {
  enum {TIM0_OVF_CNT = 3};
  register int8_t tim0_ovf_cnt = TIM0_OVF_CNT;
  register uint8_t old_t = 0;
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

    if (int_adc) {
      handle_sint_adc();
      max7219_turn_heater(CUR_T < DST_T);
      int_adc = 0;
      start_adc();
    }

    if (int_pcint0) {
      _delay_ms(70); //todo use timer for this
      if (!(PINB & PIN_BTN_UP))
        ++DST_T;
      if (!(PINB & PIN_BTN_DOWN))
        --DST_T;
      print_dest_temperature();
      max7219_turn_heater(CUR_T < DST_T);
      int_pcint0 = 0;
      enable_pcie_int();
    }

    if (int_tim0_ovf) {
      int_tim0_ovf = 0;
      if (!(--tim0_ovf_cnt)) {
        tim0_ovf_cnt = TIM0_OVF_CNT;
        if (old_t != CUR_T) {
          print_current_temperature();
          old_t = CUR_T;
        }
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

//change this table if TERMISTOR_B, TERMISTOR_RREF, TERMISTOR_R0 or TERMISTOR_T0 is changed
static const int16_t adc_temperature[] PROGMEM= {
  4939, 3406, 2883, 2584, 2381, 2229, 2108, 2010, 1927, 1855, 1793, 1737, 1687, 1642, 1601, 1563,
  1528, 1496, 1465, 1437, 1410, 1385, 1362, 1339, 1318, 1298, 1278, 1260, 1242, 1225, 1209, 1193,
  1178, 1163, 1149, 1135, 1122, 1109, 1097, 1085, 1073, 1062, 1051, 1040, 1030, 1020, 1010, 1000,
  991, 982, 972, 964, 955, 947, 938, 930, 922, 914, 907, 899, 892, 885, 878, 871,
  864, 857, 851, 844, 838, 831, 825, 819, 813, 807, 801, 795, 790, 784, 779, 773,
  768, 763, 757, 752, 747, 742, 737, 732, 727, 722, 718, 713, 708, 704, 699, 695,
  690, 686, 681, 677, 673, 669, 664, 660, 656, 652, 648, 644, 640, 636, 632, 629,
  625, 621, 617, 613, 610, 606, 603, 599, 595, 592, 588, 585, 581, 578, 575, 571,
  568, 564, 561, 558, 555, 551, 548, 545, 542, 539, 535, 532, 529, 526, 523, 520,
  517, 514, 511, 508, 505, 502, 499, 496, 493, 491, 488, 485, 482, 479, 477, 474,
  471, 468, 466, 463, 460, 457, 455, 452, 449, 447, 444, 441, 439, 436, 434, 431,
  429, 426, 423, 421, 418, 416, 413, 411, 408, 406, 404, 401, 399, 396, 394, 391,
  389, 387, 384, 382, 379, 377, 375, 372, 370, 368, 365, 363, 361, 358, 356, 354,
  352, 349, 347, 345, 343, 340, 338, 336, 334, 331, 329, 327, 325, 323, 320, 318,
  316, 314, 312, 309, 307, 305, 303, 301, 299, 297, 294, 292, 290, 288, 286, 284,
  282, 280, 278, 275, 273, 271, 269, 267, 265, 263, 261, 259, 257, 255, 253, 251,
  248, 246, 244, 242, 240, 238, 236, 234, 232, 230, 228, 226, 224, 222, 220, 218,
  216, 214, 212, 210, 208, 206, 204, 202, 200, 198, 196, 194, 192, 190, 188, 186,
  184, 182, 180, 178, 176, 174, 172, 170, 168, 166, 164, 162, 160, 158, 156, 154,
  152, 150, 148, 146, 144, 142, 140, 138, 136, 134, 132, 130, 128, 126, 124, 122,
  120, 118, 116, 114, 112, 110, 108, 106, 104, 102, 100, 98, 96, 94, 92, 90,
  88, 86, 84, 81, 79, 77, 75, 73, 71, 69, 67, 65, 63, 61, 59, 57,
  55, 52, 50, 48, 46, 44, 42, 40, 38, 36, 33, 31, 29, 27, 25, 23,
  21, 18, 16, 14, 12, 10, 7, 5, 3, 1, -1, -3, -5, -7, -10, -12,
  -14, -16, -19, -21, -23, -26, -28, -30, -33, -35, -37, -40, -42, -44, -47, -49,
  -52, -54, -57, -59, -62, -64, -67, -69, -72, -74, -77, -79, -82, -84, -87, -90,
  -92, -95, -98, -100, -103, -106, -109, -111, -114, -117, -120, -123, -125, -128, -131, -134,
  -137, -140, -143, -146, -149, -152, -155, -158, -162, -165, -168, -171, -175, -178, -181, -185,
  -188, -191, -195, -199, -202, -206, -209, -213, -217, -221, -224, -228, -232, -236, -240, -245,
  -249, -253, -257, -262, -266, -271, -275, -280, -285, -290, -295, -300, -305, -311, -316, -322,
  -327, -333, -339, -345, -352, -358, -365, -372, -379, -386, -394, -402, -410, -419, -428, -438,
  -448, -458, -469, -481, -494, -508, -523, -539, -557, -577, -600, -627, -659, -701, -762, -881,
};

uint8_t adcw_to_temperature(uint16_t adc_avg) {
  int16_t f, l, r;
  f= pgm_read_word(&adc_temperature[adc_avg / 2]);
  l= pgm_read_word(&adc_temperature[adc_avg / 2 + 1]);
  r = adc_avg % 2 ? f/2+l/2 : f;
  return r / 10;
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
