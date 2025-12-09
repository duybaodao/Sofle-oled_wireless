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
        struct zmk_led_hsb color = {.h = 0, .s = 100, .b = 100}; /* red for profile 0 */
        
        if (ev->index == 1) {
            color.h = 220; /* blue for profile 1 */
        }

        zmk_rgb_underglow_on();
        zmk_rgb_underglow_set_hsb(color);
        
        LOG_INF("Profile %d active -> Set Hue %d (Direct API)", ev->index, color.h);
    } else {
        LOG_DBG("Profile change ignored (index %d)", ev ? ev->index : -1);
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(bt_profile_rgb, handle_ble_profile_changed);
ZMK_SUBSCRIPTION(bt_profile_rgb, zmk_ble_active_profile_changed);

#endif /* IS_ENABLED(CONFIG_ZMK_BLE) */
