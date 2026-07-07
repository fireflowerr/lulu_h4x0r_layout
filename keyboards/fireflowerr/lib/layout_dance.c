#include "layout_dance.h"
#include "log.h"
#include "process_tap_dance.h"
#include "process_oneshot.h"
#include "layer_lock.h"
#include "action_layer.h"


extern tap_dance_action_t tap_dance_actions[];

void dance_layer_each(tap_dance_state_t* state, void* user_data) {
    tap_dance_layer_dance_t* data = (tap_dance_layer_dance_t*)user_data;
    tap_dance_layer_dance_t* other_data = (tap_dance_layer_dance_t*)tap_dance_actions[data->other_idx].user_data;
    tap_dance_state_t* other_state = tap_dance_get_state(data->other_idx);
    uint8_t other_count = other_state->in_use ? other_state->count : 0;
    const uint8_t target_layer = state->count;
    uint8_t prev_layer = target_layer - 1;
    LOGF("Other index: %d, Other count: %d, target layer: %d, prev layer: %d\n", data->other_idx, other_count,
        target_layer, prev_layer);

    // if alt is pressed unlock the highest layer > 0
    if (state->count == 1) {
        if (is_oneshot_layer_active()) {
            clear_oneshot_layer_state(ONESHOT_OTHER_KEY_PRESSED);
        }

        if (get_mods() & MOD_MASK_ALT) {
            uint8_t highest_layer = get_highest_layer(layer_state);
            LOGF("Attempting to invert highest layer: %d\n", highest_layer);

            if (highest_layer > 0) {
                layer_invert(highest_layer);
            }
            return;
        }
    }
    // if the other layer dance is active, toggle the layer and end both dances
    if (other_count > 0) {
        LOGF("other dance is active, toggling selected layer: %d\n", other_count);
        data->mode = LOCK;
        other_data->mode = LOCK;
        reset_tap_dance(state);
        reset_tap_dance(other_state);
        return;
    }

    // If we are past max layer, bail out
    if (target_layer > MAX_LAYER_IDX) {
        LOG("max layer exceeded, resetting\n");
        data->mode = OVERFLOW;
        reset_tap_dance(state);
        return;
    }

    // otherwise do layer dance
    if (prev_layer > 0) {
        layer_invert(prev_layer);
    }
    layer_invert(target_layer);
}

void dance_layer_finished(tap_dance_state_t *state, void *user_data) {
    if (!state->pressed) {
        return;
    }
    // getting here means we are in a long hold, act as momentary
    LOG("Long press detected, acting as momentary layer\n");
    tap_dance_layer_dance_t* data = (tap_dance_layer_dance_t*)user_data;
    data->mode = MOMENTARY;
}

void dance_layer_reset(tap_dance_state_t *state, void *user_data) {
    tap_dance_layer_dance_t* data = (tap_dance_layer_dance_t*)user_data;
    LOGF("Resetting dance. Mode was: %d\n", data->mode);
    switch (data->mode) {
        case ONESHOT:
            if (IS_LAYER_ON(state->count)) {
                set_oneshot_layer(state->count, ONESHOT_START);
                clear_oneshot_layer_state(ONESHOT_PRESSED);
            }
            break;
        case MOMENTARY:
            layer_invert(state->count);
            break;
        case OVERFLOW:
            layer_invert(state->count - 1);
            break;
        case LOCK:
        default:
            break;
    }

    data->mode = ONESHOT;
}
