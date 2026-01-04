/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zephyr/kernel.h>
#include <zmk/event_manager.h>

struct zmk_mac_connection_changed {
    bool connected;
};

ZMK_EVENT_DECLARE(zmk_mac_connection_changed);
