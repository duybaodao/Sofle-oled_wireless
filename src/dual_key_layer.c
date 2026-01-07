/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/keymap.h>
#include <stdbool.h>

#define DUAL_KEY_LAYER 4
#define LEFT_MOD_POS 53
#define RIGHT_MOD_POS 56

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)

// Track the state of both keys
static bool left_mod_held = false;
static bool right_mod_held = false;
static bool dual_layer_active = false;

static int handle_position_state_changed(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *ev = as_zmk_position_state_changed(eh);

    if (!ev) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    uint32_t pos = ev->position;
    bool pressed = ev->state;

    // Track the state of our monitored keys
    if (pos == LEFT_MOD_POS) {
        left_mod_held = pressed;
    } else if (pos == RIGHT_MOD_POS) {
        right_mod_held = pressed;
    } else {
        // Not one of our monitored keys, ignore
        return ZMK_EV_EVENT_BUBBLE;
    }

    // Check if both keys are now held
    bool both_held = left_mod_held && right_mod_held;

    // Activate layer 2 when both keys are held
    if (both_held && !dual_layer_active) {
        zmk_keymap_layer_activate(DUAL_KEY_LAYER);
        dual_layer_active = true;
    }
    // Deactivate layer 2 when either key is released
    else if (!both_held && dual_layer_active) {
        zmk_keymap_layer_deactivate(DUAL_KEY_LAYER);
        dual_layer_active = false;
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(dual_key_layer, handle_position_state_changed);
ZMK_SUBSCRIPTION(dual_key_layer, zmk_position_state_changed);

#endif /* !SPLIT OR CENTRAL */
