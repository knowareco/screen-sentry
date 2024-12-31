#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "common.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"

// WiFi credentials
#define WIFI_SSID "YOUR_SSID"
#define WIFI_PASS "YOUR_PASSWORD"

esp_err_t wifi_manager_init(void);

#endif // WIFI_MANAGER_H
