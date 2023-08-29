CC = gcc
LD = ld

CFLAGS = -m32 -march=i386 -static \
	 -fno-builtin -fno-stack-protector -fno-omit-frame-pointer \
	 -Wall -Os -I../lib
LDFLAGS = -m elf_i386

UCFILES = $(shell find ./ -name "*.c")
LCFILES = $(shell find ../lib -name "*.c")
UOBJS = $(UCFILES:.c=.o) $(LCFILES:.c=.o)

app.bin: $(UOBJS)
	$(LD) $(LDFLAGS) -e xqcl -Ttext 0x8000000 -o app.elf $(UOBJS)
	# @../utils/genApp.pl app.elf

clean:
	rm -rf $(UOBJS) app.elf
