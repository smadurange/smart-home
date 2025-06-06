CC = avr-gcc
MCU = atmega328p
PORT = /dev/cuaU0
TARGET = client

SRC = client.c fpm.c nrfm.c util.c
OBJ = $(SRC:.c=.o)

CFLAGS = -std=gnu99
CFLAGS += -Os
CFLAGS += -Wall
CFLAGS += -mmcu=$(MCU)
CFLAGS += -DBAUD=57600
CFLAGS += -DF_CPU=16000000UL
CFLAGS += -DFPM_PWD=$(FPM_PWD)
CFLAGS += -ffunction-sections -fdata-sections

LDFLAGS = -mmcu=$(MCU)
LDFLAGS += -Wl,--gc-sections

HEX_FLAGS = -O ihex
HEX_FLAGS += -j .text -j .data

AVRDUDE_FLAGS = -p $(MCU)
AVRDUDE_FLAGS += -c arduino
AVRDUDE_FLAGS += -P $(PORT)
AVRDUDE_FLAGS += -D -U

%.o: %.c
	 $(CC) $(CFLAGS) -c -o $@ $<

elf: $(OBJ)
	 $(CC) $(LDFLAGS) $(OBJ) -o $(TARGET).elf

hex: elf
	 avr-objcopy $(HEX_FLAGS) $(TARGET).elf $(TARGET).hex

upload: hex
	 avrdude $(AVRDUDE_FLAGS) flash:w:$(TARGET).hex:i

.PHONY: clean

clean:
	 rm -f *.o *.elf *.hex 


