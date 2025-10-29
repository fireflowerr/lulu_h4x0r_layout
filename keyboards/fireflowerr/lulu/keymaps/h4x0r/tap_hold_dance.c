#include "quantum.h"
#include "tap_hold_dance.h"
#include  "process_tap_dance.h"


void tap_dance_hold_each(tap_dance_state_t* state, void* user_data) {
    if (state->count == 1) {
        return;
    }

    tap_dance_tap_hold_t* tap_hold = (tap_dance_tap_hold_t*)user_data;
    register_code(tap_hold->registered_key);
}

void tap_dance_hold_finished(tap_dance_state_t *state, void *user_data) {
    tap_dance_tap_hold_t* tap_hold = (tap_dance_tap_hold_t*)user_data;
    if (state->pressed && state->count == 1) {
        register_code16(tap_hold->hold_key);
        tap_hold->registered_key = tap_hold->hold_key;
    }
}

void tap_dance_hold_reset(tap_dance_state_t* state, void* user_data) {
    tap_dance_tap_hold_t* tap_hold = (tap_dance_tap_hold_t*)user_data;
    if (tap_hold->registered_key) {
        unregister_code16(tap_hold->registered_key);
        tap_hold->registered_key = 0;
    }
}

void tap_dance_hold_each_release(tap_dance_state_t* state, void* user_data) {
    tap_dance_tap_hold_t* tap_hold = (tap_dance_tap_hold_t*)user_data;
    if (!tap_hold->registered_key) {
        tap_hold->registered_key = tap_hold->tap_key;
        tap_code16(tap_hold->registered_key);
        return;
    }
    unregister_code16(tap_hold->registered_key);
}
