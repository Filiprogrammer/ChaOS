#include "list.h"
#include "os.h"

#define MOUSEDATAPORT 0x60
#define MOUSECOMMANDPORT 0x64

static uint8_t mouseid;
static uint8_t buffer[4];
static uint8_t mouse_cycle = 0;
static int16_t mouse_x = 40;
static int16_t mouse_y = 25;
static bool btn_prim = false;
static bool btn_sec = false;
static bool btn_mid = false;
static bool mouse_enabled = false;
static int16_t mouse_max_x = 79;
static int16_t mouse_max_y = 49;
static uint8_t mouse_slowness = 10;

static listHead_t* mouse_move_listeners;
static listHead_t* mouse_click_listeners;

static bool mouse_handler_running = false;

void mouse_setup(int16_t max_x, int16_t max_y, uint8_t slowness) {
    mouse_x = max_x / 2;
    mouse_y = max_y / 2;
    mouse_max_x = max_x;
    mouse_max_y = max_y;
    mouse_slowness = slowness;
}

void mouse_set_bounds(int16_t max_x, int16_t max_y) {
    mouse_max_x = max_x;
    mouse_max_y = max_y;
}

void mouse_switch(bool enable) {
    mouse_enabled = enable;
    btn_prim = 0;
    btn_sec = 0;
    btn_mid = 0;
}

void mouse_add_move_listener(void (*move_event)(int16_t, int16_t)) {
    if (!mouse_move_listeners) mouse_move_listeners = list_create();
    list_append(mouse_move_listeners, move_event);
}

void mouse_add_click_listener(void (*click_event)(uint8_t, bool)) {
    if (!mouse_click_listeners) mouse_click_listeners = list_create();
    list_append(mouse_click_listeners, click_event);
}

void mouse_remove_move_listener(void (*move_event)(int16_t, int16_t)) {
    if (mouse_move_listeners) {
        list_delete(mouse_move_listeners, move_event);
    }
}

void mouse_remove_click_listener(void (*click_event)(uint8_t, bool)) {
    if (mouse_click_listeners) {
        list_delete(mouse_click_listeners, click_event);
    }
}

static inline void mouse_move_event(int16_t x, int16_t y) {
    if (mouse_move_listeners) {
        size_t size = list_getSize(mouse_move_listeners);
        for (size_t i = 1; i <= size; ++i) {
            void* (*move_event)(int16_t, int16_t) = list_getElement(mouse_move_listeners, i);
            if (move_event) move_event(x, y);
        }
    }
}

static inline void mouse_click_event(uint8_t btn, bool pressed) {
    if (mouse_click_listeners) {
        size_t size = list_getSize(mouse_click_listeners);
        for (size_t i = 1; i <= size; ++i) {
            void* (*click_event)(uint8_t, bool) = list_getElement(mouse_click_listeners, i);
            if (click_event) click_event(btn, pressed);
        }
    }
}

void mouse_handler(registers_t* r) {
    if (mouse_handler_running) return;
    mouse_handler_running = true;

    uint8_t _status = inportb(MOUSECOMMANDPORT);
    if (!(_status & 0x20)) {
        mouse_handler_running = false;
        return;
    }

    switch (mouse_cycle) {
        case 0:
            buffer[0] = inportb(MOUSEDATAPORT);
            if (buffer[0] & 0x08) {  //if this is really the first byte
                if (btn_prim != (buffer[0] & 0x01)) {
                    btn_prim = buffer[0] & 0x01;
                    if (mouse_enabled) mouse_click_event(0, btn_prim);
                }
                if (btn_sec != ((buffer[0] & 0x02) >> 1)) {
                    btn_sec = (buffer[0] & 0x02) >> 1;
                    if (mouse_enabled) mouse_click_event(1, btn_sec);
                }
                if (btn_mid != ((buffer[0] & 0x04) >> 2)) {
                    btn_mid = (buffer[0] & 0x04) >> 2;
                    if (mouse_enabled) mouse_click_event(2, btn_mid);
                }
                ++mouse_cycle;
            } else {
                //Mouse sent unknown package
                mouse_handler_running = false;
                return;
            }
            break;
        case 1:
            buffer[1] = inportb(MOUSEDATAPORT);  //X Movement
            ++mouse_cycle;
            break;
        case 2:
            buffer[2] = inportb(MOUSEDATAPORT);  //Y Movement
            if (mouseid == 0) {
                if (buffer[0] & 0x10) {  //if x sign bit is 1
                    mouse_x -= (buffer[1] ^ 0xFF) + 1;
                } else {
                    mouse_x += buffer[1];
                }
                if (buffer[0] & 0x20) {  //if y sign bit is 1
                    mouse_y += (buffer[2] ^ 0xFF) + 1;
                } else {
                    mouse_y -= buffer[2];
                }
                if (mouse_x > (mouse_max_x * mouse_slowness)) mouse_x = (mouse_max_x * mouse_slowness);
                if (mouse_x < 0) mouse_x = 0;
                if (mouse_y > (mouse_max_y * mouse_slowness)) mouse_y = (mouse_max_y * mouse_slowness);
                if (mouse_y < 0) mouse_y = 0;
                if (mouse_enabled) mouse_move_event(mouse_x / mouse_slowness, mouse_y / mouse_slowness);  // Call mouse move event
                mouse_cycle = 0;
            } else {
                ++mouse_cycle;
            }
            break;
        case 3:
            break;
    }
    mouse_handler_running = false;
}

void mouse_install() {
    //Installs 'mouse_handler' to IRQ12
    irq_install_handler(12, mouse_handler);
    outportb(MOUSECOMMANDPORT, 0xA8);

    uint8_t _status;

    //Enable the interrupts
    outportb(MOUSECOMMANDPORT, 0x20);
    _status = (inportb(MOUSEDATAPORT) | 2);
    outportb(MOUSECOMMANDPORT, 0x60);
    outportb(MOUSEDATAPORT, _status);

    outportb(MOUSECOMMANDPORT, 0xD4);

    //Enable the mouse
    outportb(MOUSEDATAPORT, 0xF4);
    inportb(MOUSEDATAPORT);

    //TODO: Check for mouseid
    //0 : No scrollwheel
    //1 : Scrollwheel
    //2 : 5 Mouse buttons and scrollwheel
    mouseid = 0;
}
