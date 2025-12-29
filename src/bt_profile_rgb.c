#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zmk/event_manager.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/ble.h>
#include <zmk/keymap.h>
#include <stdbool.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if IS_ENABLED(CONFIG_ZMK_BLE) && (!IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL))

static int handle_ble_profile_changed(const zmk_event_t *eh) {
    const struct zmk_ble_active_profile_changed *ev =
        as_zmk_ble_active_profile_changed(eh);

    /* Only act on profiles 0 or 1 */
    if (ev && (ev->index == 0 || ev->index == 1)) {
        // Purple (Pos 30) or Blue (Pos 31) based on user keymap (Layer 3)
        uint32_t pos = (ev->index == 1) ? 31 : 30;

        // Simulate physical keypress on Adjust layer (3) to trigger RGB binding
        // This ensures the event is processed by Split Manager for synchronization.
        zmk_keymap_layer_activate(3);
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

        zmk_keymap_layer_deactivate(3);

        LOG_INF("Profile %d -> Synced RGB via Key %d (Layer 3)", ev->index, pos);
    } else {
        LOG_DBG("Profile change ignored (index %d)", ev ? ev->index : -1);
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(bt_profile_rgb, handle_ble_profile_changed);
ZMK_SUBSCRIPTION(bt_profile_rgb, zmk_ble_active_profile_changed);

#endif /* IS_ENABLED(CONFIG_ZMK_BLE) */
