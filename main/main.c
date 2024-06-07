#include <stdio.h>
#include "graphics.h"
#include "return_t.h"
#include "button.h"
#include "esp_log.h"

#define LEFT_BTN_GPIO 9

const char *TAG = "STACK TOWER";

return_t button_callback()
{
    ESP_EARLY_LOGI(TAG, "Button Callback");
    return success;
}

void app_main(void)
{
    configure_button(LEFT_BTN_GPIO);
}
