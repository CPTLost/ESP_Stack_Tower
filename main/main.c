#include <stdio.h>
#include "graphics.h"
#include "return_t.h"
#include "button.h"
#include "esp_log.h"
#include "i2c4display.h"

#define LEFT_BTN_GPIO 9

static const char *TAG = "STACK TOWER";

return_t button_callback()
{
    ESP_EARLY_LOGI(TAG, "Button Callback");
    return success;
}

void app_main(void)
{
    // init i2c for and display
    ESP_ERROR_CHECK(initI2C(I2C_INTERFACE));
    ESP_ERROR_CHECK(graphics_init(I2C_INTERFACE, CONFIG_GRAPHICS_PIXELWIDTH, CONFIG_GRAPHICS_PIXELHEIGHT, 0, false, false));

    configure_button(LEFT_BTN_GPIO);

    graphics_startUpdate();
    graphics_clearScreen();
    graphics_println("First Try");
    graphics_println("=========");
    graphics_drawLine(10, 0, 20, 50, 1);
    graphics_finishUpdate();
}
