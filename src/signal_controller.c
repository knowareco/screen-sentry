#include "signal_controller.h"

bool is_connected = false;
signal_status_t current_signal_status = SIGNAL_OFF;

static bool bypass_timer_started = false;
static bool last_port1_state = false;
static bool last_port2_state = false;
static int64_t last_change_time = 0;

esp_err_t signal_controller_init(void) {
    current_signal_status = SIGNAL_OFF;
    is_connected = false;
    return ESP_OK;
}

esp_err_t save_signal_state(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_set_u8(nvs_handle, "signal_state", current_signal_status);
    if (err != ESP_OK) {
        nvs_close(nvs_handle);
        return err;
    }

    err = nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    return err;
}

esp_err_t load_signal_state(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        return err;
    }

    uint8_t state;
    err = nvs_get_u8(nvs_handle, "signal_state", &state);
    if (err == ESP_OK) {
        current_signal_status = (signal_status_t)state;
        gpio_set_level(HDMI_5V_CONTROL_GPIO, (state == SIGNAL_ON) ? 1 : 0);
    }

    nvs_close(nvs_handle);
    return ESP_OK;
}

bool check_connection_status(void) {
    bool current_port1 = gpio_get_level(HPD_PORT1_GPIO);
    bool current_port2 = gpio_get_level(HPD_PORT2_GPIO);
    int64_t current_time = esp_timer_get_time() / 1000; // Convert to ms

    if ((current_port1 != last_port1_state || current_port2 != last_port2_state) && (current_time - last_change_time > DEBOUNCE_TIME_MS)) {
        last_port1_state = current_port1;
        last_port2_state = current_port2;
        last_change_time = current_time;
        
        // Update global connection status
        is_connected = current_port1 && current_port2;
        
        // Log connection changes
        ESP_LOGI(TAG, "Connection status changed - Port1: %d, Port2: %d", current_port1, current_port2);
    }
    
    return is_connected;
}

bool check_for_bypass(void) {
    bool port1_connected = gpio_get_level(HPD_PORT1_GPIO);
    bool port2_connected = gpio_get_level(HPD_PORT2_GPIO);

    if (!port1_connected || !port2_connected) {
        // If either port is not connected, this may be a bypass scenario
        ESP_LOGW(TAG, "Potential bypass detected - Port1: %d, Port2: %d", port1_connected, port2_connected);
        return true;
    }

    return false;
}

void monitor_task(void *pvParameters) {
    ESP_LOGI(TAG, "Starting monitor task");
    int64_t bypass_start_time = 0;

    while (1) {
        check_connection_status();
        bool potential_bypass = check_for_bypass();

        if (potential_bypass && !bypass_timer_started) {
            // Start timing the potential bypass
            bypass_timer_started = true;
            bypass_start_time = esp_timer_get_time() / 1000; // Convert to ms
            ESP_LOGW(TAG, "Starting bypass detection timer");
        } else if (!potential_bypass) {
            // Reset bypass detection if connection is restored
            bypass_timer_started = false;
            ESP_LOGI(TAG, "Bypass condition cleared");
        }

        // If bypass has been detected for longer than threshold
        if (bypass_timer_started && (esp_timer_get_time() / 1000 - bypass_start_time > BYPASS_DETECTION_THRESHOLD_MS)) {
            ESP_LOGW(TAG, "Bypass confirmed after %d ms", BYPASS_DETECTION_THRESHOLD_MS);
            // TODO: Notify disconnect event
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Check every 100ms
    }
}

esp_err_t set_signal_state(signal_status_t new_state) {
    if (new_state == SIGNAL_ON) {
        // Only allow signal ON if both ports are connected
        if (!check_connection_status()) {
            ESP_LOGW(TAG, "Cannot enable signal - Both ports are not connected");
            return ESP_FAIL;
        }
    }

    gpio_set_level(HDMI_5V_CONTROL_GPIO, (new_state == SIGNAL_ON) ? 1 : 0); // Enable 5V line
    current_signal_status = new_state;

    // Save new state to NVS
    save_signal_state();

    ESP_LOGI(TAG, "Signal state changed to %s", (new_state == SIGNAL_ON) ? "ON" : "OFF");
    return ESP_OK;
}

void start_monitor_task(void) {
    xTaskCreate(monitor_task, "monitor_task", 2048, NULL, 5, NULL);
}

