#include "common.h"
#include "gpio_manager.h"
#include "wifi_manager.h"
#include "web_server.h"
#include "signal_controller.h"

void app_main(void) {
    ESP_LOGI(TAG, "Starting Screen Sentry");

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize components
    ESP_ERROR_CHECK(gpio_manager_init());
    ESP_ERROR_CHECK(signal_controller_init());
    ESP_ERROR_CHECK(wifi_manager_init());

    // Start monitoring task
    start_monitor_task();

    // Start the web server
    ESP_ERROR_CHECK(start_webserver());

    ESP_LOGI(TAG, "Initialization complete");
}

