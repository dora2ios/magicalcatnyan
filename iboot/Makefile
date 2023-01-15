CC		= xcrun -sdk iphoneos gcc

OBJCOPY	= /opt/homebrew/opt/binutils/bin/gobjcopy

CFLAGS	= -Iinclude/ -Iinclude/drivers/ -Iapple-include/ -DDER_TAG_SIZE=8 -target arm64-apple-ios12.0
CFLAGS	+= -Wall -Wno-incompatible-library-redeclaration -fno-stack-protector -nostdlib -static -nostdlibinc -Wl,-preload -Wl,-no_uuid
CFLAGS	+= -Wl,-e,start -Wl,-order_file,sym_order.txt -Wl,-image_base,0x18001C000 -Wl,-sectalign,__DATA,__common,0x8 -Wl,-segalign,0x8 -O0

OBJ		= tramhook

SOURCE	=	\
			tramhook.c \
			
.PHONY: all

all:
	cd ibootpatch2 && make
	gcc generate.c -o generate
	$(CC) entry.S $(SOURCE) $(CFLAGS) -o $(OBJ).o
	../../bin/vmacho -fM 0x80000 $(OBJ).o $(OBJ).prebin
	./generate $(OBJ).prebin $(OBJ).bin
	xxd -i $(OBJ).bin > $(OBJ).h
	-$(RM) $(OBJ).o
	-$(RM) $(OBJ).prebin
	
clean:
	cd ibootpatch2 && make clean
	-$(RM) $(OBJ).o
	-$(RM) $(OBJ).prebin
	-$(RM) $(OBJ).bin
	-$(RM) $(OBJ).h
	-$(RM) generate
	
