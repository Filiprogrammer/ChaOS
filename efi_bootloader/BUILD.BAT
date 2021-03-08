@echo off
cd %~d0%~p0
call CLEAN.BAT
mkdir obj >nul 2>&1

set CFLAGS=-g -fshort-wchar -fno-builtin -fno-strict-aliasing -Wall -Werror -Wno-array-bounds -ffunction-sections -fdata-sections -fno-common -m32 -march=i586 -malign-double -fno-stack-protector -D EFI32 -fno-asynchronous-unwind-tables -Wno-address -fno-pic -fno-pie -Os -D DISABLE_NEW_DEPRECATED_INTERFACES -c -Ilib/MdePkg/Include -Ilib/MdePkg/Include/Ia32

cecho {09}Compiling{0F}{\t}==^> AutoGen.c...{#}
i686-elf-gcc %CFLAGS% -include src/AutoGen.h -o obj/AutoGen.obj src/AutoGen.c || goto ERROR
cecho {0a}done{#}{\n}
cecho {09}Compiling{0F}{\t}==^> efiboot.c...{#}
i686-elf-gcc %CFLAGS% -o obj/efiboot.obj src/efiboot.c || goto ERROR
cecho {0a}done{#}{\n}

setlocal EnableDelayedExpansion

FOR /R obj %%G IN (*.obj) DO (
    set "xyz="%%G^""
    ECHO !xyz:\=/!>>obj\object_files.lst
)

endlocal

cecho {09}Creating Archive{0F}{\t}==^> efiboot.lib...{#}
i686-elf-ar cr efiboot.lib @obj/object_files.lst || goto ERROR
cecho {0a}done{#}{\n}

for /F "tokens=*" %%A in (static_library_files.lst) do (
    if not exist %%A (
        set "a=%%A"
        call set "a=%%a:/=\%%"
        call pushd %%a%%\..
        call BUILD.BAT
        popd
    )
)

cecho {09}Linking{0F}{\t}==^> efiboot.dll...{#}
i686-elf-gcc -o efiboot.dll -nostdlib -Wl,-n,-q,--gc-sections -Wl,--entry,_ModuleEntryPoint -u _ModuleEntryPoint -Wl,-Map,efiboot.map,--whole-archive -Os -Wl,-m,elf_i386,--oformat=elf32-i386 -Wl,--start-group,@static_library_files.lst,--end-group -g -fshort-wchar -fno-builtin -fno-strict-aliasing -Wall -Werror -Wno-array-bounds -ffunction-sections -fdata-sections -fno-common -m32 -march=i586 -malign-double -fno-stack-protector -D EFI32 -fno-asynchronous-unwind-tables -Wno-address -fno-pic -fno-pie -Os -D DISABLE_NEW_DEPRECATED_INTERFACES -Wl,--defsym=PECOFF_HEADER_SIZE=0x220 -Wl,--script=GccBase.lds -Wno-error || goto ERROR
cecho {0a}done{#}{\n}

cecho {09}Generating EFI App{0F}{\t}==^> BOOTIA32.EFI...{#}
i686-elf-objcopy efiboot.dll || goto ERROR
copy efiboot.dll efiboot.debug >nul 2>&1
i686-elf-objcopy --strip-unneeded -R .eh_frame efiboot.dll || goto ERROR
i686-elf-objcopy --add-gnu-debuglink=efiboot.debug efiboot.dll || goto ERROR
GenFw.exe -e UEFI_APPLICATION -o BOOTIA32.EFI efiboot.dll || goto ERROR
cecho {0a}done{#}{\n}

goto EOF

:ERROR
cecho {0C}Error{#}{\n}
EXIT /B 1

:EOF
