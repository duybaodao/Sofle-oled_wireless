/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 * 
 * This module listens for Mac connection events and activates/deactivates
 * the Mac layer accordingly.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zmk/event_manager.h>
#include <events/mac_connection_changed.h>
#include <zmk/keymap.h>

#define MAC_LAYER 1

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if IS_ENABLED(CONFIG_ZMK_BLE) && (!IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL))

static int handle_mac_connection_changed(const zmk_event_t *eh) {
    const struct zmk_mac_connection_changed *ev =
        as_zmk_mac_connection_changed(eh);

    if (!ev) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    if (ev->connected) {
        zmk_keymap_layer_activate(MAC_LAYER);
        LOG_INF("Mac layer activated");
    } else {
        zmk_keymap_layer_deactivate(MAC_LAYER);
        LOG_INF("Mac layer deactivated");
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(mac_layer_handler, handle_mac_connection_changed);
ZMK_SUBSCRIPTION(mac_layer_handler, zmk_mac_connection_changed);

#endif /* IS_ENABLED(CONFIG_ZMK_BLE) */
