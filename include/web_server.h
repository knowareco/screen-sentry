#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "common.h"
#include "esp_http_server.h"
#include "cJSON.h"
#include "signal_controller.h"

extern bool is_connected;
extern signal_status_t current_signal_status;

esp_err_t start_webserver(void);
void stop_webserver(httpd_handle_t server);

#endif // WEB_SERVER_H
