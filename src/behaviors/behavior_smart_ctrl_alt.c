/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 * 
 * Smart Ctrl/Alt Modifier Behavior
 * 
 * Acts as Ctrl by default, but when Tab is pressed while held,
 * switches to Alt and maintains Alt state for window switching.
 */

#define DT_DRV_COMPAT zmk_behavior_smart_ctrl_alt

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>
#include <zmk/behavior.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/hid.h>
#include <dt-bindings/zmk/keys.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)

// State machine states
typedef enum {
    STATE_INACTIVE,      // Modifier not pressed
    STATE_CTRL_ACTIVE,   // Ctrl is pressed and active
    STATE_ALT_ACTIVE,    // Alt is pressed and active (after Tab was pressed)
} modifier_state_t;

static modifier_state_t current_state = STATE_INACTIVE;
static bool modifier_is_held = false;

// Tab key usage ID (HID usage for Tab)
#define TAB_USAGE_ID 0x2B

static int behavior_smart_ctrl_alt_init(const struct device *dev) {
    current_state = STATE_INACTIVE;
    modifier_is_held = false;
    return 0;
}

// Handle modifier key press
static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    LOG_DBG("Smart Ctrl/Alt: Modifier pressed");
    
    modifier_is_held = true;
    current_state = STATE_CTRL_ACTIVE;
    
    // Send Ctrl press
    struct zmk_keycode_state_changed ev = {
        .usage_page = HID_USAGE_KEY,
        .keycode = HID_USAGE_KEY_KEYBOARD_LEFTCONTROL,
        .implicit_modifiers = 0,
        .state = true,
        .timestamp = event.timestamp,
    };
    
    raise_zmk_keycode_state_changed(ev);
    return 0;
}

// Handle modifier key release
static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    LOG_DBG("Smart Ctrl/Alt: Modifier released, state=%d", current_state);
    
    modifier_is_held = false;
    
    // Release whichever modifier is currently active
    uint32_t keycode_to_release;
    if (current_state == STATE_ALT_ACTIVE) {
        keycode_to_release = HID_USAGE_KEY_KEYBOARD_LEFTALT;
    } else {
        keycode_to_release = HID_USAGE_KEY_KEYBOARD_LEFTCONTROL;
    }
    
    struct zmk_keycode_state_changed ev = {
        .usage_page = HID_USAGE_KEY,
        .keycode = keycode_to_release,
        .implicit_modifiers = 0,
        .state = false,
        .timestamp = event.timestamp,
    };
    
    raise_zmk_keycode_state_changed(ev);
    
    // Reset state
    current_state = STATE_INACTIVE;
    return 0;
}

// Event listener to monitor Tab keypresses
static int handle_keycode_state_changed(const zmk_event_t *eh) {
    const struct zmk_keycode_state_changed *ev = as_zmk_keycode_state_changed(eh);
    
    if (!ev || !modifier_is_held) {
        return ZMK_EV_EVENT_BUBBLE;
    }
    
    // Check if Tab was pressed
    if (ev->usage_page == HID_USAGE_KEY && 
        ev->keycode == TAB_USAGE_ID && 
        ev->state == true) {
        
        // If we're in CTRL_ACTIVE state and Tab is pressed, switch to Alt
        if (current_state == STATE_CTRL_ACTIVE) {
            LOG_DBG("Smart Ctrl/Alt: Tab pressed, switching to Alt");
            
            // Release Ctrl
            struct zmk_keycode_state_changed ctrl_release = {
                .usage_page = HID_USAGE_KEY,
                .keycode = HID_USAGE_KEY_KEYBOARD_LEFTCONTROL,
                .implicit_modifiers = 0,
                .state = false,
                .timestamp = ev->timestamp,
            };
            raise_zmk_keycode_state_changed(ctrl_release);
            
            // Press Alt
            struct zmk_keycode_state_changed alt_press = {
                .usage_page = HID_USAGE_KEY,
                .keycode = HID_USAGE_KEY_KEYBOARD_LEFTALT,
                .implicit_modifiers = 0,
                .state = true,
                .timestamp = ev->timestamp,
            };
            raise_zmk_keycode_state_changed(alt_press);
            
            // Update state
            current_state = STATE_ALT_ACTIVE;
        }
    }
    
    return ZMK_EV_EVENT_BUBBLE;
}

static const struct behavior_driver_api behavior_smart_ctrl_alt_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

BEHAVIOR_DT_INST_DEFINE(0, behavior_smart_ctrl_alt_init, NULL, NULL, NULL,
                        POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                        &behavior_smart_ctrl_alt_driver_api);

// Register event listener
ZMK_LISTENER(smart_ctrl_alt_listener, handle_keycode_state_changed);
ZMK_SUBSCRIPTION(smart_ctrl_alt_listener, zmk_keycode_state_changed);

#endif /* !SPLIT OR CENTRAL */

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
