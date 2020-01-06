#ifndef SYSCALL_H
#define SYSCALL_H

#include "os.h"
#include "fileManager.h"
#include "keyboard.h"
#include "pci.h"

void syscall_handler(registers_t* regs);
void syscall_install();

#define DECL_SYSCALL0(fn)                int syscall_##fn();
#define DECL_SYSCALL1(fn,p1)             int syscall_##fn(p1);
#define DECL_SYSCALL2(fn,p1,p2)          int syscall_##fn(p1,p2);
#define DECL_SYSCALL3(fn,p1,p2,p3)       int syscall_##fn(p1,p2,p3);
#define DECL_SYSCALL4(fn,p1,p2,p3,p4)    int syscall_##fn(p1,p2,p3,p4);
#define DECL_SYSCALL5(fn,p1,p2,p3,p4,p5) int syscall_##fn(p1,p2,p3,p4,p5);

#define DEFN_SYSCALL0(fn, num) \
int syscall_##fn() \
{ \
  int a; \
  __asm__ volatile("int $0x7F" : "=a" (a) : "0" (num)); \
  return a; \
}

#define DEFN_SYSCALL1(fn, num, P1) \
int syscall_##fn(P1 p1) \
{ \
  int a; \
  __asm__ volatile("int $0x7F" : "=a" (a) : "0" (num), "b" ((int)p1)); \
  return a; \
}

#define DEFN_SYSCALL2(fn, num, P1, P2) \
int syscall_##fn(P1 p1, P2 p2) \
{ \
  int a; \
  __asm__ volatile("int $0x7F" : "=a" (a) : "0" (num), "b" ((int)p1), "c" ((int)p2)); \
  return a; \
}

#define DEFN_SYSCALL3(fn, num, P1, P2, P3) \
int syscall_##fn(P1 p1, P2 p2, P3 p3) \
{ \
  int a; \
  __asm__ volatile("int $0x7F" : "=a" (a) : "0" (num), "b" ((int)p1), "c" ((int)p2), "d"((int)p3)); \
  return a; \
}

#define DEFN_SYSCALL4(fn, num, P1, P2, P3, P4) \
int syscall_##fn(P1 p1, P2 p2, P3 p3, P4 p4) \
{ \
  int a; \
  __asm__ volatile("int $0x7F" : "=a" (a) : "0" (num), "b" ((int)p1), "c" ((int)p2), "d" ((int)p3), "S" ((int)p4)); \
  return a; \
}

#define DEFN_SYSCALL5(fn, num) \
int syscall_##fn(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) \
{ \
  int a; \
  __asm__ volatile("int $0x7F" : "=a" (a) : "0" (num), "b" ((int)p1), "c" ((int)p2), "d" ((int)p3), "S" ((int)p4), "D" ((int)p5)); \
  return a; \
}

/******* declaration of syscalls in syscall.h **************/

DECL_SYSCALL1( puts,  uint8_t*                                         )
DECL_SYSCALL1( putch, uint8_t                                          )
DECL_SYSCALL2( settextcolor, uint8_t, uint8_t                          )
DECL_SYSCALL0( getpid                                                  )
DECL_SYSCALL1( sleepCurrentTask, uint32_t                              )
DECL_SYSCALL0( switch_context                                          )
DECL_SYSCALL1( keyboard_isKeyDown, KEY_t                               )
DECL_SYSCALL0( getkey                                                  )
DECL_SYSCALL1( video_set_mode, uint8_t                                 )
DECL_SYSCALL2( put_pixel, int16_t, int16_t                             )
DECL_SYSCALL1( file_createDirectory, char*                             )
DECL_SYSCALL2( file_create, file_t*, char*                             )
DECL_SYSCALL0( move_cursor_left                                        )
DECL_SYSCALL0( move_cursor_right                                       )
DECL_SYSCALL3( file_findByIndex, file_t*, char*, uint32_t              )
DECL_SYSCALL1( file_isDirectory, char*                                 )
DECL_SYSCALL0( desktop_enable                                          )
DECL_SYSCALL2( file_find, file_t*, char*                               )
DECL_SYSCALL4( file_readContents, file_t*, uint8_t*, uint32_t, uint32_t)
DECL_SYSCALL0( reboot                                                  )
DECL_SYSCALL3( draw_picture, uint8_t*, int16_t, int16_t                )
DECL_SYSCALL1( file_delete, char*                                      )
DECL_SYSCALL1( keyToASCII, KEY_t                                       )
DECL_SYSCALL0( timer_getMilliseconds                                   )
DECL_SYSCALL0( random                                                  )
DECL_SYSCALL1( fileManage_getBootPath, char*                           )
DECL_SYSCALL0( exitCurrentTask                                         )
DECL_SYSCALL1( file_execute, char*                                     )
DECL_SYSCALL2( pci_getDevice, uint32_t, pciDev_t*                      )

/***********************************************************/

#endif
