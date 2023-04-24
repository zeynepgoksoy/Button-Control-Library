#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "freertos/queue.h"
#include "button_control.h"

int mybutton_getlevel()
{
    return gpio_get_level(GPIO_NUM_17);
}

void run_on_event(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data)
{
    button_control_event_data_t *button_event_data;
    button_event_data = ((button_control_event_data_t *)event_data);

    printf("\n\n");
    if (button_event_data->button_event_state == pressed)
    {
        printf("new button state : pressed ");
        printf(", time taken at released state: %d  ", button_event_data->sec);
    }
    else if (button_event_data->button_event_state == released)
    {
        printf("new button state : released ");
        printf(", time taken at pressed state: %d  ", button_event_data->sec);
    }
    else
    {
        printf("unkown state ! ");
        printf(", time taken at unknown state: %d  ", button_event_data->sec);
    }
    printf(" , event name : %s \n", button_event_data->name);
}

void app_main()
{

    gpio_set_direction(GPIO_NUM_14, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_14, GPIO_PULLUP_ONLY);

    gpio_set_direction(GPIO_NUM_17, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_17, GPIO_PULLUP_ONLY);

    gpio_set_direction(GPIO_NUM_26, GPIO_MODE_OUTPUT);

    button_control_handle_t btn = create_button_control(GPIO_NUM_14, "deneme1", run_on_event);
    // button_control_handle_t btn2 = create_button_control_fp(mybutton_getlevel, "deneme2", run_on_event);

    button_control_set_polling_time(btn, 200);
    vTaskDelay(pdMS_TO_TICKS(30000));
    delete_button_control(btn, run_on_event);
    // delete_button_control(btn2);
}
