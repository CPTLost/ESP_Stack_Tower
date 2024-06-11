#ifndef STACK_TOWER
#define STACK_TOWER

#include "return_t.h"
#include "inttypes.h"

typedef struct
{
    int8_t width;
    uint8_t height;
} block_t;

return_t setBlockAnimationFrame(animation_movement ani_movement, uint8_t x_start, uint8_t y_position,
                                block_t *block, uint8_t width_of_screen);

return_t updateSetBlock(animation_movement ani_movement, uint8_t x_start, uint8_t prev_y_position,
                        block_t *block, uint8_t width_of_screen);

#endif