all: main.elf

main.o: main.cpp
	arm-none-eabi-g++ -I. -mcpu=cortex-m4 -fno-exceptions -fno-rtti -o main.o -c main.cpp

syscalls.o: syscalls.c
	arm-none-eabi-gcc -I. -mcpu=cortex-m4 -fno-exceptions -o syscalls.o -c syscalls.c

startup.o: startup.c
	arm-none-eabi-gcc -I. -mcpu=cortex-m4 -fno-exceptions -o startup.o -c startup.c

unwind.o: unwind.c
	arm-none-eabi-gcc -I. -mcpu=cortex-m4 -fno-exceptions -o unwind.o -c unwind.c

main.elf: main.o syscalls.o startup.o unwind.o
	arm-none-eabi-g++ -I. -mcpu=cortex-m4 -nostdlib -o main.elf main.o syscalls.o startup.o unwind.o -T linker_script.ld

clean:
	rm -f *.o *.elf
