/*
 * max7219.c
 *
 * Created: 25.10.2016 18:05:29
 *  Author: Lezh1k
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>
#include "commons.h"
#include "max7219.h"

#define SPI_DDR       DDRB
#define SPI_PORT      PORTB
#define SPI_PIN       PINB

#define SPI_MOSI      (1<<PB3)
#define SPI_CS        (1<<PB4)
#define SPI_SCK       (1<<PB2)

#define HEATER_PORT		SPI_SCK

void spi_send(uint8_t sb);
void spi_max7219_send(uint16_t w);      

void
spi_send(uint8_t sb) {
  register int8_t i = 8;
  do {
    if (sb & 0x80) SPI_PORT |= SPI_MOSI;
    else SPI_PORT &= ~SPI_MOSI;
    SPI_PORT |= SPI_SCK; //strobe high
    sb <<= 1;
    SPI_PORT &= ~SPI_SCK; //strobe low
  } while (--i);
}
//////////////////////////////////////////////////////////////////////////

void 
spi_max7219_send(uint16_t w) {
  SPI_PORT &= ~SPI_CS;
	spi_send(w >> 8);
  spi_send(w & 0x00ff);
  SPI_PORT |= SPI_CS;
}
//////////////////////////////////////////////////////////////////////////

void 
max7219_init() {
  SPI_DDR = SPI_MOSI | SPI_CS | SPI_SCK;
  SPI_PORT = SPI_MOSI | SPI_CS ;							
	
	spi_max7219_send(0x0c01); //normal operation
	spi_max7219_send(0x0900); //decode mode disable
	spi_max7219_send(0x0f00); //test disable				   
  spi_max7219_send(0x0b03); //symbols count = 4	   
  spi_max7219_send(0x0a0f); //brightness level is quater of max (0f is max)
  //
  //// - - - - 
  spi_max7219_send(0x0101);
  spi_max7219_send(0x0201);
  spi_max7219_send(0x0301);
  spi_max7219_send(0x0401);
}
//////////////////////////////////////////////////////////////////////////

enum {SC_G = 1, SC_F = 1 << 1, SC_E = 1 << 2, SC_D = 1 << 3,
SC_C = 1 << 4, SC_B = 1 << 5, SC_A = 1 << 6, SC_DP = 1 << 7};

const uint8_t codes[10] PROGMEM= {
	SC_A | SC_B | SC_C | SC_D | SC_E | SC_F, // 0
	SC_B | SC_C, //1
	SC_A | SC_B | SC_G | SC_E | SC_D, //2
	SC_A | SC_B | SC_G | SC_C | SC_D, //3
	SC_F | SC_G | SC_B | SC_C, //4
	SC_A | SC_F | SC_G | SC_C | SC_D, //5
	SC_A | SC_F | SC_G | SC_E | SC_D | SC_C, //6
	SC_A | SC_B | SC_C, //7
	SC_A | SC_B | SC_C | SC_D | SC_E | SC_F | SC_G, //8
	SC_A | SC_B | SC_C | SC_D | SC_F | SC_G, //9
};

const uint8_t addr[] PROGMEM = {0x01, 0x02, 0x03, 0x04};

// 0bDP_A_B_C_D_E_F_G
void 
max7219_set_symbol(max7219_screen_t pos, 
                   uint8_t sym) {
	register uint8_t spi_port = SPI_PORT;
	register uint8_t ipos = pos*2;
  uint16_t s = (pgm_read_byte(&addr[ipos+1]) << 8) | pgm_read_byte(&codes[sym%10]);
	SPI_PORT &= ~HEATER_PORT;
  spi_max7219_send(s);
  sym /= 10;
  s = (pgm_read_byte(&addr[ipos]) << 8) | pgm_read_byte(&codes[sym%10]);
  spi_max7219_send(s);
	SPI_PORT = spi_port;
}
//////////////////////////////////////////////////////////////////////////

void 
max7219_turn_heater(max7219_heater_state_t state) {
	if (state == ON) SPI_PORT |= HEATER_PORT;
	else SPI_PORT &= ~HEATER_PORT;
}

//////////////////////////////////////////////////////////////////////////