// Copyright 2024 jack (@waffle87)
// SPDX-License-Identifier: GPL-2.0-or-later
#include "oled.h"

static char img_buffer[128*32];
static layer_state_t cache = -1;


void render_layer_state(void) {
    if (layer_state == cache) {
        oled_write_raw_P(img_buffer, sizeof(img_buffer));
        return;
    }

    uint8_t column_offset = 4;
    for (uint8_t layer = 1; layer <= 5; ++layer) {
        bool filled = (layer_state & (1 << layer));

        uint8_t start_x = column_offset;
        uint8_t start_y = 0;

        for (uint8_t y = 0; y < 32; ++y) {
            for (uint8_t x = 0; x < 24; ++x) {
                uint16_t oled_pos = (start_y + y) * 128 + (start_x + x);
                uint16_t circle_pos = y * 24 + x;

                if (filled) {
                    img_buffer[oled_pos] = pgm_read_byte(&filled_circle[circle_pos]);
                } else {
                    img_buffer[oled_pos] = pgm_read_byte(&hollow_circle[circle_pos]);
                }
            }
        }

        column_offset += 25;
    }

    cache = layer_state;
    oled_write_raw_P(img_buffer, sizeof(img_buffer));
}
