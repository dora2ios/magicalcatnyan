CC		= xcrun -sdk iphoneos gcc

OBJCOPY	= /opt/homebrew/opt/binutils/bin/gobjcopy

CFLAGS	= -Iinclude/ -Iinclude/drivers/ -DDER_TAG_SIZE=8 -target arm64-apple-ios12.0
CFLAGS	+= -Wall -Wno-incompatible-library-redeclaration -fno-stack-protector -nostdlib -static -nostdlibinc -Wl,-preload -Wl,-no_uuid -O0
CFLAGS	+= -Wl,-e,start -Wl,-order_file,sym_order.txt -Wl,-image_base,0x180018000 -Wl,-sectalign,__DATA,__common,0x8 -Wl,-segalign,0x8

OBJ		= ibootpatch2

SOURCE	=	\
			ibootpatch2.c \
			offsetfinder.c \
			lib/memset.c \
			lib/memmem.c \
			lib/memcmp.c \
			lib/memcpy.c \
			
.PHONY: all

all:
	cd asm && make
	gcc generate.c -o generate
	$(CC) entry.S $(SOURCE) $(CFLAGS) -o $(OBJ).o
	../../../bin/vmacho -fM 0x80000 $(OBJ).o $(OBJ).prebin
	./generate $(OBJ).prebin $(OBJ).bin
	xxd -i $(OBJ).bin > $(OBJ).h
	-$(RM) $(OBJ).o
	-$(RM) $(OBJ).prebin
	
clean:
	cd asm && make clean
	-$(RM) generate
	-$(RM) $(OBJ).o
	-$(RM) $(OBJ).h
	-$(RM) $(OBJ).bin
	-$(RM) $(OBJ).prebin
	
