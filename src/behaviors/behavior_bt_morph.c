/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_bt_morph

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>
#include <zmk/behavior.h>
#include <zmk/ble.h>
#include <dt-bindings/zmk/keys.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/hid.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)

static int behavior_bt_morph_init(const struct device *dev) {
    return 0;
}

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    uint32_t keycode = binding->param1;

#if IS_ENABLED(CONFIG_ZMK_BLE)
    uint8_t active_profile = zmk_ble_active_profile_index();
    LOG_DBG("BT Morph: Profile %d, Code %d -> %d", active_profile, binding->param1, (active_profile == 1 ? binding->param2 : binding->param1));
    if (active_profile == 1) {
        keycode = binding->param2;
    }
#endif

    struct zmk_keycode_state_changed ev = {
        .usage_page = ZMK_HID_USAGE_PAGE(keycode),
        .keycode = ZMK_HID_USAGE_ID(keycode),
        .implicit_modifiers = 0,
        .state = true,
        .timestamp = event.timestamp,
    };
    
    raise_zmk_keycode_state_changed(ev);
    return 0;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    uint32_t keycode = binding->param1;

#if IS_ENABLED(CONFIG_ZMK_BLE)
    uint8_t active_profile = zmk_ble_active_profile_index();
    if (active_profile == 1) {
        keycode = binding->param2;
    }
#endif

    struct zmk_keycode_state_changed ev = {
        .usage_page = ZMK_HID_USAGE_PAGE(keycode),
        .keycode = ZMK_HID_USAGE_ID(keycode),
        .implicit_modifiers = 0,
        .state = false,
        .timestamp = event.timestamp,
    };

    raise_zmk_keycode_state_changed(ev);
    return 0;
}

static const struct behavior_driver_api behavior_bt_morph_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

BEHAVIOR_DT_INST_DEFINE(0, behavior_bt_morph_init, NULL, NULL, NULL,
                        POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                        &behavior_bt_morph_driver_api);

#endif /* !SPLIT OR CENTRAL */

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
