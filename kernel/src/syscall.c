#include "syscall.h"

DEFN_SYSCALL1( puts,                       0, char*                     )
DEFN_SYSCALL1( putch,                      1, char                      )
DEFN_SYSCALL2( settextcolor,               2, uint8_t, uint8_t          )
DEFN_SYSCALL0( getpid,                     3                            )
DEFN_SYSCALL1( sleepCurrentThread,         4, uint32_t                  )
DEFN_SYSCALL0( switch_context,             5                            )
DEFN_SYSCALL1( keyboard_isKeyDown,         6, KEY_t                     )
DEFN_SYSCALL0( getkey,                     7                            )
DEFN_SYSCALL1( video_set_mode,             8, uint8_t                   )
DEFN_SYSCALL2( put_pixel,                  9, int16_t, int16_t          )
DEFN_SYSCALL1( file_createDirectory,      10, char*                     )
DEFN_SYSCALL2( file_create,               11, file_t*, char*            )
DEFN_SYSCALL0( move_cursor_left,          12                            )
DEFN_SYSCALL0( move_cursor_right,         13                            )
DEFN_SYSCALL3( file_findByIndex,          14, file_t*, char*, uint32_t  )
DEFN_SYSCALL1( file_isDirectory,          15, char*                     )
DEFN_SYSCALL0( desktop_enable,            16                            )
DEFN_SYSCALL2( file_find,                 17, file_t* ,char*            )
DEFN_SYSCALL4( file_readContents,         18, file_t*, uint8_t*, uint32_t, uint32_t)
DEFN_SYSCALL0( reboot,                    19                            )
DEFN_SYSCALL3( draw_picture,              20, uint8_t*, int16_t, int16_t)
DEFN_SYSCALL1( file_delete,               21, char*                     )
DEFN_SYSCALL1( keyToASCII,                22, KEY_t                     )
DEFN_SYSCALL0( timer_getMilliseconds,     23                            )
DEFN_SYSCALL0( random,                    24                            )
DEFN_SYSCALL1( fileManage_getBootPath,    25, char*                     )
DEFN_SYSCALL0( exitCurrentTask,           26                            )
DEFN_SYSCALL1( file_execute,              27, char*                     )
DEFN_SYSCALL2( pci_getDevice,             28, uint32_t, pciDev_t*       )
DEFN_SYSCALL1( create_thread,             29, void*                     )
DEFN_SYSCALL0( exitCurrentThread,         30                            )

static void* syscalls[] = {
    &puts,
    &putch,
    &settextcolor,
    &getpid,
    &sleepCurrentThread,
    &switch_context,
    &keyboard_isKeyDown,
    &getkey,
    &video_set_mode,
    &put_pixel,
    &file_createDirectory,
    &file_create,
    &move_cursor_left,
    &move_cursor_right,
    &file_findByIndex,
    &file_isDirectory,
    &desktop_enable,
    &file_find,
    &file_readContents,
    &reboot,
    &draw_picture,
    &file_delete,
    &keyToASCII,
    &timer_getMilliseconds,
    &random,
    &fileManage_getBootPath,
    &exitCurrentTask,
    &file_execute,
    &pci_getDevice,
    &create_thread,
    &exitCurrentThread};

void syscall_handler(registers_t* r) {
    // Firstly, check if the requested syscall number is valid. The syscall number is found in EAX.
    if (r->eax >= sizeof(syscalls))
        return;

    void* addr = syscalls[r->eax];  // Get the required syscall location.

    // Unknown number of parameters, so just push all of them onto the stack in the correct order.
    // The function will use all the parameters it wants, and we can pop them all back off afterwards.
    __asm__ volatile(
        "push %1; \
        push %2; \
        push %3; \
        push %4; \
        push %5; \
        call *%6; \
        add $20, %%esp;"
        : "=a"(r->eax)
        : "D"(r->edi), "S"(r->esi), "d"(r->edx), "c"(r->ecx), "b"(r->ebx), "a"(addr));
}

void syscall_install() {
    irq_install_handler(0x7F - 32, &syscall_handler);
}
