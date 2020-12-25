#include "userlib.h"

float box_x1[] = {0.25, -5, -7};
float box_x2[] = {10, 0, 3};
float box_y1[] = {0.25, 5, -9};
float box_y2[] = {10, 10, -7};
uint8_t box_color[] = {0x03, 0xE0, 0x1C};
#define BOX_NUMBER 3
#define GAME_WIDTH 320
#define GAME_HEIGHT 200
#define MAX_RENDER_DIST 12
#define FOV 80
#define BG_COLOR 0
#define FPS 15

float camX = -2;
float camY = -2;
float dir = 0;

uint8_t screenBuffer[4 + GAME_WIDTH * GAME_HEIGHT];

void step() {
    bool keyPressed = false;
    while (!keyPressed) {
        if (isKeyDown(KEY_A)) {
            camX -= cos(dir);
            camY -= sin(dir);
            keyPressed = true;
        }
        if (isKeyDown(KEY_D)) {
            camX += cos(dir);
            camY += sin(dir);
            keyPressed = true;
        }
        if (isKeyDown(KEY_W)) {
            camX += sin(-dir);
            camY += cos(dir);
            keyPressed = true;
        }
        if (isKeyDown(KEY_S)) {
            camX -= sin(-dir);
            camY -= cos(dir);
            keyPressed = true;
        }
        if (isKeyDown(KEY_LEFT)) {
            dir += 5;
            keyPressed = true;
        }
        if (isKeyDown(KEY_RIGHT)) {
            dir -= 5;
            keyPressed = true;
        }
        if (isKeyDown(KEY_ESC)) {
            video_set_mode(0);
            exit();
        }
    }
}

bool box_isCollidingRot(uint8_t boxId, float xoffset, float yoffset, float zoffset, float x, float y, float z, float rot_z) {
    float sin_rot_z = sin(rot_z);
    float cos_rot_z = cos(rot_z);

    // FINAL RXYZ
    float ix = cos_rot_z;
    float iy = -sin_rot_z;

    float jx = sin_rot_z;
    float jy = cos_rot_z;

    // DETERMINANTS
    float _D = ix * jy - jx * iy;
    float _DX = x * jy - jx * y;
    float _DY = ix * y - x * iy;

    // OUTPUT
    float ax = _DX / _D;
    float ay = _DY / _D;

    if ((xoffset + ax) >= box_x1[boxId] && (xoffset + ax) <= box_x2[boxId] && (yoffset + ay) >= box_y1[boxId] && (yoffset + ay) <= box_y2[boxId]) {
        return true;
    }
    return false;
}

inline void plot_pixel(int16_t x, int16_t y, uint8_t color) {
    screenBuffer[4 + y * GAME_WIDTH + x] = color;
}

uint8_t darken_color(uint8_t color, float dark) {
    if (dark <= 1.0) return color;
    uint8_t r = ((color & 0xE0) >> 5);
    uint8_t g = ((color & 0x1C) >> 2);
    uint8_t b = (color & 0x03);
    r /= dark;
    g /= dark;
    b /= dark;
    return (r << 5) | (g << 2) | b;
}

void draw() {
    int16_t y = 0;
    for (int16_t x = -GAME_WIDTH / 2; x < GAME_WIDTH / 2; ++x) {
        float len = sqrt(power(x, 2) + power(FOV, 2) + power(y, 2));
        float v0x = x / len;
        float v0y = FOV / len;
        float v0z = -y / len;

        for (float i = 0; i < MAX_RENDER_DIST; i += 0.25) {
            float checkx = v0x * i;
            float checky = v0y * i;
            float checkz = v0z * i;

            for (uint8_t j = 0; j < BOX_NUMBER; ++j) {
                if (box_isCollidingRot(j, camX, camY, 0, checkx, checky, checkz, dir)) {
                    int16_t k = 0;
                    if (i) {
                        float height = MIN((len / i) * 2.0, GAME_HEIGHT / 2);
                        uint8_t color = darken_color(box_color[j], i / (MAX_RENDER_DIST / 2));
                        for (; k < height; ++k) {
                            plot_pixel(GAME_WIDTH / 2 + x, GAME_HEIGHT / 2 - k - 1, color);
                            plot_pixel(GAME_WIDTH / 2 + x, GAME_HEIGHT / 2 + k, color);
                        }
                    } else {
                        for (; k < GAME_HEIGHT; ++k) {
                            screenBuffer[4 + k * GAME_WIDTH + (GAME_WIDTH / 2 + x)] = box_color[j];
                        }
                    }
                    for (; k < GAME_HEIGHT / 2; ++k) {
                        plot_pixel(GAME_WIDTH / 2 + x, GAME_HEIGHT / 2 - k - 1, BG_COLOR);
                        plot_pixel(GAME_WIDTH / 2 + x, GAME_HEIGHT / 2 + k, BG_COLOR);
                    }
                    goto rayLoop;
                }
            }
        }
        for (int16_t k = 0; k < GAME_HEIGHT; ++k) {
            screenBuffer[4 + k * GAME_WIDTH + (GAME_WIDTH / 2 + x)] = BG_COLOR;
        }
    rayLoop:;
    }
}

int main() {
    video_set_mode(2);

    screenBuffer[0] = (uint8_t)(GAME_WIDTH >> 8);
    screenBuffer[1] = (uint8_t)(GAME_WIDTH);
    screenBuffer[2] = (uint8_t)(GAME_HEIGHT >> 8);
    screenBuffer[3] = (uint8_t)(GAME_HEIGHT);
    draw();
    draw_picture(screenBuffer, 0, 0);

    for (;;) {
        int32_t sleepUntil = ((int32_t)getMilliseconds()) + 1000 / FPS;
        step();
        draw();
        draw_picture(screenBuffer, 0, 0);

        int32_t time_sleep = sleepUntil - ((int32_t)getMilliseconds());
        if (time_sleep > 0) {
            sleepMilliSeconds(time_sleep);
        }
    }

    return 0;
}
