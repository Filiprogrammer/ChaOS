#include "pc_speaker.h"

#include "task.h"

/**
 * @brief Play sound using built in speaker.
 * 
 * @param frequency The frequency of the sound in Hz
 */
void play_sound(uint32_t frequency) {
    outportb(0x43, 0xB6);
    frequency = 1193180 / frequency;
    outportb(0x42, frequency);
    outportb(0x42, frequency >> 8);
    uint8_t temp = inportb(0x61);
    if (temp != (temp | 3)) {
        outportb(0x61, temp | 3);
    }
}

/**
 * @brief Make the speaker shutup.
 * 
 */
void nosound() {
    outportb(0x61, inportb(0x61) & 0xFC);
}

void beep() {
    play_sound(100);
    sleepMilliSeconds(100);
    nosound();
}
