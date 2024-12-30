#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "driver/gpio.h"
#include "cJSON.h"

// Wi-Fi credentials
#define WIFI_SSID "YOUR_SSID"
#define WIFI_PASS "YOUR_PASSWORD"

// Define GPIO pins
#define HPD_PORT1_GPIO GPIO_NUM_34          // HDMI Port 1 HPD
#define HPD_PORT2_GPIO GPIO_NUM_35          // HDMI Port 2 HPD
#define HDMI_5V_CONTROL_GPIO GPIO_NUM_26     // HDMI 5V Control

// Current Signal Status
typedef enum {
    SIGNAL_OFF = 0,
    SIGNAL_ON = 1
} signal_status_t;

static signal_status_t current_signal_status = SIGNAL_OFF;

// Connected Status
static bool is_connected = false;

// CORS helper functions
static void set_cors_headers(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type, Authorization");
}

// Helper function to send JSON responses
static esp_err_t send_json_response(httpd_req_t *req, cJSON *json) {
    char *response = cJSON_PrintUnformatted(json);
    if (response == NULL) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "application/json");
    set_cors_headers(req);  // Ensure CORS headers are set
    httpd_resp_sendstr(req, response);
    free(response);
    return ESP_OK;
}

// Handle OPTIONS method for CORS preflight
static esp_err_t handle_options(httpd_req_t *req) {
    set_cors_headers(req);
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

// GET /api/connection-status handler
static esp_err_t handle_connection_status(httpd_req_t *req) {
    if (req->method == HTTP_OPTIONS) {
        handle_options(req);
        return ESP_OK;
    }

    // Create JSON object
    cJSON *json = cJSON_CreateObject();
    if (json == NULL) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    // Update connection status
    is_connected = gpio_get_level(HPD_PORT1_GPIO) && gpio_get_level(HPD_PORT2_GPIO);

    cJSON_AddBoolToObject(json, "connected", is_connected);
    cJSON *ports = cJSON_CreateObject();
    cJSON_AddBoolToObject(ports, "port1", gpio_get_level(HPD_PORT1_GPIO));
    cJSON_AddBoolToObject(ports, "port2", gpio_get_level(HPD_PORT2_GPIO));
    cJSON_AddItemToObject(json, "ports", ports);

    // Send JSON response
    send_json_response(req, json);
    cJSON_Delete(json);
    return ESP_OK;
}

// GET /api/signal-status handler
static esp_err_t handle_signal_status(httpd_req_t *req) {
    if (req->method == HTTP_OPTIONS) {
        handle_options(req);
        return ESP_OK;
    }

    // Create JSON object
    cJSON *json = cJSON_CreateObject();
    if (json == NULL) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    const char *status_str = (current_signal_status == SIGNAL_ON) ? "On" : "Off";
    cJSON_AddStringToObject(json, "status", status_str);

    // Send JSON response
    send_json_response(req, json);
    cJSON_Delete(json);
    return ESP_OK;
}

// POST /api/set-signal handler
static esp_err_t handle_set_signal(httpd_req_t *req) {
    if (req->method == HTTP_OPTIONS) {
        handle_options(req);
        return ESP_OK;
    }

    char content[100];
    int ret, remaining = req->content_len;

    if (req->content_len >= sizeof(content)) {
        // Request body too large
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Request body too large");
        return ESP_FAIL;
    }

    // Read the request body
    while (remaining > 0) {
        if ((ret = httpd_req_recv(req, content, remaining)) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                // Retry receiving data
                continue;
            }
            // In case of other errors, respond with 500
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        remaining -= ret;
    }

    // Null-terminate the received content
    content[req->content_len] = '\0';

    // Parse JSON
    cJSON *json = cJSON_Parse(content);
    if (json == NULL) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }

    cJSON *status_item = cJSON_GetObjectItemCaseSensitive(json, "status");
    if (!cJSON_IsString(status_item) || (status_item->valuestring == NULL)) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }

    // Set signal status based on received value
    if (strcmp(status_item->valuestring, "ON") == 0) {
        gpio_set_level(HDMI_5V_CONTROL_GPIO, 1);  // Enable HDMI 5V
        current_signal_status = SIGNAL_ON;
    } else if (strcmp(status_item->valuestring, "OFF") == 0) {
        gpio_set_level(HDMI_5V_CONTROL_GPIO, 0);  // Disable HDMI 5V
        current_signal_status = SIGNAL_OFF;
    } else {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid status value");
        return ESP_FAIL;
    }

    // Create response JSON
    cJSON *response_json = cJSON_CreateObject();
    if (response_json == NULL) {
        cJSON_Delete(json);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    cJSON_AddBoolToObject(response_json, "success", true);
    const char *new_status_str = (current_signal_status == SIGNAL_ON) ? "ON" : "OFF";
    cJSON_AddStringToObject(response_json, "status", new_status_str);

    // Send response
    send_json_response(req, response_json);
    cJSON_Delete(response_json);
    cJSON_Delete(json);
    return ESP_OK;
}

// Define URI handlers
static const httpd_uri_t hpd_status_uri = {
    .uri       = "/api/connection-status",
    .method    = HTTP_GET,
    .handler   = handle_connection_status,
    .user_ctx  = NULL
};

static const httpd_uri_t signal_status_uri = {
    .uri       = "/api/signal-status",
    .method    = HTTP_GET,
    .handler   = handle_signal_status,
    .user_ctx  = NULL
};

static const httpd_uri_t set_signal_uri = {
    .uri       = "/api/set-signal",
    .method    = HTTP_POST,
    .handler   = handle_set_signal,
    .user_ctx  = NULL
};

// HTTP server instance
static httpd_handle_t server_handle = NULL;

// Function to register URI handlers
static void register_handlers(httpd_handle_t server) {
    httpd_register_uri_handler(server, &hpd_status_uri);
    httpd_register_uri_handler(server, &signal_status_uri);
    httpd_register_uri_handler(server, &set_signal_uri);
}

// Function to start the web server
static httpd_handle_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    if (httpd_start(&server_handle, &config) == ESP_OK) {
        // Register URI handlers
        register_handlers(server_handle);
        return server_handle;
    }

    return NULL;
}

// Function to stop the web server
static void stop_webserver(httpd_handle_t server) {
    if (server) {
        httpd_stop(server);
    }
}

// Function to initialize GPIOs
static void init_gpio(void) {
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
}

// Wi-Fi event handler
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    char addr_str[16];
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        esp_ip4addr_ntoa(&event->ip_info.ip, addr_str, sizeof(addr_str));
    }
}

// Function to initialize Wi-Fi
static void wifi_init(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Create default Wi-Fi station
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register event handlers
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    // Configure Wi-Fi
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. In case your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

}

void app_main(void) {
    // Initialize GPIOs
    init_gpio();

    // Initialize Wi-Fi
    wifi_init();

    // Start the web server
    server_handle = start_webserver();

    // TODO: Handle graceful shutdown
}

