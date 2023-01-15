CC		= xcrun -sdk iphoneos gcc

OBJCOPY	= /opt/homebrew/opt/binutils/bin/gobjcopy

CFLAGS	= -Iinclude/ -Iinclude/drivers/ -Iapple-include/ -DDER_TAG_SIZE=8 -target arm64-apple-ios12.0
CFLAGS	+= -Wall -Wno-incompatible-library-redeclaration -fno-stack-protector -nostdlib -static -nostdlibinc -Wl,-preload -Wl,-no_uuid
CFLAGS	+= -Wl,-e,start -Wl,-order_file,sym_order.txt -Wl,-image_base,0x100000000 -Wl,-sectalign,__DATA,__common,0x8 -Wl,-segalign,0x4000

PKOKESHI_VERSION ?= 1.0.1-$(shell git rev-parse HEAD | cut -c1-8)

VERSIONFLAGS = -DKOKESHI_VERSION='"$(PKOKESHI_VERSION)"'


OBJ		= payload

SOURCE	=	\
			payload.c \
			offsetfinder.c \
			lib/memmem.c \
			lib/memcpy.c \
			lib/memset.c \
			lib/memcmp.c \
			lib/strcmp.c \
			lib/strchr.c \
			lib/strncmp.c \
			lib/strlen.c \
			lib/strcat.c \
			lib/strcpy.c \
			lib/strstr.c \
			lib/strtoull.c \
			drivers/dt/dtree.c \
			drivers/dt/dtree_getprop.c \
			drivers/fb/fb.c \
			kpf.c \
			shellcode.S \
			drivers/xnu/xnu.s \
			printf.c \
			
.PHONY: all

all:
	$(CC) entry.S $(SOURCE) $(CFLAGS) $(VERSIONFLAGS) -o $(OBJ).o
	../../bin/vmacho -fM 0x80000 $(OBJ).o $(OBJ).bin
	mv -v $(OBJ).bin magic
	xxd -i magic > magic.h
	mv -v magic $(OBJ).bin
	
clean:
	-$(RM) $(ASMSOURCE)
	-$(RM) $(OBJ).o
	-$(RM) $(OBJ).bin
	-$(RM) magic
	-$(RM) magic.h
