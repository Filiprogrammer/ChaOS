#!/bin/sh

error() {
    printf "\033[1;31mError\033[0m\n"
    exit 1
}

cd "$(dirname "$(readlink -f "$0")")"
./clean.sh
mkdir obj 2>/dev/null

CFLAGS="-g -fshort-wchar -fno-builtin -fno-strict-aliasing -Wall -Werror -Wno-array-bounds -ffunction-sections -fdata-sections -fno-common -m32 -march=i586 -malign-double -fno-stack-protector -D EFI32 -fno-asynchronous-unwind-tables -Wno-address -fno-pic -fno-pie -Os -D DISABLE_NEW_DEPRECATED_INTERFACES -c -Ilib/MdePkg/Include -Ilib/MdePkg/Include/Ia32"

printf "\033[1;34mCompiling\033[1;97m\t==> AutoGen.c...\033[0m"
i686-elf-gcc $CFLAGS -include src/AutoGen.h -o obj/AutoGen.obj src/AutoGen.c || error
printf "\033[1;32mdone\033[0m\n"
printf "\033[1;34mCompiling\033[1;97m\t==> efiboot.c...\033[0m"
i686-elf-gcc $CFLAGS -o obj/efiboot.obj src/efiboot.c || error
printf "\033[1;32mdone\033[0m\n"

for objfile in $(find obj -name '*.obj'); do
    echo "\"$(pwd)/$objfile\"" >> obj/object_files.lst
done

printf "\033[1;34mCreating Archive\033[1;97m\t==> efiboot.lib...\033[0m"
i686-elf-ar cr efiboot.lib @obj/object_files.lst || error
printf "\033[1;32mdone\033[0m\n"

printf "\033[1;34mLinking\033[1;97m\t==> efiboot.dll...\033[0m"
i686-elf-gcc -o efiboot.dll -nostdlib -Wl,-n,-q,--gc-sections -Wl,--entry,_ModuleEntryPoint -u _ModuleEntryPoint -Wl,-Map,efiboot.map,--whole-archive -Os -Wl,-m,elf_i386,--oformat=elf32-i386 -Wl,--start-group,@static_library_files.lst,--end-group -g -fshort-wchar -fno-builtin -fno-strict-aliasing -Wall -Werror -Wno-array-bounds -ffunction-sections -fdata-sections -fno-common -m32 -march=i586 -malign-double -fno-stack-protector -D EFI32 -fno-asynchronous-unwind-tables -Wno-address -fno-pic -fno-pie -Os -D DISABLE_NEW_DEPRECATED_INTERFACES -Wl,--defsym=PECOFF_HEADER_SIZE=0x220 -Wl,--script=GccBase.lds -Wno-error || error
printf "\033[1;32mdone\033[0m\n"

printf "\033[1;34mGenerating EFI App\033[1;97m\t==> BOOTIA32.EFI...\033[0m"
i686-elf-objcopy efiboot.dll || error
cp efiboot.dll efiboot.debug 2>/dev/null
i686-elf-objcopy --strip-unneeded -R .eh_frame efiboot.dll || error
i686-elf-objcopy --add-gnu-debuglink=efiboot.debug efiboot.dll || error
GenFw -e UEFI_APPLICATION -o BOOTIA32.EFI efiboot.dll || error
printf "\033[1;32mdone\033[0m\n"
