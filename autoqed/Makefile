CC = gcc
LD = ld

CFLAGS = -m32 -march=i386 -static \
	 -fno-builtin -fno-stack-protector -fno-omit-frame-pointer \
	 -Wall -I../lib -Os -fno-asynchronous-unwind-tables
LDFLAGS = -m elf_i386

UCFILES = $(shell find ./ -name "*.c")
LCFILES = $(shell find ../lib -name "*.c")
UOBJS = $(UCFILES:.c=.o) $(LCFILES:.c=.o)


autoqed.bin: $(UOBJS)
	$(LD) $(LDFLAGS) -e main -Ttext 0x8000000 -o autoqed.elf $(UOBJS)
	# @../utils/genApp.pl autoqed.elf

clean:
	rm -rf $(UOBJS) autoqed.elf
