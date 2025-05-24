// display.h
//
//
// developed by: Bruno Castro (github: bdCastro)

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <string.h>

#include "hid-interface.h"

#define APP_BUTTON (GPIO_NUM_0) // Use BOOT signal by default

void app_main() {
    HID_init();

    // configures the BOOT button to send the <word> string
    const gpio_config_t boot_button_config = {
        .pin_bit_mask = BIT64(APP_BUTTON),
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_up_en = true,
        .pull_down_en = false,
    };

    ESP_ERROR_CHECK(gpio_config(&boot_button_config));

    char* word = "bruno@teste.com.br\ttesteTESTEbanana\n\0";
    while(true) {
        static bool send_hid_data = true;

        if (send_hid_data) {
            HID_write(word);
        }

        send_hid_data = !gpio_get_level(APP_BUTTON);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}