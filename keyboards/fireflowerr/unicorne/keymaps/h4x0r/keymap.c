#include QMK_KEYBOARD_H
#include "process_tap_dance.h"
#include "process_key_override.h"
#include "process_combo.h"
#include "keycodes.h"
#include "lib/layout_dance.h"

// inverted curly braces - shift press for square
enum custom_keycodes {
    CK_LCBR = SAFE_RANGE,
    CK_RCBR,
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case CK_LCBR:
            if (record->event.pressed) {
                if (get_mods() & MOD_MASK_SHIFT) {
                    uint8_t mods = get_mods();
                    unregister_mods(mods & MOD_MASK_SHIFT);
                    register_code(KC_LBRC);
                    register_mods(mods);
                } else {
                    register_code16(KC_LCBR);
                }
            } else {
                unregister_code(KC_LBRC);
                unregister_code16(KC_LCBR);
            }
            return false;
        case CK_RCBR:
            if (record->event.pressed) {
                if (get_mods() & MOD_MASK_SHIFT) {
                    uint8_t mods = get_mods();
                    unregister_mods(mods & MOD_MASK_SHIFT);
                    register_code(KC_RBRC);
                    register_mods(mods);
                } else {
                    register_code16(KC_RCBR);
                }
            } else {
                unregister_code(KC_RBRC);
                unregister_code16(KC_RCBR);
            }
            return false;
        default:
            return true;
    }
}

// double shift is caps
const uint16_t PROGMEM shift_combo[] = {KC_LSFT, KC_RSFT, COMBO_END};
combo_t key_combos[] = {
    COMBO(shift_combo, KC_CAPS)
};

enum layers {
    _QWERTY,
    _SYMBOL,
    _CONTROL,
    _RUNESCAPE,
};

const key_override_t *key_overrides[] = {
    &(ko_make_basic(MOD_MASK_SHIFT, KC_BSPC, KC_DEL)),
    &(ko_make_basic(MOD_MASK_SHIFT, KC_QUOT, KC_DQUO)),
    &(ko_make_basic(MOD_MASK_SHIFT, KC_DOT, KC_COMM)),
};

enum {
    TD_LAYER_LEFT,
    TD_LAYER_RIGHT,
    TD_ESC_RCTL,
    TD_DEL_RALT,
};

tap_dance_action_t tap_dance_actions[] = {
    [TD_LAYER_LEFT] = ACTION_LAYER_DANCE(TD_LAYER_RIGHT),
    [TD_LAYER_RIGHT] = ACTION_LAYER_DANCE(TD_LAYER_LEFT)
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
[_QWERTY] = LAYOUT(
        KC_TAB,            KC_Q,    KC_W,    KC_E,   KC_R,   KC_T,   KC_Y, KC_U, KC_I,    KC_O,   KC_P,    KC_BSPC,
        TD(TD_LAYER_LEFT), KC_A,    KC_S,    KC_D,   KC_F,   KC_G,   KC_H, KC_J, KC_K,    KC_L,   KC_SCLN, TD(TD_LAYER_RIGHT),
        KC_LSFT,           KC_Z,    KC_X,    KC_C,   KC_V,   KC_B,   KC_N, KC_M, KC_QUOT, KC_DOT, KC_SLSH, KC_RSFT,
                                    KC_LALT, KC_LCTL, KC_SPC,  KC_ENT, KC_ESC, KC_RWIN
),
[_SYMBOL] = LAYOUT(
        KC_GRV, KC_TILD,  KC_EXLM, KC_AT,   KC_HASH, KC_LPRN, KC_RPRN, KC_1, KC_2, KC_3, KC_EQL,    KC_MINS,
        KC_TRNS, KC_PIPE, KC_DLR,  KC_PERC, KC_CIRC, CK_LCBR, CK_RCBR, KC_4, KC_5, KC_6, KC_PLUS, KC_TRNS,
        KC_TRNS, KC_BSLS, KC_AMPR, KC_ASTR, KC_UNDS, KC_LABK, KC_RABK, KC_7, KC_8, KC_9, KC_0,    KC_TRNS,
                                    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
),
[_CONTROL] = LAYOUT(
        QK_BOOT,   KC_F1, KC_F2,  KC_F3, KC_F4,   RM_VALU, KC_PGUP, KC_NO,   KC_UP,   KC_NO,    KC_HOME, KC_PSCR,
        KC_TRNS, KC_F5, KC_F6,  KC_F7, KC_F8,   RM_VALD, KC_PGDN, KC_LEFT, KC_DOWN, KC_RIGHT, KC_END,  KC_TRNS,
        KC_TRNS, KC_F9, KC_F10, KC_F11, KC_F12, RM_TOGG, KC_VOLD, KC_VOLU, KC_MUTE, KC_BRID,  KC_BRIU, KC_TRNS,
                                    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
),
[_RUNESCAPE] = LAYOUT(
        KC_TRNS, KC_F1, KC_F2,    KC_UP,   KC_F3,   KC_TRNS, KC_TRNS, KC_TRNS,   KC_TRNS,   KC_TRNS,    KC_TRNS, KC_TRNS,
        KC_TRNS, KC_F4, KC_LEFT,  KC_DOWN, KC_RIGHT,   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  KC_TRNS,
        KC_TRNS, KC_F5, KC_1,     KC_2,    KC_3, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  KC_TRNS, KC_TRNS,
                                    KC_TRNS, KC_ESC, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
),
};
