###############################################################################
# Makefile for s3c2440
###############################################################################

## General Flags
PROJECT = boot
CPU = arm920
TARGET = boot.elf
CC = arm-none-eabi-gcc
AS = arm-none-eabi-as
LD = arm-none-eabi-ld

## Options common to compile, link and assembly rules
COMMON = -mcpu=$(CPU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Os -fno-builtin -Wall

## Assembly specific flags
ASMFLAGS = $(COMMON)
# ASMFLAGS += -Wa,-gdwarf2

## Linker flags
# LDFLAGS = $(COMMON)
#LDFLAGS = -T armboot.lds 
#LDFLAGS +=  -Map=boot.map
LDFLAGS = -Wl,-T armboot.lds 
LDFLAGS +=  -Wl,-Map=boot.map

## Libraries
# LIBS = -lm -lc 
LIBS =  -nostdlib -lgcc -lc

## Objects that must be built in order to link
OBJECTS = start.o memsetup.o Flash.o board.o main.o
OBJECTS += serial.o dm9000x.o arp.o string.o net.o tftp.o console.o vsprintf.o

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) boot.bin boot.lss size

## Compile
board.o: ../board.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $< -o $@

serial.o: ../serial.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $< -o $@

dm9000x.o: ../dm9000x.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $< -o $@

start.o: ../start.s
	$(AS) $(INCLUDES) $(ASMFLAGS) $< -o $@

memsetup.o: ../memsetup.s
	$(AS) $(INCLUDES) $(ASMFLAGS) $< -o $@

arp.o: ../net/arp.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $< -o $@

string.o: ../string.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $< -o $@

net.o: ../net/net.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $< -o $@

Flash.o: ../Flash.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $< -o $@

tftp.o: ../net/tftp.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $< -o $@

console.o: ../console.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $< -o $@

vsprintf.o: ../vsprintf.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $< -o $@

main.o: ../main.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $< -o $@

# .c.s:
#	$(CC) $(CFLAGS) -S -o $*.s ./$< 	
# .s.o:
#	$(AS) $(ASMLAGS) -c-o $*.s ./$< 	

##Link 
$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET) 

%.bin: $(TARGET)
	arm-none-eabi-objcopy -O binary $< $@

%.lss: $(TARGET)
	arm-none-eabi-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@arm-none-eabi-size ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) boot.elf dep/* boot.bin *.s *.lss *.map


## Other dependencies
# -include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

