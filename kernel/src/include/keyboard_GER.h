#ifndef KEYBOARD_H
#define KEYBOARD_H

#define ESC       27
#define BACKSPACE '\b'
#define TAB       '\t'
#define ENTER     '\n'
#define RETURN    '\r'
#define NEWLINE   ENTER

// Non-ASCII special scancodes // Esc in scancode is 1
#define    KESC         1
#define    KF1          0x80
#define    KF2         (KF1  + 1)
#define    KF3         (KF2  + 1)
#define    KF4         (KF3  + 1)
#define    KF5         (KF4  + 1)
#define    KF6         (KF5  + 1)
#define    KF7         (KF6  + 1)
#define    KF8         (KF7  + 1)
#define    KF9         (KF8  + 1)
#define    KF10        (KF9  + 1)
#define    KF11        (KF10 + 1)
#define    KF12        (KF11 + 1)

// Cursor Keys
#define    KINS         0x90
#define    KDEL        (KINS  + 1)
#define    KHOME       (KDEL  + 1)
#define    KEND        (KHOME + 1)
#define    KPGUP       (KEND  + 1)
#define    KPGDN       (KPGUP + 1)
#define    KLEFT       (KPGDN + 1)
#define    KUP         (KLEFT + 1)
#define    KDOWN       (KUP   + 1)
#define    KRIGHT      (KDOWN + 1)

// "Meta" keys
#define    KMETA_ALT     0x0200                                // Alt is pressed
#define    KMETA_CTRL    0x0400                                // Ctrl is pressed
#define    KMETA_SHIFT   0x0800                                // Shift is pressed
#define    KMETA_ANY    (KMETA_ALT | KMETA_CTRL | KMETA_SHIFT)
#define    KMETA_CAPS    0x1000                                // CapsLock is on
#define    KMETA_NUM     0x2000                                // NumLock is on
#define    KMETA_SCRL    0x4000                                // ScrollLock is on

// Other keys
#define    KPRNT    ( KRT    + 1 )
#define    KPAUSE   ( KPRNT  + 1 )
#define    KLWIN    ( KPAUSE + 1 )
#define    KRWIN    ( KLWIN  + 1 )
#define    KMENU    ( KRWIN  + 1 )

#define    KRLEFT_CTRL        0x1D
#define    KRRIGHT_CTRL       0x1D

#define    KRLEFT_ALT         0x38
#define    KRRIGHT_ALT        0x38

#define    KRLEFT_SHIFT       0x2A
#define    KRRIGHT_SHIFT      0x36

#define    KRCAPS_LOCK        0x3A
#define    KRSCROLL_LOCK      0x46
#define    KRNUM_LOCK         0x45
#define    KRDEL              0x53

// Keymaps: German, ascii

// Non-Shifted scan codes to ASCII:
static uint8_t asciiNonShift[] = {
0, ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 0xDF, '\'', BACKSPACE,
TAB, 'q', 'w',   'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', 0xFC, '+', ENTER, 0,
'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 0xF6, 0xE4, '^', 0, '#',
'y', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '-', 0, 0, 0, ' ', 0,
KF1, KF2, KF3, KF4, KF5, KF6, KF7, KF8, KF9, KF10, 0, 0,
KHOME, KUP, KPGUP,'-', KLEFT, '5', KRIGHT, '+', KEND, KDOWN, KPGDN, KINS, KDEL, 0, 0, '<', KF11, KF12 };

// Shifted scan codes to ASCII:
static uint8_t asciiShift[] = {
0, ESC, '!', '"', 0xA7, '$', '%', '&', '/', '(', ')', '=', '?', '`', BACKSPACE,
TAB, 'Q', 'W',   'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P', 0xDC, '*', ENTER, 0,
'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 0xD6, 0xC4, 0xB0, 0, '\'',
'Y', 'X', 'C', 'V', 'B', 'N', 'M', ';', ':', '_', 0, 0, 0, ' ', 0,
KF1,   KF2, KF3, KF4, KF5, KF6, KF7, KF8, KF9, KF10, 0, 0,
KHOME, KUP, KPGUP, '-', KLEFT, '5',   KRIGHT, '+', KEND, KDOWN, KPGDN, KINS, KDEL, 0, 0, '>', KF11, KF12 };

static uint8_t asciiAltGr[] = {
0, 0, 0, 0xB2, 0xB3, 0, 0, 0, 0x7B, 0x5B, 0x5D, 0x7D, 0x5C, 0, 0, /*   ��  {[]}\   */
0, 0x40, 0, 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0x7E, 0, 0,             /*   @ �      ~  */
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0xB5, 0, 0, 0, 0, 0, 0, 0, 0,                   /*         �     */
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7C, 0, 0 };        /*   |           */
#endif
