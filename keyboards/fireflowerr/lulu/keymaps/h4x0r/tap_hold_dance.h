#pragma once
#include  "process_tap_dance.h"


typedef struct {
    uint16_t tap_key;
    uint16_t hold_key;
    uint16_t registered_key;
} tap_dance_tap_hold_t;

void tap_dance_hold_each(tap_dance_state_t* state, void* user_data);
void tap_dance_hold_finished(tap_dance_state_t* state, void *user_data);
void tap_dance_hold_reset(tap_dance_state_t* state, void* user_data);
void tap_dance_hold_each_release(tap_dance_state_t* state, void* user_data);

#define TAP_HOLD_ACTION(tap, hold) \
    { \
        .fn = { \
            tap_dance_hold_each, \
            tap_dance_hold_finished, \
            tap_dance_hold_reset, \
            tap_dance_hold_each_release, \
        }, \
        .user_data = (void*)&((tap_dance_tap_hold_t){ \
                .tap_key = tap, \
                .hold_key = hold, \
        }) \
    }
