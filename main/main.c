/* ESP_STACK_TOWER was created by Noah Plank as a Projekt for the Embedded Systems course of the FHV*/

#include <stdio.h>
#include <assert.h>
#include "graphics.h"
#include "return_t.h"
#include "button.h"
#include "esp_log.h"
#include "i2c4display.h"
#include "stack_tower.h"
#include "esp_rom_sys.h"

// Maybe put into SDKconfig
#define LEFT_BTN_GPIO 9
#define HEIGHT_OF_BLOCK 12
#define WIDTH_OF_BLOCK 30
#define X_MOVEMENT decreasing_x_movement // decreasing means that you start from max x value and then move down. Dependend on display orientation
#define GAMEMODE INCREASING_DIFFICULTY_HARD

#if (GAMEMODE == INCREASING_DIFFICULTY_HARD)

#define GAME_STARTING_LEVEL_DELAY 100  // in ms
#define GAME_STARTING_ANIM_DELAY 10000 // in µs
#define GAME_MIN_ANIM_DELAY 5000
#endif

#if (GAMEMODE == INCREASING_DIFFICULTY_EASY)

#define GAME_STARTING_LEVEL_DELAY 100  // in ms
#define GAME_STARTING_ANIM_DELAY 12000 // in µs
#define GAME_MIN_ANIM_DELAY 6000
#endif

#if (GAMEMODE == FIXED_DIFICULTY_HARD)

#define GAME_STARTING_LEVEL_DELAY 100 // in ms
#define GAME_STARTING_ANIM_DELAY 5000 // in µs
#define GAME_MIN_ANIM_DELAY 5000
#endif

#if (GAMEMODE == FIXED_DIFFICULTY_EASY)

#define GAME_STARTING_LEVEL_DELAY 100  // in ms
#define GAME_STARTING_ANIM_DELAY 10000 // in µs
#define GAME_MIN_ANIM_DELAY 10000
#endif

static const char *TAG = "STACK TOWER";

// globals
static uint32_t game_count = 0;
static bool game_start = false;
static bool game_lost = false;
static bool button_pressed = false;
static bool clear_screen = false;

// This function is called when interrupt is triggered = Button is pressed
return_t button_callback()
{
    if (false == game_start)
    {
        game_start = true;
        clear_screen = true;
        return success;
    }

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

    // Due to display orientation it needs to start from max x position and count down
    uint8_t x_start = (X_MOVEMENT == decreasing_x_movement) ? CONFIG_GRAPHICS_PIXELWIDTH : 0;
    uint8_t prev_block_y_pos = 0;

    block_t block = {block.width = WIDTH_OF_BLOCK, block.height = HEIGHT_OF_BLOCK};

    uint32_t game_level_delay = GAME_STARTING_LEVEL_DELAY;
    uint32_t game_anim_delay = GAME_STARTING_ANIM_DELAY;

    // Starting Screen
    graphics_startUpdate();
    graphics_println("    STACK TOWER");
    graphics_println("       THE GAME");
    graphics_finishUpdate();

    while (!game_start)
    {
        graphics_startUpdate();
        graphics_setCursor(0, 40);
        graphics_writeString("  Press Btn to Start");
        graphics_finishUpdate();
        vTaskDelay(800 / portTICK_PERIOD_MS);
        graphics_startUpdate();
        graphics_clearRegion(0, 40, 128, 64);
        graphics_finishUpdate();
        vTaskDelay(250 / portTICK_PERIOD_MS);
    };

    // Main level loop
    while (1)
    {
        int y_position = 0;
        bool stop_anim = false;

        // This block resets the game after its lost
        if (game_lost == true && game_count != 0)
        {
            x_start = (X_MOVEMENT == decreasing_x_movement) ? CONFIG_GRAPHICS_PIXELWIDTH : 0;
            prev_block_y_pos = 0;
            block.width = WIDTH_OF_BLOCK;
            game_anim_delay = GAME_STARTING_ANIM_DELAY;

            char game_score[16];
            sprintf(game_score, "Game Score: %ld", game_count);
            graphics_startUpdate();
            graphics_writeString(game_score);
            graphics_finishUpdate();

            ESP_LOGW(TAG, "Game count: %" PRIu32, game_count);
            game_count = 0;
        }

        // Block animation: one while loop iteration == 1 level animation
        while ((y_position < CONFIG_GRAPHICS_PIXELHEIGHT + WIDTH_OF_BLOCK) && (stop_anim == false) && (game_lost == false))
        {
            /*  - After a game is lost the screen needs to be cleared before starting the new game
                - The cleaer_screen is set to true inside the button_callback when the game was lost and also when level 10 is reached */
            if (clear_screen == true)
            {
                clear_screen = false;
                graphics_startUpdate();
                graphics_clearScreen();
                graphics_finishUpdate();
            }

            // In every loop the y_position is incremented by 1 -> the block position is updated (= shifted to the left by 1)
            y_position += 1;
            graphics_startUpdate();
            // When the block gets out of bounds you lose, except in the first round
            if (setBlockAnimationFrame(X_MOVEMENT, x_start, y_position, &block, CONFIG_GRAPHICS_PIXELHEIGHT) == out_of_bounds)
            {
                if (game_count != 0)
                {
                    ESP_LOGE(TAG, "OUT OF BOUNDS -> YOU LOST!");
                    game_lost = true;
                }
            }
            graphics_finishUpdate();

            // When the button is pressed the overhanging part of the block is removed and the block width is updated
            if (button_pressed == true)
            {
                button_pressed = false;
                stop_anim = true;

                graphics_startUpdate();
                adaptBlockFrame(X_MOVEMENT, x_start, prev_block_y_pos, &block, CONFIG_GRAPHICS_PIXELHEIGHT);
                graphics_finishUpdate();

                /*  Here the block width of the next block is calculated and it also
                    remembers where the current block is placed by using the blocks left end (= y_position) as a reference */
                if (0 == (game_count % (CONFIG_GRAPHICS_PIXELWIDTH / HEIGHT_OF_BLOCK)))
                {
                    block.width -= (y_position > CONFIG_GRAPHICS_PIXELHEIGHT) ? (y_position - CONFIG_GRAPHICS_PIXELHEIGHT) : (0);
                    prev_block_y_pos = (y_position > CONFIG_GRAPHICS_PIXELHEIGHT) ? (CONFIG_GRAPHICS_PIXELHEIGHT) : (y_position);
                }
                else
                {
                    block.width -= (y_position >= prev_block_y_pos) ? (y_position - prev_block_y_pos) : (prev_block_y_pos - y_position);
                    if ((prev_block_y_pos > y_position))
                    {
                        prev_block_y_pos = y_position;
                    }
                }

                // Checks if u missed your target completely
                if (block.width <= 0)
                {
                    ESP_LOGE(TAG, "YOU LOST!");
                    game_lost = true;
                }

                // This shifts the animation to the next level (determined by block heigth), (there are 10 levels per screen with a block height of 12)
                if (X_MOVEMENT == decreasing_x_movement)
                {
                    x_start -= HEIGHT_OF_BLOCK;
                }
                else
                {
                    x_start += HEIGHT_OF_BLOCK;
                }

                // When the game isn't lost increment game count
                if (game_lost == false)
                {
                    game_count += 1;

                    if (0 == (game_count % (CONFIG_GRAPHICS_PIXELWIDTH / HEIGHT_OF_BLOCK)))
                    {
                        clear_screen = true;
                        prev_block_y_pos = 0;
                        x_start = (X_MOVEMENT == decreasing_x_movement) ? CONFIG_GRAPHICS_PIXELWIDTH : 0;
                    }
                }
                if (GAMEMODE == INCREASING_DIFFICULTY_EASY || GAMEMODE == INCREASING_DIFFICULTY_HARD)
                {
                    game_anim_delay = (game_anim_delay <= GAME_MIN_ANIM_DELAY) ? (GAME_MIN_ANIM_DELAY) : (game_anim_delay - 250);
                    ESP_LOGE(TAG, "game anim delay: %ld", game_anim_delay);
                }
            }
            // speed of Animation
            esp_rom_delay_us(game_anim_delay);
        }
        // time delay between blocks
        vTaskDelay(game_level_delay / portTICK_PERIOD_MS);
    }
}
