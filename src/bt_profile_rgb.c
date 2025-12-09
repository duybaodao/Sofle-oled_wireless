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
        // Red (0) or Blue (198) matches user keymap
        uint16_t hue = (ev->index == 1) ? 198 : 0;
        
        const struct device *rgb_ug_dev = zmk_behavior_get_binding("rgb_ug");
        
        if (rgb_ug_dev) {
            // 1. Turn ON
            static struct zmk_behavior_binding binding_on = {
                .param1 = RGB_ON_CMD,
                .param2 = 0,
            };
            binding_on.behavior_dev = rgb_ug_dev;
            
            // 2. Set Color
            static struct zmk_behavior_binding binding_color = {
                .param1 = RGB_COLOR_HSB_CMD,
            };
            binding_color.behavior_dev = rgb_ug_dev;
            binding_color.param2 = RGB_COLOR_HSB_VAL(hue, 100, 100);
            
            struct zmk_behavior_binding_event event = {
                .position = 0, 
                .timestamp = k_uptime_get()
            };
            
            // Invoke ON
            zmk_behavior_invoke_binding(&binding_on, event, true);
            zmk_behavior_invoke_binding(&binding_on, event, false);
            
            k_busy_wait(50000); // 50ms wait
            
            // Invoke Color
            zmk_behavior_invoke_binding(&binding_color, event, true);
            zmk_behavior_invoke_binding(&binding_color, event, false);
            
            LOG_INF("Profile %d -> ON + Hue %d (Behavior)", ev->index, hue);
        } else {
             // Fallback to direct local control if behavior not found
             struct zmk_led_hsb color = {.h = hue, .s = 100, .b = 100};
             zmk_rgb_underglow_on();
             zmk_rgb_underglow_set_hsb(color);
             LOG_WRN("Behavior not found, using direct API (Local only)");
        }
    } else {
        LOG_DBG("Profile change ignored (index %d)", ev ? ev->index : -1);
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(bt_profile_rgb, handle_ble_profile_changed);
ZMK_SUBSCRIPTION(bt_profile_rgb, zmk_ble_active_profile_changed);

#endif /* IS_ENABLED(CONFIG_ZMK_BLE) */
