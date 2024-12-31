#include "web_server.h"

static httpd_handle_t server_handle = NULL;

static void set_cors_headers(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type, Authorization");
}

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
    int ret = ESP_FAIL;
    int remaining = req->content_len;

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
        ret = set_signal_state(SIGNAL_ON);
    } else if (strcmp(status_item->valuestring, "OFF") == 0) {
        ret = set_signal_state(SIGNAL_OFF);
    } else {
        ret = ESP_FAIL;
    }

    if (ret != ESP_OK) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to set signal state");
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

// Function to register URI handlers
static void register_handlers(httpd_handle_t server) {
    httpd_register_uri_handler(server, &hpd_status_uri);
    httpd_register_uri_handler(server, &signal_status_uri);
    httpd_register_uri_handler(server, &set_signal_uri);
}

// Function to start the web server
esp_err_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    if (httpd_start(&server_handle, &config) == ESP_OK) {
        // Register URI handlers
        register_handlers(server_handle);
        return ESP_OK;
    }

    return ESP_FAIL;
}

// Function to stop the web server
void stop_webserver(httpd_handle_t server) {
    if (server) {
        httpd_stop(server);
    }
}
