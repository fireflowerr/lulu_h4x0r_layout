#include QMK_KEYBOARD_H
#include "quantum.h"
#include "process_tap_dance.h"
#include "layer_lock.h"
#include "process_key_override.h"
#include "keycodes.h"
#include "action.h"
#include "log.h"

enum layers {
    _QWERTY,
    _FUNCTION,
    _MEDIA,
    _NUMBER,
    _GAMING,
    _LAST_LAYER = _GAMING,
};

enum td_actions {
    TD_LAYER,
    TD_RLAYER,
    TD_ESC_RCTL,
};

enum custom_keycodes {
    CKC_LAYER_LOCK = SAFE_RANGE,
};

typedef struct {
    uint16_t tap_key;
    uint16_t hold_key;
    uint16_t registered_key;
} tap_dance_tap_hold_t;

typedef struct {
    bool hand_swap;
    bool pinned;
} tap_dance_layer_dance_t;

const key_override_t alt_rshift_capslock_override = ko_make_basic(MOD_MASK_ALT, KC_RIGHT_SHIFT, KC_CAPS_LOCK);

const key_override_t* key_overrides[] = {
    &alt_rshift_capslock_override,
};

static uint8_t dance_layer = 0;
static bool oneshot_hand_swap = false;

/**
 * While layer dance active:
 *  Toggle lock on the dance layer
 * Otherwise:
 *  Toggle lock on highest active layer
 */
void ckc_layer_lock(void) {
    LOG("ckc_layer_lock\n");
    if (dance_layer) {
        LOGF("Locking layer: %d\n", dance_layer);
        layer_lock_invert(dance_layer);
    } else {
        uint8_t highest_layer = get_highest_layer(layer_state);
        if (highest_layer) {
            LOGF("Locking layer: %d\n", highest_layer);
            layer_lock_invert(highest_layer);
        }
    }
}

void dance_layer_each(tap_dance_state_t *state, void *user_data) {
    uint8_t target_layer = state->count;
    // will decide in release or finish whether this is a oneshot or momentory lock
    if (target_layer == 1 && state->weak_mods & MOD_MASK_ALT) {
        LOG("staring hand swap tap dance\n");
        tap_dance_layer_dance_t* data = (tap_dance_layer_dance_t*)user_data;
        data->hand_swap = true;
        return;
    }

    // double tap locks in hand swap
    tap_dance_layer_dance_t* data = (tap_dance_layer_dance_t*)user_data;
    if (data->hand_swap) {
        LOG("pinning hand swap\n");
        oneshot_hand_swap = false;
        data->pinned = true;

        reset_tap_dance(state);
        return;
    }

    // otherwise do layer dance
    if (target_layer > 0) {
        if (target_layer > _LAST_LAYER) {
            reset_tap_dance(state);
        } else {
            layer_on(target_layer);
            dance_layer = target_layer;
        }

        uint8_t prev_layer = target_layer - 1;
        if (prev_layer > 0 && !is_layer_locked(prev_layer)) {
            layer_off(prev_layer);
        }
        return;
    }
    LOG("staring layer dance\n");
}


void dance_layer_finished(tap_dance_state_t *state, void *user_data) {
    // if this is a long hold, act as MO hand swap
    if (state->count == 1 && state->pressed && ((tap_dance_layer_dance_t*)user_data)->hand_swap) {
        LOG("activing momentary hand swap\n");
        swap_hands_toggle();
    }
}

void dance_layer_release(tap_dance_state_t *state, void *user_data) {
    // if it is a tap, act as OS hand swap
    if (state->count == 1 && !state->finished && ((tap_dance_layer_dance_t*)user_data)->hand_swap) {
        LOG("activating one shot hand swap\n");
        oneshot_hand_swap = true;
        swap_hands_toggle();
    }
}

void dance_layer_reset(tap_dance_state_t *state, void *user_data) {
    tap_dance_layer_dance_t* data = (tap_dance_layer_dance_t*)user_data;
    // clean hand swap related state
    if (data->hand_swap) {
        if (!oneshot_hand_swap && !data->pinned) {
            LOG("deactivating hand swap\n");
            swap_hands_toggle();
        }
        data->pinned = false;
        data->hand_swap = false;
    }

    // clean up layer dance state and deactivate if not locked
    uint8_t target_layer = state->count;
    if (target_layer > 0 && !is_layer_locked(target_layer)) {
        layer_off(target_layer);
    }

    dance_layer = 0;
}


void dance_rlayer_each(tap_dance_state_t *state, void *user_data) {
    if (state->weak_mods & MOD_MASK_SHIFT) {
        register_code16(S(KC_QUOTE));
    } else {
        register_code16(KC_QUOTE);
    }
}

void dance_rlayer_each_release(tap_dance_state_t *state, void *user_data) {
    uint8_t highest_layer = get_highest_layer(layer_state);
    if (highest_layer) {
        return;
    }

    if (state->weak_mods & MOD_MASK_SHIFT) {
        unregister_code16(S(KC_QUOTE));
    } else {
        unregister_code16(KC_QUOTE);
    }
}

void tap_dance_tap_hold_each(tap_dance_state_t *state, void *user_data) {
    if (state->count == 1) {
        return;
    }

    tap_dance_tap_hold_t* tap_hold = (tap_dance_tap_hold_t*)user_data;
    register_code16(tap_hold->registered_key);
}

void tap_dance_hold_finished(tap_dance_state_t *state, void *user_data) {
    tap_dance_tap_hold_t* tap_hold = (tap_dance_tap_hold_t*)user_data;
    if (state->pressed && state->count == 1) {
            register_code16(tap_hold->hold_key);
            tap_hold->registered_key = tap_hold->hold_key;
    }
}

void tap_dance_hold_release(tap_dance_state_t* state, void* user_data) {
    tap_dance_tap_hold_t* tap_hold = (tap_dance_tap_hold_t*)user_data;
    if (!tap_hold->registered_key) {
        tap_hold->registered_key = tap_hold->tap_key;
        tap_code16(tap_hold->registered_key);
    }
}

void tap_dance_hold_reset(tap_dance_state_t *state, void *user_data) {
    tap_dance_tap_hold_t* tap_hold = (tap_dance_tap_hold_t*)user_data;
    if (tap_hold->registered_key) {
        unregister_code16(tap_hold->registered_key);
        tap_hold->registered_key = 0;
    }
}

tap_dance_action_t tap_dance_actions[] = {
    /*
     * - ctrl:
     *  act as CKC_LAYER_LOCK
     * - alt:
     *   - tap:
     *      act as SH_OS
     *   - double tap:
     *      act as SH_TOGG
     *   - hold:
     *      act as SH_MON
     * - otherwise:
     *   - tap:
     *      cyle through layers
     *   - tap + hold
     *      act as MO(layer) for currently selected layer
     */
    [TD_LAYER] = {
        .fn = {
            dance_layer_each,
            dance_layer_finished,
            dance_layer_reset,
            dance_layer_release,
        },
        .user_data = (void*)&((tap_dance_layer_dance_t){
            false,
            false,
        }),
    },
    /*
     * - ctrl
     *   acts as TD_LAYER sans ctrl processing
     * - otherwise
     *   - layer 0
     *      acts as KC_QUOT
     *   - otherwise
     *      acts as CKC_LAYER_LOCK
     */
    [TD_RLAYER] = ACTION_TAP_DANCE_FN_ADVANCED_WITH_RELEASE(dance_rlayer_each, NULL, NULL, dance_rlayer_each_release),
    /*
     * - tap
     *   acts as KC_ESC
     * - hold
     *   acts as KC_RCTRL
     */
    [TD_ESC_RCTL] = {
        .fn = {
            tap_dance_tap_hold_each,
            tap_dance_hold_finished,
            tap_dance_hold_reset,
            tap_dance_hold_release,
        },
        .user_data = (void*)&((tap_dance_tap_hold_t){
            .tap_key = KC_ESC,
            .hold_key = KC_RIGHT_CTRL,
        }),
    },
};

/**
 * Provides layer lock override when ctrl pressed.
 */
bool process_layer(keyrecord_t *record) {
    uint8_t mods = get_mods();
    if (!record->event.pressed || !(mods & MOD_MASK_CTRL)) {
        return true;
    }

    uint8_t highest_layer = get_highest_layer(layer_state);
    if (highest_layer) {
        ckc_layer_lock();

        if (!dance_layer) {
            reset_tap_dance(&tap_dance_actions[TD_LAYER].state);
        }
    }
    return false;
}

/**
 * Provides layer lock override when ctrl is not pressed.
 */
bool process_r_layer(keyrecord_t *record) {
    if (!record->event.pressed || get_mods() & MOD_MASK_CTRL) {
        return true;
    }

    uint8_t highest_layer = get_highest_layer(layer_state);
    if (highest_layer) {
        ckc_layer_lock();

        if (!dance_layer) {
            reset_tap_dance(&tap_dance_actions[TD_RLAYER].state);
        }
        return false;
    }
    return true;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    LOGF("keycode: %d\n", keycode);

    // handle oneshot hand swap
    if (record->event.pressed && oneshot_hand_swap && keycode != TD(TD_LAYER)) {
        LOG("processing one shot hand swap\n");
        oneshot_hand_swap = false;
        tap_dance_action_t* action = &tap_dance_actions[TD_LAYER];
        if (!action->state.finished) {
            LOG("hand swap tap dance active, resetting...\n");
            reset_tap_dance(&tap_dance_actions[TD_LAYER].state);
        }
        LOG("tap dance not active, toggling swap\n");
        swap_hands_toggle();
        return true;
    }

    // advanced overrides
    switch(keycode) {
        // cant do two tapdances at once so our layer lock cant itself be a tapdance
        case TD(TD_LAYER):
            return process_layer(record);
        case TD(TD_RLAYER):
            return process_r_layer(record);

        // handle custom keycodes
        case CKC_LAYER_LOCK:
            ckc_layer_lock();
            return false;
    }

    return true;
}

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

/* QWERTY (1)
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * |  ~   |   1  |   2  |   3  |   4  |   5  |                    |   6  |   7  |   8  |   9  |   0  |  -   |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | Tab  |   Q  |   W  |   E  |   R  |   T  |                    |   Y  |   U  |   I  |   O  |   P  |  =   |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | LAYER|   A  |   S  |   D  |   F  |   G  |-------.    ,-------|   H  |   J  |   K  |   L  |   ;  |  '   |
 * |------+------+------+------+------+------|   [   |    |    ]  |------+------+------+------+------+------|
 * |LShift|   Z  |   X  |   C  |   V  |   B  |-------|    |-------|   N  |   M  |   ,  |   .  |   /  |RShift|
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *                   | LAlt | LGUI | LCtl | /Space  /       \Enter \  | ESC  |BackSP|  \   |
 *                   |      |      |      |/       /         \      \ | RCtl |      |      |
 *                   `----------------------------'           '------''--------------------'
 */
[_QWERTY] = LAYOUT(
  KC_GRAVE,     KC_1,   KC_2,    KC_3,    KC_4,    KC_5,                             KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINUS,
  KC_TAB,       KC_Q,   KC_W,    KC_E,    KC_R,    KC_T,                             KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_EQUAL,
  TD(TD_LAYER), KC_A,   KC_S,    KC_D,    KC_F,    KC_G,                             KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, TD(TD_RLAYER),
  KC_LSFT,      KC_Z,   KC_X,    KC_C,    KC_V,    KC_B, KC_LBRC, KC_RBRC,   KC_N,    KC_M,    KC_COMM, KC_DOT,      KC_SLSH,     KC_RSFT,
                                         KC_LALT, KC_LGUI, KC_LCTL,  KC_SPC,  KC_ENT,   TD(TD_ESC_RCTL), KC_BSPC, KC_BSLS
),

/* FUNCTION (2)
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * |  F1  |  F2  |  F3  |  F4  |  F5  |  F6  |                    |  F7  |  F8  |  F9  | F10  | F11  | F12  |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | TRANS| TRANS| TRANS| TRANS| TRANS| TRANS|                    | TRANS| TRANS| TRANS| TRANS| TRANS| TRANS|
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | LAYER| TRANS| TRANS| TRANS| TRANS| TRANS|-------.    ,-------| TRANS| TRANS| TRANS| TRANS| TRANS| LOCK |
 * |------+------+------+------+------+------| TRANS |    | TRANS |------+------+------+------+------+------|
 * | TRANS| TRANS| TRANS| TRANS| TRANS| TRANS|-------|    |-------| TRANS| TRANS| TRANS| TRANS| TRANS| TRANS|
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *                   | TRANS| TRANS| TRANS| /TRANS  /       \TRANS \  | RCtl | DEL  | RAlt |
 *                   |      |      |      |/       /         \      \ |      |      |      |
 *                   `----------------------------'           '------''--------------------'
 */
[_FUNCTION] = LAYOUT(
  KC_F1,        KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,                             KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,
  KC_TRNS,      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                           KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
  TD(TD_LAYER), KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                           KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, TD(TD_RLAYER),
  KC_TRNS,      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,      KC_TRNS,    KC_TRNS,
                                              KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_RCTL, KC_DEL, KC_RALT
),

/* MEDIA (3)
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * | TRANS| TRANS| TRANS| TRANS| TRANS| TRANS|                    | TRANS| TRANS| TRANS| TRANS| TRANS| TRANS|
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |      |      | VOL- | VOL+ | MUTE | PRINT|                    | HOME |      |  UP  |      |      |      |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | LAYER|      | RGB- | RGB+ | RGBT |      |-------.    ,-------| PGUP | LEFT | DOWN | RIGHT|      | LOCK |
 * |------+------+------+------+------+------|       |    |  END  |------+------+------+------+------+------|
 * |      |      |      |      |      |      |-------|    |-------|PGDOWN|      |      |      |      |      |
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *                   | TRANS| TRANS| TRANS| /TRANS  /       \TRANS \  | TRANS| TRANS| TRANS|
 *                   |      |      |      |/       /         \      \ |      |      |      |
 *                   `----------------------------'           '------''--------------------'
 */
[_MEDIA] = LAYOUT(
  KC_TRNS,      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                                  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  KC_TRNS, KC_TRNS,
  KC_NO,        KC_NO,   KC_VOLD, KC_VOLU, KC_MUTE, KC_PRINT_SCREEN,                          KC_HOME, KC_NO,   KC_UP,   KC_NO,    KC_NO,   KC_NO,
  TD(TD_LAYER), KC_NO,   RM_VALD, RM_VALU, RM_TOGG, KC_NO,                                    KC_PGUP, KC_LEFT, KC_DOWN, KC_RIGHT, KC_NO,   TD(TD_RLAYER),
  KC_NO,        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,           KC_NO,   KC_END, KC_PGDN, KC_NO,   KC_NO,   KC_NO,        KC_NO,       KC_NO,
                                                      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
),

/* NUMBER (4)
 * ,-----------------------------------------.                   ,-----------------------------------------.
 * | TRANS| TRANS| TRANS| TRANS| TRANS| TRANS|                   | TRANS| TRANS| TRANS| TRANS| TRANS| TRANS|
 * |------+------+------+------+------+------|                   |------+------+------+------+------+------|
 * |      |      |   !  |   @  |   #  |      |                   |   *  |   7  |   8  |   9  |   -  |      |
 * |------+------+------+------+------+------|                   |------+------+------+------+------+------|
 * | LAYER|      |   $  |   %  |   ^  |      |-------.   ,-------|   0  |   4  |   5  |   6  |   +  | LOCK |
 * |------+------+------+------+------+------| TRANS |   | TRANS |------+------+------+------+------+------|
 * |      |      |   &  |   *  |   (  |   )  |-------|   |-------|   /  |   1  |   2  |   3  |   =  |      |
 * `-----------------------------------------/       /    \      \-----------------------------------------'
 *                    | TRANS| TRANS| TRANS| /TRANS  /      \TRANS \  | TRANS| TRANS| TRANS|
 *                    |      |      |      |/       /        \      \ |      |      |      |
 *                    `----------------------------'          '------''--------------------'
 */
[_NUMBER] = LAYOUT(
   KC_TRNS,      KC_TRNS, KC_TRNS,   KC_TRNS, KC_TRNS, KC_TRNS,                          KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  KC_TRNS,
   KC_NO,        KC_NO,   KC_EXLM,   KC_AT,   KC_HASH, KC_TRNS,                          KC_ASTR, KC_7,    KC_8,    KC_9,    KC_MINUS, KC_NO,
   TD(TD_LAYER), KC_TRNS, KC_DOLLAR, KC_PERC, KC_CIRC, KC_TRNS,                          KC_0,    KC_4,    KC_5,    KC_6,    KC_PLUS,  TD(TD_RLAYER),
   KC_TRNS,      KC_TRNS, KC_AMPR,   KC_ASTR, KC_LPRN, KC_RPRN, KC_TRNS, KC_TRNS, KC_SLSH, KC_1,    KC_2,    KC_3,        KC_EQUAL,     KC_NO,
                                              KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
),

/* GAMING (5)
 * ,-----------------------------------------.                   ,-----------------------------------------.
 * | TRANS| TRANS| TRANS| TRANS| TRANS| TRANS|                   | TRANS| TRANS| TRANS| TRANS| TRANS| TRANS|
 * |------+------+------+------+------+------|                   |------+------+------+------+------+------|
 * | TRANS| TRANS| TRANS| TRANS| TRANS| TRANS|                   | TRANS| TRANS| TRANS| TRANS| TRANS| TRANS|
 * |------+------+------+------+------+------|                   |------+------+------+------+------+------|
 * | LAYER| TRANS| TRANS| TRANS| TRANS| TRANS|-------.   ,-------| TRANS| TRANS| TRANS| TRANS| TRANS| LOCK |
 * |------+------+------+------+------+------| ENTER |   | TRANS |------+------+------+------+------+------|
 * | TRANS| TRANS| TRANS| TRANS| TRANS| TRANS|-------|   |-------| TRANS| TRANS| TRANS| TRANS| TRANS| TRANS|
 * `-----------------------------------------/       /    \      \-----------------------------------------'
 *                    | TRANS| ESC  | TRANS| /TRANS  /      \TRANS \  | TRANS| RWIN | TRANS|
 *                    |      |      |      |/       /        \      \ |      |      |      |
 *                    `----------------------------'          '------''--------------------'
 */
[_GAMING] = LAYOUT(
  KC_TRNS,      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                            KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  KC_TRNS,
  KC_TRNS,      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                            KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  KC_TRNS,
  TD(TD_LAYER), KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                            KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  TD(TD_RLAYER),
  KC_TRNS,      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_ENTER, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,     KC_TRNS,      KC_TRNS,
                                              KC_TRNS, KC_ESC,  KC_TRNS, KC_TRNS,  KC_TRNS, KC_TRNS, KC_RGUI, KC_TRNS
),
};

const keypos_t PROGMEM hand_swap_config[MATRIX_ROWS][MATRIX_COLS] = {
    {{0, 5}, {1, 5}, {2, 5}, {3, 5}, {4, 5}, {5, 5}},
    {{0, 6}, {1, 6}, {2, 6}, {3, 6}, {4, 6}, {5, 6}},
    {{0, 2}, {1, 7}, {2, 7}, {3, 7}, {4, 7}, {5, 7}},
    {{0, 8}, {1, 8}, {2, 8}, {3, 8}, {4, 8}, {5, 8}},
    {{0, 9}, {1, 9}, {2, 9}, {3, 9}, {4, 9}, {5, 9}},
    {{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}},
    {{0, 1}, {1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}},
    {{0, 7}, {1, 2}, {2, 2}, {3, 2}, {4, 2}, {5, 2}},
    {{0, 3}, {1, 3}, {2, 3}, {3, 3}, {4, 3}, {5, 3}},
    {{0, 4}, {1, 4}, {2, 4}, {3, 4}, {4, 4}, {5, 4}},
};
