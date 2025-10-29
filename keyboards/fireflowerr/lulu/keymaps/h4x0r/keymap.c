// Copyright 2022 Cole Smith <cole@boadsource.xyz>
// SPDX-License-Identifier: GPL-2.0-or-later
#include QMK_KEYBOARD_H
#include "layout_dance.h"
#include "tap_hold_dance.h"
#include "process_key_override.h"
#include "keycodes.h"


enum layers {
    _QWERTY,
    _SYMBOL,
    _CONTROL,
};

const key_override_t *key_overrides[] = {
    &(ko_make_basic(MOD_MASK_SHIFT, KC_DOT, KC_COMMA)),
    &(ko_make_basic(MOD_MASK_ALT, KC_LSFT, KC_CAPS)),
    &(ko_make_basic(MOD_MASK_ALT, KC_RSFT, KC_CAPS)),
};

enum {
    TD_LAYER_LEFT,
    TD_LAYER_RIGHT,
    TD_ESC_RCTL,
    TD_DEL_RALT,
};

tap_dance_action_t tap_dance_actions[] = {
    [TD_LAYER_LEFT] = ACTION_LAYER_DANCE(TD_LAYER_RIGHT),
    [TD_LAYER_RIGHT] = ACTION_LAYER_DANCE(TD_LAYER_LEFT),
    [TD_ESC_RCTL] = TAP_HOLD_ACTION(KC_ESC, KC_RIGHT_CTRL),
    [TD_DEL_RALT] = TAP_HOLD_ACTION(KC_DEL, KC_RIGHT_ALT),
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
[_QWERTY] = LAYOUT(
    KC_GRV,            KC_1,   KC_2,    KC_3,    KC_4,    KC_5,                            KC_6,    KC_7,    KC_8,    KC_9, KC_0,    KC_MINS,
    KC_TAB,            KC_Q,   KC_W,    KC_E,    KC_R,    KC_T,                            KC_Y,    KC_U,    KC_I,    KC_O, KC_P,    KC_EQL,
    TD(TD_LAYER_LEFT), KC_A,   KC_S,    KC_D,    KC_F,    KC_G,                            KC_H,    KC_J,    KC_K,    KC_L, KC_SCLN, TD(TD_LAYER_RIGHT),
    KC_LSFT,           KC_Z,   KC_X,    KC_C,    KC_V,    KC_B, KC_LBRC,  KC_RBRC, KC_N,    KC_M,    KC_QUOT, KC_DOT,   KC_SLSH,     KC_RSFT,
                                                 KC_LALT, KC_LWIN, KC_LCTL, KC_SPC,   KC_ENT,  TD(TD_ESC_RCTL), KC_BSPC, TD(TD_DEL_RALT)
),
[_SYMBOL] = LAYOUT(
    KC_F1,      KC_F2,      KC_F3,   KC_F4,   KC_F5,   KC_F6,                              KC_F7,   KC_F8,   KC_F9, KC_F10, KC_F11,  KC_F12,
    S(KC_LBRC), S(KC_RBRC), KC_EXLM, KC_AT,   KC_HASH, KC_QUES,                            KC_BSLS, KC_7,    KC_8,  KC_9,   KC_MINS, KC_PLUS,
    KC_TRNS,    KC_DQUO,    KC_DLR,  KC_PERC, KC_CIRC, KC_COLN,                            KC_0,    KC_4,    KC_5,  KC_6,   KC_PIPE, KC_TRNS,
    KC_TRNS,    KC_UNDS,    KC_AMPR, KC_ASTR, KC_LPRN, KC_RPRN, KC_GRAVE, KC_TILD, KC_COMM, KC_1,    KC_2,  KC_3,       KC_LT,       KC_GT,
                                                 KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
),
[_CONTROL] = LAYOUT(
    KC_NO,   KC_NO, KC_NO,   KC_NO,   KC_NO,   KC_NO,                         KC_NO,   KC_NO,   KC_NO,   KC_NO,    KC_NO,   KC_NO,
    KC_NO,   KC_NO, KC_VOLD, KC_VOLU, KC_MUTE, KC_PSCR,                       KC_HOME, KC_NO,   KC_UP,   KC_NO,    KC_PGUP, KC_NO,
    KC_TRNS, KC_NO, RM_VALD, RM_VALU, RM_TOGG, KC_NO,                         KC_END,  KC_LEFT, KC_DOWN, KC_RIGHT, KC_PGDN, KC_TRNS,
    KC_TRNS, KC_NO, KC_NO,   KC_NO,   KC_NO,   KC_NO, KC_NO,   KC_NO, KC_NO,   KC_NO,   KC_NO,   KC_NO,        KC_NO,       KC_NO,
                                    KC_TRNS,    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,   KC_TRNS, KC_TRNS
)
};
