#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zmk/event_manager.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/ble.h>
#include <zmk/keymap.h>
#include <stdbool.h>

#define RGB_LAYER 4
#define RGB_BT1_KEY 32 // green
#define RGB_BT2_KEY 31 // blue

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if IS_ENABLED(CONFIG_ZMK_BLE) && (!IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL))

static int handle_ble_profile_changed(const zmk_event_t *eh) {
    const struct zmk_ble_active_profile_changed *ev =
        as_zmk_ble_active_profile_changed(eh);

    if (!ev) {
        return ZMK_EV_EVENT_BUBBLE;
    }
    uint8_t idx = ev->index;

    // Initialize pos to 0, though we will set it in the switch
    uint32_t pos = 0;
    switch (idx) {
        case 0:
            pos = RGB_BT1_KEY;
            LOG_INF("BT Profile 0 (Windows) - will trigger RGB key %d", pos);
            break;
        case 1:
            pos = RGB_BT2_KEY;
            LOG_INF("BT Profile 1 (Mac) - will trigger RGB key %d", pos);
            break;
        default:
            break;
    }

    if (pos != 0) {
        // Simulate physical keypress on Adjust layer to trigger RGB binding
        // This ensures the event is processed by Split Manager for synchronization.
        zmk_keymap_layer_activate(RGB_LAYER);
        k_busy_wait(10000); // Allow layer state to settle

        struct zmk_position_state_changed press_ev = {
            .source = 0, // Local
            .position = pos,
            .state = true,
            .timestamp = k_uptime_get()
        };
        raise_zmk_position_state_changed(press_ev);

        k_busy_wait(50000); // Wait for processing

        struct zmk_position_state_changed release_ev = {
            .source = 0, // Local
            .position = pos,
            .state = false,
            .timestamp = k_uptime_get()  
        };
        raise_zmk_position_state_changed(release_ev);

        zmk_keymap_layer_deactivate(RGB_LAYER);
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(bt_profile_rgb, handle_ble_profile_changed);
ZMK_SUBSCRIPTION(bt_profile_rgb, zmk_ble_active_profile_changed);

#endif /* IS_ENABLED(CONFIG_ZMK_BLE) */
