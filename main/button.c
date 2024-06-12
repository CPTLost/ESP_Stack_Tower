#include "button.h"

#include <driver/gpio.h>

static bool isr_installed = false;

void button_callback(void);

void button_isrHandler()
{
    button_callback();
}

return_t button_configure(uint8_t GPIO_BUTTON_PIN)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = 1 << GPIO_BUTTON_PIN,
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };
    gpio_config(&io_conf);

    if (isr_installed == false)
    {
        gpio_install_isr_service(0);
        isr_installed = true;
    }
    gpio_isr_handler_add(GPIO_BUTTON_PIN, button_isrHandler, NULL);

    return success;
}

__attribute__((weak)) void button_callback(void) {}
