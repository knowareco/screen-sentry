#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"

#define TAG "SCREEN_SENTRY"

// GPIO definitions
#define HPD_PORT1_GPIO GPIO_NUM_34          // HDMI Port 1 HPD
#define HPD_PORT2_GPIO GPIO_NUM_35          // HDMI Port 2 HPD
#define HDMI_5V_CONTROL_GPIO GPIO_NUM_26    // HDMI 5V Control

// Timing constants
#define DEBOUNCE_TIME_MS 50
#define BYPASS_DETECTION_THRESHOLD_MS 1000

typedef enum {
    SIGNAL_OFF = 0,
    SIGNAL_ON = 1
} signal_status_t;

#endif // COMMON_H
