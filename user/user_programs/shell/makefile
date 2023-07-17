USERLIB=../../user_tools

ASFLAGSOBJ= -O32 -f elf
NASM = nasm

CFLAGS= -std=c99 -Werror -Wall -O3 -ffreestanding -fleading-underscore -nostdlib -nostdinc -fno-builtin -I'$(USERLIB)/src/include'
CC= i686-elf-gcc

LDFLAGS= -T $(USERLIB)/user.ld -Map program.map -nostdinc -s --gc-sections --whole-archive
LD= i686-elf-ld

all: SHELL.ELF
ifeq ($(OS),Windows_NT)
	@echo [94mCleaning up...[0m
	@del *.o
else
	@printf "\033[1;34mCleaning up...\033[0m\n"
	@rm *.o
endif

SHELL.ELF: 	program.o
ifeq ($(OS),Windows_NT)
	@echo|set /p dummy="[94mLinking program[97m	==> $@...[0m"
else
	@printf "\033[1;34mLinking program\033[1;97m\t==> $@...\033[0m"
endif
	@$(LD) $(LDFLAGS) -L$(USERLIB) -luser $+ -o $@
ifeq ($(OS),Windows_NT)
	@echo [92mdone[0m
else
	@printf "\033[1;32mdone\033[0m\n"
endif

program.o: program.c
ifeq ($(OS),Windows_NT)
	@echo|set /p dummy="[94mCompiling[97m	==> $<...[0m"
else
	@printf "\033[1;34mCompiling\033[1;97m\t==> $<...\033[0m"
endif
	@$(CC) -c $(CFLAGS) $< -o $@
ifeq ($(OS),Windows_NT)
	@echo [92mdone[0m
else
	@printf "\033[1;32mdone\033[0m\n"
endif

%.o: %.asm
	$(NASM) $(ASFLAGSOBJ) $< -o $@

#    $<  Erste Abh�ngigkeit
#    $+  Liste aller Abh�ngigkeiten 
#    $@  Name des Targets
