#include <stdio.h>
#include "graphics.h"
#include "return_t.h"
#include "button.h"
#include "esp_log.h"
#include "i2c4display.h"
#include "stack_tower.h"

#define LEFT_BTN_GPIO 9
#define HEIGHT_OF_BLOCK 10
#define WIDTH_OF_BLOCK 30

static const char *TAG = "STACK TOWER";

bool button_pressed = false; // TEMP
return_t button_callback()
{
    ESP_EARLY_LOGI(TAG, "Button Callback");
    button_pressed = true;
    // stop_block = !stop_block;
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
    graphics_finishUpdate();

    uint8_t x_start = CONFIG_GRAPHICS_PIXELWIDTH;
    uint8_t prev_y_pos = 0;

    block_t block = {block.width = WIDTH_OF_BLOCK, block.height = HEIGHT_OF_BLOCK};

    while (1)
    {
        int y_position = 0;
        bool stop_block = false;
        while ((y_position < CONFIG_GRAPHICS_PIXELHEIGHT + WIDTH_OF_BLOCK) && (stop_block == false))
        {
            y_position += 1;
            graphics_startUpdate();
            if (setBlockAnimationFrame(decreasing_x_movement, x_start, y_position, &block, CONFIG_GRAPHICS_PIXELHEIGHT) == out_of_bounds)
            {
                ESP_LOGE(TAG, "OUT OF BOUNDS");
            }
            graphics_finishUpdate();
            if (button_pressed == true)
            {
                button_pressed = false;
                stop_block = true;

                graphics_startUpdate();
                updateSetBlock(decreasing_x_movement, x_start, prev_y_pos, &block, CONFIG_GRAPHICS_PIXELHEIGHT);
                graphics_finishUpdate();

                if (prev_y_pos != 0)
                {
                    block.width -= (y_position >= prev_y_pos) ? (y_position - prev_y_pos) : (prev_y_pos - y_position);
                    if (block.width <= 0)
                    {
                        ESP_LOGE(TAG, "YOU LOST!");
                        stop_block = true;
                    }
                }
                ESP_LOGI(TAG, "Block width: %" PRIu8, block.width);

                x_start -= HEIGHT_OF_BLOCK;

                if ((prev_y_pos > y_position) || (prev_y_pos == 0))
                {
                    prev_y_pos = y_position;
                }
            }
            vTaskDelay(20 / portTICK_PERIOD_MS);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        // graphics_clearScreen();
    }
}
