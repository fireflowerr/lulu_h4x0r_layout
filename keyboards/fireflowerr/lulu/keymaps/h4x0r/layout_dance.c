#include "layout_dance.h"
#include "log.h"
#include  "process_tap_dance.h"
#include "layer_lock.h"
#include "action_layer.h"


extern tap_dance_action_t tap_dance_actions[];

void dance_layer_each(tap_dance_state_t* state, void* user_data) {
    // if the other layer dance is active, perform a lock
    tap_dance_layer_dance_t* data = (tap_dance_layer_dance_t*)user_data;
    tap_dance_state_t* other_state = &(tap_dance_actions[data->other_idx]).state;
    uint8_t other_count = other_state->count;
    const uint8_t target_layer = state->count;
    uint8_t prev_layer = target_layer - 1;
    LOGF("Other index: %d, Other count: %d, target layer%d, prev layer %d\n", data->other_idx, other_count,
        target_layer, prev_layer);

    if (other_count > 0) {
        if (layer_state_is(other_count)) {
            layer_lock_invert(other_count);
        } else {
            tap_dance_layer_dance_t* other_data = tap_dance_actions[data->other_idx].user_data;
            // the other action would have unlocked and turned off durring iteration
            other_data->revert_on_reset = false;
        }

        reset_tap_dance(other_state);
        reset_tap_dance(state);
        return;
    }

    // if alt is pressed unlock the highest layer > 0
    if (get_mods() & MOD_MASK_ALT) {
        uint8_t highest_layer = get_highest_layer(layer_state);
        if (highest_layer) {
            LOGF("inverting layer lock: %d\n", highest_layer);
            layer_lock_invert(highest_layer);
        }
        return;
    }

    // otherwise do layer dance
    if (target_layer > MAX_LAYER_IDX) {
        LOG("max layer exceeded, resetting\n");
        reset_tap_dance(state);
        return;
    }

    if (is_layer_locked(target_layer)) {
        layer_lock_invert(target_layer);
    } else {
        layer_invert(target_layer);
    }

    if (prev_layer == 0) {
        data->revert_on_reset = true;
        return;
    }

    if (!layer_state_is(prev_layer)) { // if we get here previous layer was locked
        layer_lock_invert(prev_layer);
    } else {
        layer_invert(prev_layer);
    }
}

void dance_layer_reset(tap_dance_state_t *state, void *user_data) {
    tap_dance_layer_dance_t* data = (tap_dance_layer_dance_t*)user_data;
    if (data->revert_on_reset) {
        uint8_t target_layer = state->count;
        LOGF("reset target layer: %d\n", target_layer);
        if (target_layer > 0 && !is_layer_locked(target_layer)) {
            layer_invert(target_layer);
        }
        data->revert_on_reset = false;
    }
}
