#include "gpio_manager.h"

esp_err_t gpio_manager_init(void) {
    // Configure HPD_PORT1_GPIO and HPD_PORT2_GPIO as inputs with internal pull-ups
    gpio_config_t io_conf_hpd = {
        .pin_bit_mask = ((1ULL << HPD_PORT1_GPIO) | (1ULL << HPD_PORT2_GPIO)),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf_hpd);

    // Configure HDMI_5V_CONTROL_GPIO as output, initially set to LOW (OFF)
    gpio_config_t io_conf_hdmi = {
        .pin_bit_mask = (1ULL << HDMI_5V_CONTROL_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf_hdmi);

    // Set initial state to OFF (cut 5V)
    gpio_set_level(HDMI_5V_CONTROL_GPIO, 0);

    return ESP_OK;
}

