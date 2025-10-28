#pragma once
#include "quantum.h"
#include "process_tap_dance.h"


#ifndef MAX_LAYER_IDX
    #define MAX_LAYER_IDX 5
#endif

void dance_layer_each(tap_dance_state_t* state, void* user_data);
void dance_layer_reset(tap_dance_state_t* state, void *user_data);

#define ACTION_LAYER_DANCE(other_layer_action_idx) \
    { \
        .fn = { \
            dance_layer_each, \
            NULL, \
            dance_layer_reset, \
            NULL, \
        }, \
        .user_data = (void*)(other_layer_action_idx) \
    }
