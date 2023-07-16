ifeq ($(TEST), 1)
SOURCESTEST = $(wildcard src-test/*.c)
OBJECTSTEST = $(addprefix obj-test/,$(addsuffix .o,$(notdir $(basename $(SOURCESTEST)))))
endif

SOURCES = src/kernel.asm $(filter-out src/kernel.asm ,$(wildcard src/*.asm src/*.c))
OBJECTS = $(addprefix obj/,$(addsuffix .o,$(notdir $(basename $(SOURCES)))))

ASFLAGSOBJ= -O32 -f elf
ASFLAGSOBJDEBUG= -O32 -f elf -g
NASM = nasm

CFLAGS= -std=gnu99 -march=i386 -mtune=i386 -Werror -Wall -O -ffreestanding -fleading-underscore -nostdlib -nostdinc -fno-builtin -fno-stack-protector -Isrc/include
CFLAGSDEBUG= -std=gnu99 -march=i386 -mtune=i386 -Werror -Wall -O -ffreestanding -fleading-underscore -nostdlib -nostdinc -fno-builtin -fno-stack-protector -Isrc/include -ggdb
CC= i686-elf-gcc

LDFLAGS= -T kernel.ld -Map kernel.map -nostdinc
LDFLAGSDEBUG = -T kernel_debug.ld -Map kernel.map -nostdinc
LD= i686-elf-ld

OBJCOPY= i686-elf-objcopy

obj/%.o: src/%.asm
ifeq ($(OS),Windows_NT)
	@cecho {09}Assembling{0F}{\t}==^> $<...{#}
else
	@printf "\033[1;34mAssembling\033[1;97m\t==> $<...\033[0m"
endif
ifeq ($(DEBUG), 1)
	@$(NASM) $(ASFLAGSOBJDEBUG) $< -o $@
else
	@$(NASM) $(ASFLAGSOBJ) $< -o $@
endif
ifeq ($(OS),Windows_NT)
	@cecho .{0a}done{#}{\n}
else
	@printf "\033[1;32mdone\033[0m\n"
endif

obj/%.o: src/%.c
ifeq ($(OS),Windows_NT)
	@cecho {09}Compiling{0F}{\t}==^> $<...{#}
else
	@printf "\033[1;34mCompiling\033[1;97m\t==> $<...\033[0m"
endif
ifeq ($(TEST), 1)
ifeq ($(DEBUG), 1)
	@$(CC) $(CFLAGSDEBUG) "-D__VERSION_STRING=\"$(VERSION_STRING)\"" -Isrc-test/include -D__TEST -c -o $@ $<
else
	@$(CC) $(CFLAGS) "-D__VERSION_STRING=\"$(VERSION_STRING)\"" -Isrc-test/include -D__TEST -c -o $@ $<
endif
else
ifeq ($(DEBUG), 1)
	@$(CC) $(CFLAGSDEBUG) "-D__VERSION_STRING=\"$(VERSION_STRING)\"" -c -o $@ $<
else
	@$(CC) $(CFLAGS) "-D__VERSION_STRING=\"$(VERSION_STRING)\"" -c -o $@ $<
endif
endif
ifeq ($(OS),Windows_NT)
	@cecho .{0a}done{#}{\n}
else
	@printf "\033[1;32mdone\033[0m\n"
endif

obj-test/%.o: src-test/%.c
ifeq ($(OS),Windows_NT)
	@cecho {09}Compiling test{0F}{\t}==^> $<...{#}
else
	@printf "\033[1;34mCompiling test\033[1;97m\t==> $<...\033[0m"
endif
ifeq ($(DEBUG), 1)
	@$(CC) $(CFLAGSDEBUG) -Isrc-test/include -c -o $@ $<
else
	@$(CC) $(CFLAGS) -Isrc-test/include -c -o $@ $<
endif
ifeq ($(OS),Windows_NT)
	@cecho .{0a}done{#}{\n}
else
	@printf "\033[1;32mdone\033[0m\n"
endif

ifeq ($(TEST), 1)
CHAOSKRN.SYS: $(OBJECTS) $(OBJECTSTEST)
else
CHAOSKRN.SYS: $(OBJECTS)
endif
ifeq ($(DEBUG), 1)
ifeq ($(OS),Windows_NT)
	@cecho {09}Linking Kernel{0F}{\t}==^> CHAOSKRN.SYS_debug...{#}
else
	@printf "\033[1;34mLinking Kernel\033[1;97m\t==> CHAOSKRN.SYS_debug...\033[0m"
endif
	@$(LD) $(LDFLAGSDEBUG) $+ -o $@_debug
ifeq ($(OS),Windows_NT)
	@cecho .{0a}done{#}{\n}
	@cecho {09}Stripping CHAOSKRN.SYS_debug{0F}{\t}==^> CHAOSKRN.SYS...{#}
else
	@printf "\033[1;32mdone\033[0m\n"
	@printf "\033[1;34mStripping CHAOSKRN.SYS_debug\033[1;97m\t==> CHAOSKRN.SYS...\033[0m"
endif
	@$(OBJCOPY) -O binary $@_debug $@
ifeq ($(OS),Windows_NT)
	@cecho .{0a}done{#}{\n}
else
	@printf "\033[1;32mdone\033[0m\n"
endif
else
ifeq ($(OS),Windows_NT)
	@cecho {09}Linking Kernel{0F}{\t}==^> CHAOSKRN.SYS...{#}
else
	@printf "\033[1;34mLinking Kernel\033[1;97m\t==> CHAOSKRN.SYS...\033[0m"
endif
	@$(LD) $(LDFLAGS) $+ -o $@
ifeq ($(OS),Windows_NT)
	@cecho .{0a}done{#}{\n}
else
	@printf "\033[1;32mdone\033[0m\n"
endif
endif
