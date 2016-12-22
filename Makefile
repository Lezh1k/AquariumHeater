AVR_TOOLCHAIN_PATH = ~/avr8-gnu-toolchain-linux_x86_64
AVR_TOOLCHAIN_BIN = $(AVR_TOOLCHAIN_PATH)/bin
AVR_PREFIX = $(AVR_TOOLCHAIN_BIN)/avr-

CC = $(AVR_PREFIX)gcc
OBJCOPY = $(AVR_PREFIX)objcopy
OBJDUMP = $(AVR_PREFIX)objdump
AR = $(AVR_PREFIX)ar
NM = $(AVR_PREFIX)nm
AS = $(AVR_PREFIX)as
LD = $(AVR_PREFIX)ld
SIZE = $(AVR_PREFIX)size

LIBS = 
DEFS = -DNDEBUG -D__AVR_ATtiny25__ 

# Directories
INCLUDE_DIR = include
BUILD_DIR = build
SRC_DIR = src
BIN_DIR = bin

#device and program
PRG = heater
MMCU = -mmcu=attiny25
OPTIMIZE = -Os

INCLUDES = -Iinclude -I$(AVR_TOOLCHAIN_PATH)/avr/include

override CFLAGS = $(INCLUDES) $(MMCU) $(OPTIMIZE) $(DEFS) -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -std=gnu99 \
  -Wl,--start-group  -Wl,--end-group -Wl,--gc-sections

override LDFLAGS = -Wl,-Map,$(BIN_DIR)/$(PRG).map $(MMCU)

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS	= $(patsubst %,$(BUILD_DIR)/%.o, $(subst src/,,$(subst .c,,$(SOURCES))))

all: directories heater hex_size
heater: $(BIN_DIR)/$(PRG).elf $(BIN_DIR)/lst $(BIN_DIR)/text 

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -Wall $(CFLAGS) -c $< -o $@

$(BIN_DIR)/$(PRG).elf: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(BIN_DIR)/lst: $(BIN_DIR)/$(PRG).lst
$(BIN_DIR)/%.lst: $(BIN_DIR)/%.elf 
	$(OBJDUMP) -h -S $< > $@

$(BIN_DIR)/text: $(BIN_DIR)/hex $(BIN_DIR)/bin

$(BIN_DIR)/hex: $(BIN_DIR)/$(PRG).hex
$(BIN_DIR)/%.hex: $(BIN_DIR)/%.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

$(BIN_DIR)/bin: $(BIN_DIR)/$(PRG).bin
$(BIN_DIR)/%.bin: $(BIN_DIR)/%.elf
	$(OBJCOPY) -j .text -j .data -O binary $< $@

hex_size:
	$(SIZE) $(BIN_DIR)/$(PRG).elf

directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BIN_DIR)

clean:
	@rm -rf $(BUILD_DIR)/*
	@rm -rf $(BIN_DIR)/*

mrproper:
	@rm -rf $(BUILD_DIR) 
	@rm -rf $(BIN_DIR)

program:
	@avrdude -c usbasp -p t25 -U flash:w:$(BIN_DIR)/$(PRG).hex

fuse_program:
	@avrdude -c usbasp -p t25 -U lfuse:w:0x62:m -U hfuse:w:0x5f:m -U efuse:w:0xff:m 

