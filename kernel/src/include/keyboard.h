#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "stdbool.h"
#include "stdint.h"

// clang-format off
typedef enum {
    __KEY_INVALID,
    KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z, // KEY_Z is bottom left
    KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
    KEY_BCKTICK, KEY_MINUS, KEY_EQUAL, KEY_BACKSL, KEY_BACK, KEY_SPACE, KEY_TAB, KEY_CAPS, KEY_OSQBRA, KEY_CSQBRA, // KEY_OSQBRA Open square bracket
    KEY_LSHIFT, KEY_LCTRL, KEY_LGUI, KEY_LALT, KEY_RSHIFT, KEY_RCTRL, KEY_RGUI, KEY_ALTGR, KEY_MENU, KEY_ENTER, KEY_ESC,
    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
    KEY_PRINT, KEY_SCROLL, KEY_PAUSE, KEY_NUMLOCK, KEY_INS, KEY_DEL,
    KEY_HOME, KEY_PGUP, KEY_END, KEY_PGDOWN, KEY_UP, KEY_LEFT, KEY_DOWN, KEY_RIGHT,
    KEY_KPDIV, KEY_KPMULT, KEY_KPMINUS, KEY_KPPLUS, KEY_KPENTER, KEY_KPDOT,
    KEY_KP0, KEY_KP1, KEY_KP2, KEY_KP3, KEY_KP4, KEY_KP5, KEY_KP6, KEY_KP7, KEY_KP8, KEY_KP9,
    KEY_SEMICOL, KEY_APPOS, KEY_COMMA, KEY_DOT, KEY_SLASH,
    KEY_OEM_102, // German keyboard has one key more than the international one. ( <>| ) <-- Beside left shift
    __KEY_NUMBER
} KEY_t;
// clang-format on

KEY_t getkey();
char getch();
bool keyboard_isKeyDown(KEY_t key);
void keyboard_setTypematic(uint8_t rate, uint8_t delay);
char keyToASCII(KEY_t key);

#endif
