#include "os.h"
#include "keyboard.h"

static const KEY_t scancodeToKey_default[] = {
//  0           1           2           3           4           5            6             7
//---------------------------------------------------------------------------------------------------------
    0,          KEY_ESC,    KEY_1,      KEY_2,      KEY_3,      KEY_4,       KEY_5,        KEY_6,      // 0
    KEY_7,      KEY_8,      KEY_9,      KEY_0,      KEY_MINUS,  KEY_EQUAL,   KEY_BACK,     KEY_TAB,
    KEY_Q,      KEY_W,      KEY_E,      KEY_R,      KEY_T,      KEY_Y,       KEY_U,        KEY_I,      // 1
    KEY_O,      KEY_P,      KEY_OSQBRA, KEY_CSQBRA, KEY_ENTER,  KEY_LCTRL,   KEY_A,        KEY_S,
    KEY_D,      KEY_F,      KEY_G,      KEY_H,      KEY_J,      KEY_K,       KEY_L,        KEY_SEMICOL,// 2
    KEY_APPOS,  KEY_BCKTICK,KEY_LSHIFT, KEY_BACKSL, KEY_Z,      KEY_X,       KEY_C,        KEY_V,
    KEY_B,      KEY_N,      KEY_M,      KEY_COMMA,  KEY_DOT,    KEY_SLASH,   KEY_RSHIFT,   KEY_KPMULT, // 3
    KEY_LALT,   KEY_SPACE,  KEY_CAPS,   KEY_F1,     KEY_F2,     KEY_F3,      KEY_F4,       KEY_F5,
    KEY_F6,     KEY_F7,     KEY_F8,     KEY_F9,     KEY_F10,    KEY_NUMLOCK, KEY_SCROLL,   KEY_KP7,    // 4
    KEY_KP8,    KEY_KP9,    KEY_KPMINUS,KEY_KP4,    KEY_KP5,    KEY_KP6,     KEY_KPPLUS,   KEY_KP1,
    KEY_KP2,    KEY_KP3,    KEY_KP0,    KEY_KPDOT,  0,          0,           KEY_OEM_102,  KEY_F11,    // 5
    KEY_F12,    0,          0,          0,          0,          0,           0,            0,
    0,          0,          0,          0,          0,          0,           0,            0,          // 6
    0,          0,          0,          0,          0,          0,           0,            0,
    0,          0,          0,          0,          0,          0,           0,            0,          // 7
    0,          0,          0,          0,          0,          0,           0,            0
};

static const KEY_t scancodeToKey_E0[] = {
//  0           1           2           3           4           5            6             7
//---------------------------------------------------------------------------------------------------------
    0,          0,          0,          0,          0,          0,           0,            0,          // 0
    0,          0,          0,          0,          0,          0,           0,            0,
    0,          0,          0,          0,          0,          0,           0,            0,          // 1
    0,          0,          0,          0,          KEY_KPENTER,KEY_RCTRL,   0,            0,
    0,          0,          0,          0,          0,          0,           0,            0,          // 2
    0,          0,          0,          0,          0,          0,           0,            0,
    0,          0,          0,          0,          0,          KEY_KPDIV,   0,            KEY_PRINT,  // 3
    KEY_ALTGR,  0,          0,          0,          0,          0,           0,            0,
    0,          0,          0,          0,          0,          0,           0,            KEY_HOME,   // 4
    KEY_UP,     KEY_PGUP,   0,          KEY_LEFT,   0,          KEY_RIGHT,   0,            KEY_END,
    KEY_DOWN,   KEY_PGDOWN, KEY_INS,    KEY_DEL,    0,          0,           0,            0,          // 5
    0,          0,          0,          KEY_LGUI,   KEY_RGUI,   0,           0,            0,
    0,          0,          0,          0,          0,          0,           0,            0,          // 6
    0,          0,          0,          0,          0,          0,           0,            0,
    0,          0,          0,          0,          0,          0,           0,            0,          // 7
    0,          0,          0,          0,          0,          0,           0,            0
};

static const char keyToASCII_default[__KEY_NUMBER] = {
      0, // Invalid key
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
    'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
    'u', 'v', 'w', 'x', 'z', 'y', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '^',0xDF,0x60, '#',
   '\b', ' ','\t',   0,0xFC, '+',   0,   0,   0,   0,
      0,   0,   0,   0,   0,'\n',   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0, '/', '*', '-', '+','\n', '.', '0',
    '1', '2', '3', '4', '5', '6', '7', '8', '9',0xF6,
   0xE4, ',', '.', '-', '<'
};

static const char keyToASCII_shift[__KEY_NUMBER] = {
      0, // Invalid key
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Z', 'Y', '=', '!', '"',0xA7,
    '$', '%', '&', '/', '(', ')',0xB0, '?', '`','\'',
      0,   0,   0,   0,0xDC, '*',   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,0xD6,
   0xC4, ';', ':', '_', '>'
};

static const char keyToASCII_altGr[__KEY_NUMBER] = {
      0, // Invalid key
      0,   0,   0,   0,0x80,   0,   0,   0,   0,   0,
      0,   0,0xB5,   0,   0,   0, '@',   0,   0,   0,
      0,   0,   0,   0,   0,   0, '}',   0,0xB2,0xB3,
      0,   0,   0, '{', '[', ']',   0,'\\',   0,   0,
      0,   0,   0,   0,   0, '~',   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0, '|'
};

static const char keyToASCII_shiftAltGr[__KEY_NUMBER] = {0};

static bool pressedKeys[__KEY_NUMBER] = {false};
static uint8_t led = 0;

#define KQSIZE    20 // size of key queue

// Key Queue
KEY_t  KEYQUEUE[KQSIZE];     // circular queue buffer
KEY_t* pHeadKQ;              // pointer to the head of valid data
KEY_t* pTailKQ;              // pointer to the tail of valid data
uint32_t KQ_count_read;      // number of data read from queue buffer
uint32_t KQ_count_write;     // number of data put into queue buffer

static uint8_t getScancode() {
    uint8_t scancode = 0;

    if( inportb(0x64)&1 )
        scancode = inportb(0x60);   // 0x60: get scan code from the keyboard

    // ACK: toggle bit 7 at port 0x61
    uint8_t port_value = inportb(0x61);
    outportb(0x61, port_value |  0x80); // 0->1
    outportb(0x61, port_value &~ 0x80); // 1->0

    return scancode;
}

static KEY_t scancodeToKey(uint8_t scancode, bool* pressed) {
    static uint8_t byteCounter = 0;

    *pressed = !(scancode & 0x80);

    if(scancode == 0xE0) {
        byteCounter = 0xFF;
    }
    else if(scancode == 0xE1) {
        byteCounter = 1;
    }
    else {
        if(byteCounter != 0 && byteCounter <= 3) { // E1
            ++byteCounter;
            if(byteCounter == 3){
                return KEY_PAUSE;
            }
        } else if(byteCounter == 0xFF) { // E0
            byteCounter = 0;
            return scancodeToKey_E0[scancode & 0x7F];
        } else { // Default
            return scancodeToKey_default[scancode & 0x7F];
        }
    }
    return __KEY_INVALID;
}

char keyToASCII(KEY_t key) {
    bool altGr = pressedKeys[KEY_ALTGR];
    bool shift = pressedKeys[KEY_LSHIFT] || pressedKeys[KEY_RSHIFT];
    if(led & 0x04) // is CapsLock on?
        shift = !shift;

    if(altGr){
        if(shift) {
            return keyToASCII_shiftAltGr[key];
        } else {
            return keyToASCII_altGr[key];
        }
    } else if(shift) {
        return keyToASCII_shift[key];
    }
    return keyToASCII_default[key];
}

static void keyboard_updateLED() {
    outportb(0x60, 0xED);
    inportb(0x60);
    outportb(0x60, led);
    inportb(0x60);
}

void keyboard_handler(registers_t* r) {
    uint8_t scancode = getScancode();
    bool pressed = false;
    KEY_t key = scancodeToKey(scancode, &pressed);
    if(key == __KEY_INVALID) return;
    pressedKeys[key] = pressed;
    if(pressed) {
        if(key == KEY_CAPS) {
            led ^= 0x04;
            keyboard_updateLED();
        }
        *(pTailKQ) = key;
        ++(KQ_count_write);

        if(pTailKQ > KEYQUEUE){
           --pTailKQ;
        }
        if(pTailKQ == KEYQUEUE){
           pTailKQ = (KEYQUEUE)+KQSIZE-1;
        }
    }
}

// Read from key queue and return the key
KEY_t getkey() {
    if(KQ_count_write > KQ_count_read) {
        KEY_t key = *(pHeadKQ);
        ++(KQ_count_read);

        if(pHeadKQ > KEYQUEUE) {
            --pHeadKQ;
        }
        if(pHeadKQ == KEYQUEUE) {
            pHeadKQ = (KEYQUEUE)+KQSIZE-1;
        }
        return key;
    }
    return 0;
}

// Read from key queue and convert that to ASCII and return the ASCII char
char getch() {
    return keyToASCII(getkey());
}

bool keyboard_isKeyDown(KEY_t key) {
    return pressedKeys[key];
}

/**
 * @brief Set the repeat rate for holding down a key.
 * 
 * @param rate Repeat rate (00000b = 30 Hz, ..., 11111b = 2 Hz)
 * @param delay Delay before keys repeat (00b = 250 ms, 01b = 500 ms, 10b = 750 ms, 11b = 1000 ms)
 */
void keyboard_setTypematic(uint8_t rate, uint8_t delay) {
    outportb(0x60, 0xF3);
    inportb(0x60);
    uint8_t tmp = (rate & 0x1F) | ((delay & 0x03) << 5);
    outportb(0x60, tmp);
    inportb(0x60);
}

void keyboard_install() {
    /* Installs 'keyboard_handler' to IRQ1 */
    irq_install_handler(1, keyboard_handler);
    while( inportb(0x64)&1 )
        inportb(0x60);

    memset(KEYQUEUE, 0, KQSIZE);
    pHeadKQ = KEYQUEUE;          // pointer to the head of valid data
    pTailKQ = KEYQUEUE;          // pointer to the tail of valid data
    KQ_count_read  = 0;          // number of data read from queue buffer
    KQ_count_write = 0;          // number of data put into queue buffer
}
