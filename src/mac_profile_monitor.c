/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 * 
 * This module monitors Bluetooth profile changes and raises
 * mac_connection_changed events when switching to/from the Mac profile.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zmk/event_manager.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <events/mac_connection_changed.h>
#include <zmk/ble.h>

#define MAC_BT_PROFILE 1

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if IS_ENABLED(CONFIG_ZMK_BLE) && (!IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL))

static int handle_ble_profile_changed(const zmk_event_t *eh) {
    const struct zmk_ble_active_profile_changed *ev =
        as_zmk_ble_active_profile_changed(eh);

    if (!ev) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    uint8_t idx = ev->index;
    bool is_mac = (idx == MAC_BT_PROFILE);

    // Raise custom Mac connection event
    struct zmk_mac_connection_changed mac_ev = {
        .connected = is_mac
    };
    
    raise_zmk_mac_connection_changed(mac_ev);
    
    LOG_INF("Mac connection changed: %s", is_mac ? "connected" : "disconnected");

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(mac_profile_monitor, handle_ble_profile_changed);
ZMK_SUBSCRIPTION(mac_profile_monitor, zmk_ble_active_profile_changed);

#endif /* IS_ENABLED(CONFIG_ZMK_BLE) */
