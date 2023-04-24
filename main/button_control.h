#ifndef _BUTTON_CONTROL_H_
#define _BUTTON_CONTROL_H_

#include <stdio.h>
#include <time.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "freertos/queue.h"
#include "esp_event.h"

#define MY_EVENT_ID 1
ESP_EVENT_DECLARE_BASE(MY_EVENT_BASE);

typedef void (*event_handle_function_pointer)();
typedef int (*integer_function_pointer)();

typedef enum
{
    released,
    pressed,
} BUTTON_CONTROL_STATE_T;

typedef struct
{
    char name[10];
    int sec;
    BUTTON_CONTROL_STATE_T button_event_state; // shows button state : pressed or released

} button_control_event_data_t;

typedef void *button_control_handle_t;

esp_err_t button_control_set_polling_time(button_control_handle_t button_handle, int polling_time);
button_control_handle_t create_button_control(gpio_num_t gpio_num, char *name, event_handle_function_pointer function_p);
button_control_handle_t create_button_control_fp(integer_function_pointer fp, char *name, event_handle_function_pointer function_p);
void delete_button_control(button_control_handle_t button_handle, event_handle_function_pointer function_p);

#endif