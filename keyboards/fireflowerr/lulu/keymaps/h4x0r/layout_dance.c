#include "layout_dance.h"
#include  "process_tap_dance.h"
#include "layer_lock.h"
#include "action_layer.h"


extern tap_dance_action_t tap_dance_actions[];
extern const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS];

void dance_layer_each(tap_dance_state_t* state, void* user_data) {
    // if the other layer dance is active, perform a lock
    if (user_data != NULL) {
       uint8_t other_action_idx = *((uint8_t*)user_data);
       tap_dance_state_t* other_state = &(tap_dance_actions[other_action_idx]).state;
       uint8_t other_count = other_state->count;
       if (other_count > 0) {
           layer_lock_invert(other_count);
           return;
       }
    }

    // if alt is pressed unlock the highest layer > 0
    if (get_mods() & MOD_MASK_ALT) {
        uint8_t highest_layer = get_highest_layer(layer_state);
        if (highest_layer) {
            layer_lock_invert(highest_layer);
        }
    }

    // otherwise do layer dance
    const uint8_t target_layer = state->count;
    if (target_layer > 0) {
        if (target_layer > MAX_LAYER_IDX) {
            reset_tap_dance(state);
        } else {
            layer_invert(target_layer);
        }

        uint8_t prev_layer = target_layer - 1;
        if (!layer_state_is(prev_layer)) {
            layer_invert(prev_layer);
        } else if (prev_layer > 0 && !is_layer_locked(prev_layer)) {
            layer_invert(prev_layer);
        }
    }
}

void dance_layer_reset(tap_dance_state_t *state, void *user_data) {
    uint8_t target_layer = state->count;
    if (target_layer > 0 && !is_layer_locked(target_layer)) {
        layer_invert(target_layer);
    }
}
