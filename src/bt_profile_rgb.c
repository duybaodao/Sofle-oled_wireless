#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zmk/event_manager.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/rgb_underglow.h>
#include <zmk/ble.h>
#include <zmk/behavior.h>
#include <dt-bindings/zmk/rgb.h>
#include <stdbool.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if IS_ENABLED(CONFIG_ZMK_BLE) && (!IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL))

static int handle_ble_profile_changed(const zmk_event_t *eh) {
    const struct zmk_ble_active_profile_changed *ev =
        as_zmk_ble_active_profile_changed(eh);

    /* Only act on profiles 0 or 1; ignore others. */
    if (ev && (ev->index == 0 || ev->index == 1)) {
        // Red (0) or Blue (220)
        uint16_t hue = (ev->index == 1) ? 220 : 0;
        
        // Use behavior to ensure split sync works
        const struct device *rgb_ug_dev = zmk_behavior_get_binding("rgb_ug");
        
        if (rgb_ug_dev) {
            // 1. Turn ON (Global Sync)
            static struct zmk_behavior_binding binding_on = {
                .param1 = RGB_ON_CMD,
                .param2 = 0,
            };
            binding_on.behavior_dev = rgb_ug_dev;
            
            // 2. Set Color (Global Sync)
            static struct zmk_behavior_binding binding_color = {
                .param1 = RGB_COLOR_HSB_CMD,
            };
            binding_color.behavior_dev = rgb_ug_dev;
            binding_color.param2 = RGB_COLOR_HSB_VAL(hue, 100, 100);
            
            // Use a virtual position to avoid "No behavior assigned" warnings
            struct zmk_behavior_binding_event event = {
                .position = 2147483647, // INT32_MAX
                .timestamp = k_uptime_get()
            };
            
            // Invoke ON
            zmk_behavior_invoke_binding(&binding_on, event, true);
            zmk_behavior_invoke_binding(&binding_on, event, false);
            
            // Invoke Color
            zmk_behavior_invoke_binding(&binding_color, event, true);
            zmk_behavior_invoke_binding(&binding_color, event, false);
            
            LOG_INF("Profile %d active -> RGB ON + Hue %d (via behavior)", ev->index, hue);
        } else {
            LOG_ERR("Could not find rgb_ug behavior");
        }
    } else {
        LOG_DBG("Profile change ignored (index %d)", ev ? ev->index : -1);
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(bt_profile_rgb, handle_ble_profile_changed);
ZMK_SUBSCRIPTION(bt_profile_rgb, zmk_ble_active_profile_changed);

#endif /* IS_ENABLED(CONFIG_ZMK_BLE) */
