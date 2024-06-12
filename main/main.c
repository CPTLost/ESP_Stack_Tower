#include <stdio.h>
#include "graphics.h"
#include "return_t.h"
#include "button.h"
#include "esp_log.h"
#include "i2c4display.h"
#include "stack_tower.h"

#define LEFT_BTN_GPIO 9

// Maybe put into SDKconfig
#define HEIGHT_OF_BLOCK 12
#define WIDTH_OF_BLOCK 30
#define X_MOVEMENT decreasing_x_movement

static const char *TAG = "STACK TOWER";

// globals
uint32_t game_count = 0;
bool clear_screen = false;
bool game_lost = false;
bool button_pressed = false;

// This function is called when interrupt is triggered
return_t button_callback()
{
    ESP_EARLY_LOGI(TAG, "Button Callback"); // DEBUG
    // reset game with button press when lost
    if (game_lost == true)
    {
        game_lost = false;
        clear_screen = true;
    }
    else
    {
        button_pressed = true;
    }
    return success;
}

void app_main(void)
{
    // init i2c for and display
    ESP_ERROR_CHECK(initI2C(I2C_INTERFACE));
    ESP_ERROR_CHECK(graphics_init(I2C_INTERFACE, CONFIG_GRAPHICS_PIXELWIDTH, CONFIG_GRAPHICS_PIXELHEIGHT, 0, false, false));
    button_configure(LEFT_BTN_GPIO);

    uint8_t x_start = CONFIG_GRAPHICS_PIXELWIDTH; // Due to display orientation it needs to start from max x pos and count down
    uint8_t prev_block_y_pos = 0;

    block_t block = {block.width = WIDTH_OF_BLOCK, block.height = HEIGHT_OF_BLOCK};

    // Main loop
    while (1)
    {
        // Starting point after every block animation
        int y_position = 0;
        bool stop_anim = false;

        if (game_lost == true && game_count != 0)
        {
            x_start = CONFIG_GRAPHICS_PIXELWIDTH;
            prev_block_y_pos = 0;
            block.width = WIDTH_OF_BLOCK;

            ESP_LOGW(TAG, "Game count: %" PRIu32, game_count);
            game_count = 0;
        }

        // Game loop: Block Animation
        while ((y_position < CONFIG_GRAPHICS_PIXELHEIGHT + WIDTH_OF_BLOCK) && (stop_anim == false) && (game_lost == false))
        {
            // After game is lost the screen needs to be clean before starting with new game
            if (clear_screen == true)
            {
                clear_screen = false;
                graphics_startUpdate();
                graphics_clearScreen();
                graphics_finishUpdate();
            }

            // In every loop the y_position is incremented by 1 -> the block position is updated (shifted to the left by 1)
            y_position += 1;
            graphics_startUpdate();
            // When the block gets out of bounds you lose, except at the frist one
            if (setBlockAnimationFrame(X_MOVEMENT, x_start, y_position, &block, CONFIG_GRAPHICS_PIXELHEIGHT) == out_of_bounds)
            {
                if (game_count != 0)
                {
                    ESP_LOGE(TAG, "OUT OF BOUNDS -> YOU LOST!");
                    game_lost = true;
                }
            }
            graphics_finishUpdate();

            // When the button is pressed the overhanging bit of the block is removed and the block width is updated
            if (button_pressed == true)
            {
                button_pressed = false;
                stop_anim = true;

                graphics_startUpdate();
                adaptBlockFrame(X_MOVEMENT, x_start, prev_block_y_pos, &block, CONFIG_GRAPHICS_PIXELHEIGHT);
                graphics_finishUpdate();

                // Here the new block width is calculated, it must skip the first block
                if (game_count != 0)
                {
                    block.width -= (y_position >= prev_block_y_pos) ? (y_position - prev_block_y_pos) : (prev_block_y_pos - y_position);
                    if (block.width <= 0)
                    {
                        ESP_LOGE(TAG, "YOU LOST!");
                        game_lost = true;
                    }
                }

                // This shifts the animation to the next line (determined by block heigth)
                (X_MOVEMENT == decreasing_x_movement) ? (x_start -= HEIGHT_OF_BLOCK) : (x_start += HEIGHT_OF_BLOCK);

                // Gets correct value for prev y position
                if ((prev_block_y_pos > y_position) || (game_count == 0))
                {
                    prev_block_y_pos = y_position;
                }

                if (game_lost == false)
                {
                    game_count += 1;
                }
            }
            // speed of Animation
            vTaskDelay(20 / portTICK_PERIOD_MS);
        }
        // When game not lost increment game count
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
