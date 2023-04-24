#include <string.h>
#include "button_control.h"
#include "esp_err.h"
#include "esp_log.h"

#define BTN_UP 1
#define BTN_DOWN 0
#define BTN_UNKNOWN -1
#define QUEUE_SIZE 10

#define BUTTON_CONTROL_POLL_DELAY 1000

typedef struct
{
    gpio_num_t gpio_num;
    char name[10];
    int (*btn_funct_pointer)();
    button_control_event_data_t button_event_data;
    esp_event_loop_handle_t loop_handle;
    TaskHandle_t myTaskHandle;
    int polling_delay_time;
} button_control_t;

typedef struct
{
    BUTTON_CONTROL_STATE_T state;
    int level;
    int prev_level;
    bool pressing;
    bool releasing;

} button_status; // button_status

ESP_EVENT_DEFINE_BASE(MY_EVENT_BASE);
static const char *TAG = "MyModule";

esp_err_t button_control_set_polling_time(button_control_handle_t button_handle, int polling_time) // sets the given polling_time to button's polling time
{
    button_control_t *button = button_handle;

    if (button == NULL)
    {
        ESP_LOGW(TAG, "Button not created!");
        return ESP_ERR_INVALID_ARG;
    }

    button->polling_delay_time = polling_time;
    return ESP_OK;
}

void button_polling_task(void *arg) // button polling task
{
    button_control_t *btn_ctl = (button_control_t *)(arg);
    button_status button1;
    clock_t start_pressing = 0.0, start_releasing = 0.0, end_pressing, end_releasing;
    button_control_event_data_t *button_event_data = &(btn_ctl->button_event_data);
    int sec;
    btn_ctl->polling_delay_time = BUTTON_CONTROL_POLL_DELAY;

    button1.prev_level = BTN_UNKNOWN;

    if (btn_ctl->btn_funct_pointer) // if function pointer is not null
    {
        button1.level = btn_ctl->btn_funct_pointer();
    }
    else
    {

        button1.level = gpio_get_level(btn_ctl->gpio_num); // get generic level function with gpio num
    }

    if (button1.level == BTN_DOWN)
    {
        button1.prev_level = BTN_UP;
        button1.pressing = true;
    }
    else
    {
        button1.prev_level = BTN_DOWN;
        button1.releasing = true;
    }

    while (1)
    {
        if (btn_ctl->btn_funct_pointer) // if function pointer is not null
        {
            button1.level = btn_ctl->btn_funct_pointer();
        }
        else
        {
            button1.level = gpio_get_level(btn_ctl->gpio_num); // get level function with gpio num
        }

        // we set the pins as pull-up registor so the pressed and released things are in negatif logic - pull-up registor olduğu için pressed ve released olayları ters
        if (button1.prev_level == BTN_UP && button1.level == BTN_DOWN) // in this if block, button's new state become pressed so we calculate the time taken during release time in here
        {
            if (button1.pressing == false) // if pressing was false then button pressed
            {
                start_pressing = clock();
                button1.pressing = true;
            }
            if (button1.releasing) // if button was releasing before then we calculate the time taken during release
            {
                end_releasing = clock();
                sec = ((end_releasing - start_releasing) / CLOCKS_PER_SEC);
                button_event_data->button_event_state = pressed;
                sprintf(button_event_data->name, "%s", btn_ctl->name);
                button_event_data->sec = sec;
                esp_event_post_to(btn_ctl->loop_handle, MY_EVENT_BASE, MY_EVENT_ID, (button_event_data), sizeof(button_control_event_data_t), 100);
                button1.releasing = false;
            }
            button1.state = pressed;
            button1.prev_level = BTN_DOWN;
        }
        else if (button1.prev_level == BTN_DOWN && button1.level == BTN_UP) // in this if block, button's new state become released so we calculate the time taken during press time in here
        {
            if (button1.releasing == false)
            {
                start_releasing = clock();
                button1.releasing = true;
            }
            if (button1.pressing)
            {
                end_pressing = clock();
                sec = ((end_pressing - start_pressing) / CLOCKS_PER_SEC);
                button_event_data->button_event_state = released;
                sprintf(button_event_data->name, "%s", btn_ctl->name);
                button_event_data->sec = sec;
                esp_event_post_to(btn_ctl->loop_handle, MY_EVENT_BASE, MY_EVENT_ID, (button_event_data), sizeof(button_control_event_data_t), 100);
                button1.pressing = false;
            }

            button1.state = released;
            button1.prev_level = BTN_UP;
        }

        vTaskDelay(pdMS_TO_TICKS(btn_ctl->polling_delay_time));
    }
}
// creates the button according to given parameters.
button_control_handle_t create_button_control(gpio_num_t gpio_num, char *name, event_handle_function_pointer function_p)
{
    button_control_t *btn_ctl = malloc(sizeof(button_control_t));
    xTaskCreate(button_polling_task, "button_polling_task", 4096, btn_ctl, 1, &btn_ctl->myTaskHandle);
    btn_ctl->btn_funct_pointer = NULL;
    btn_ctl->gpio_num = gpio_num;
    strcpy(btn_ctl->name, name);
    esp_event_loop_args_t loop_args = {
        .queue_size = QUEUE_SIZE,
        .task_name = "loop_task",
        .task_priority = 1,
        .task_stack_size = 4096,
        .task_core_id = 0};
    esp_event_loop_create(&loop_args, &btn_ctl->loop_handle);
    esp_event_handler_register_with(btn_ctl->loop_handle, ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, function_p, NULL);

    return (button_control_handle_t)btn_ctl;
}
// creates the button according to given parameters.
button_control_handle_t create_button_control_fp(integer_function_pointer fp, char *name, event_handle_function_pointer function_p)
{
    button_control_t *btn_ctl = malloc(sizeof(button_control_t));
    xTaskCreate(button_polling_task, "button_polling_task", 4096, btn_ctl, 1, &btn_ctl->myTaskHandle);
    btn_ctl->btn_funct_pointer = fp;
    btn_ctl->gpio_num = -1;
    strcpy(btn_ctl->name, name);
    esp_event_loop_args_t loop_args = {
        .queue_size = QUEUE_SIZE,
        .task_name = "loop_task",
        .task_priority = 1,
        .task_stack_size = 4096,
        .task_core_id = 0};
    esp_event_loop_create(&loop_args, &btn_ctl->loop_handle);
    esp_event_handler_register_with(btn_ctl->loop_handle, ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, function_p, NULL);

    return (button_control_handle_t)btn_ctl;
}
// deletes button and frees memory
void delete_button_control(button_control_handle_t button_handle, event_handle_function_pointer function_p)
{
    button_control_t *button = button_handle;
    esp_event_handler_unregister_with(button->loop_handle, MY_EVENT_BASE, MY_EVENT_ID, function_p);
    esp_event_loop_delete(button->loop_handle);
    vTaskDelete(button->myTaskHandle);
    free(button);
}
