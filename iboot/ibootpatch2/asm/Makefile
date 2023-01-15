CC		= xcrun -sdk iphoneos clang
OBJCOPY	= /opt/homebrew/opt/binutils/bin/gobjcopy
CFLAGS	= -target arm64-apple-darwin -Wall
			
.PHONY: all

all:
	-$(RM) shellcode.h
	$(CC) $(CFLAGS) shellcode.s -o shellcode.o
	$(OBJCOPY) -O binary -j .text shellcode.o shellcode.bin
	xxd -i shellcode.bin > shellcode.h
	-$(RM) shellcode.o
	
clean:
	-$(RM) shellcode.h
	-$(RM) shellcode.o
	-$(RM) shellcode.bin
