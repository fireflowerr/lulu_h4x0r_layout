#pragma once
#include "quantum.h"
#include "process_tap_dance.h"


#ifndef MAX_LAYER_IDX
    #define MAX_LAYER_IDX 5
#endif

void dance_layer_each(tap_dance_state_t* state, void* user_data);
void dance_layer_finished(tap_dance_state_t* state, void* user_data);
void dance_layer_reset(tap_dance_state_t* state, void *user_data);
enum layer_dance_mode {
    ONESHOT,
    MOMENTARY,
    LOCK,
    OVERFLOW,
};

typedef struct {
    const uint8_t other_idx;
    enum layer_dance_mode mode;
} tap_dance_layer_dance_t;

#define ACTION_LAYER_DANCE(other_layer_action_idx) \
    { \
        .fn = { \
            dance_layer_each, \
            dance_layer_finished, \
            dance_layer_reset, \
            NULL, \
        }, \
        .user_data = (void*)&((tap_dance_layer_dance_t){ \
            .other_idx = other_layer_action_idx, \
            .mode = ONESHOT, \
        }) \
    }
