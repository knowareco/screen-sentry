#ifndef SIGNAL_CONTROLLER_H
#define SIGNAL_CONTROLLER_H

#include "common.h"
#include "driver/gpio.h"
#include "nvs.h"
#include "nvs_flash.h"

extern bool is_connected;
extern signal_status_t current_signal_status;

esp_err_t signal_controller_init(void);
esp_err_t set_signal_state(signal_status_t new_state);
bool check_connection_status(void);
bool check_for_bypass(void);
void start_monitor_task(void);

#endif // SIGNAL_CONTROLLER_H
