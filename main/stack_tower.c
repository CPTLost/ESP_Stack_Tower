/* This file was created by Noah Plank for the Projekt ESP_STACK_TOWER */

#include "stack_tower.h"

#include "graphics.h"

// This function returns out_of_bounds (= 1) when out of bounds or success (= 0) when not
return_t setBlockAnimationFrame(animation_movement ani_movement, uint8_t x_start, uint8_t y_position, block_t *block, uint8_t width_of_screen)
{

    if (y_position >= block->width + width_of_screen)
    {
        return out_of_bounds;
    }
    for (int i = 0; i < block->height; i += 1)
    {
        graphics_setPixel((ani_movement == increasing_x_movement) ? (x_start + i) : (x_start - i), (y_position >= width_of_screen) ? (width_of_screen) : (y_position));
        graphics_clearPixel((ani_movement == increasing_x_movement) ? (x_start + i) : (x_start - i), (y_position - block->width >= 0) ? (y_position - block->width) : (0));
    }

    return success;
}

return_t adaptBlockFrame(animation_movement ani_movement, uint8_t x_start, uint8_t prev_y_position, block_t *block, uint8_t width_of_screen)
{
    // for the first block
    if (0 == prev_y_position)
    {
        return success;
    }
    for (int i = 0; i < block->height; i += 1)
    {
        for (int j = 0; j < width_of_screen; j += 1)
        {
            if (j <= (prev_y_position - block->width))
            {
                graphics_clearPixel((ani_movement == increasing_x_movement) ? (x_start + i) : (x_start - i), j);
            }
            if (j > prev_y_position)
            {
                graphics_clearPixel((ani_movement == increasing_x_movement) ? (x_start + i) : (x_start - i), j);
            }
        }
    }
    return success;
}