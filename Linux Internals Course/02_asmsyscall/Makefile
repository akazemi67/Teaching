
all: code code.o

code: code.o
	ld -g -static -e _start --stats -o code code.o

code.o: code.asm
	nasm -o code.o -f elf64 code.asm

clean: 
	rm -v code.o code


