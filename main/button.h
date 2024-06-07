#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>
#include <inttypes.h>

#include "return_t.h"

return_t configure_button(uint8_t GPIO_BUTTON_PIN);

#endif