#include <zephyr/kernel.h>

// Work item that will run periodically
static struct k_work_delayable log_work;

// Function called each time the work runs
static void log_work_handler(struct k_work *work) {
    printk(">>> usb_log_test.c: Hello every 3 seconds! <<<\n");

    // Reschedule to run again after 3000 ms
    k_work_reschedule(&log_work, K_MSEC(3000));
}

// Init function runs once at startup
static int usb_log_test_init(void) {
    // Initialize the delayed work item
    k_work_init_delayable(&log_work, log_work_handler);

    // Schedule first run after 3 seconds
    k_work_schedule(&log_work, K_MSEC(3000));

    printk(">>> usb_log_test.c: SYS_INIT started, periodic logging enabled <<<\n");
    return 0;
}

// Register init function to run at application startup
SYS_INIT(usb_log_test_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
