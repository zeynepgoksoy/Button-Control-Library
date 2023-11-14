##### Library that calculate time passed according to button's state if it's pressed or released. #####

* **esp_err_t button_control_set_polling_time(button_control_handle_t button_handle, int polling_time)** function sets button's polling_delay_time to the given parameter.

* **button_control_handle_t create_button_control(gpio_num_t gpio_num, char \*name, event_handle_function_pointer function_p)** function creates a button and calculates time according to given gpio pin number with default gpio functions.

* **button_control_handle_t create_button_control_fp(integer_function_pointer fp, char \*name, event_handle_function_pointer function_p)** function takes function pointer as parameter, creates a button and calculates time according to given function's methods.

* **void delete_button_control(button_control_handle_t button_handle, event_handle_function_pointer function_p)** function deletes button and it's handlers then frees the allocated memory for button.
