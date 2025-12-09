#include <zephyr/kernel.h>
#include <stdbool.h>
#include <zmk/event_manager.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/rgb_underglow.h>
#include <zmk/ble.h>
#include <zephyr/logging/log.h>

#warning "Compiling bt_profile_rgb.c"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static int handle_ble_profile_changed(const struct zmk_event_header *eh) {
    const struct zmk_ble_active_profile_changed *ev =
        (const struct zmk_ble_active_profile_changed *)eh;

    /* Only act on profiles 0 or 1; ignore others. */
    if (ev && (ev->index == 0 || ev->index == 1)) {
        struct zmk_led_hsb color = {.h = 0, .s = 100, .b = 30}; /* red for profile 0 */
        if (ev->index == 1) {
            color.h = 240; /* blue for profile 1 */
        }

        zmk_rgb_underglow_on();
        zmk_rgb_underglow_set_hsb(color);
        LOG_INF("Profile %d active -> hue %d", ev->index, color.h);
    } else {
        LOG_DBG("Profile change ignored (index %d)", ev ? ev->index : -1);
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(bt_profile_rgb, handle_ble_profile_changed);
ZMK_SUBSCRIPTION(bt_profile_rgb, zmk_ble_active_profile_changed);
