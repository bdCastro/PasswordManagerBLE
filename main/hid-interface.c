// hid-interface.c
//
//
// developed by: Bruno Castro (github: bdCastro)

#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tinyusb.h"
#include "class/hid/hid_device.h"
#include "driver/gpio.h"

#include "hid-interface.h"

static const char *TAG = "HID";

/************* TinyUSB descriptors ****************/

#define TUSB_DESC_TOTAL_LEN      (TUD_CONFIG_DESC_LEN + CFG_TUD_HID * TUD_HID_DESC_LEN)

// HID Queue
static QueueHandle_t HIDQueue;

/**
 * @brief HID report descriptor
 *
 * In this example we implement Keyboard + Mouse HID device,
 * so we must define both report descriptors
 */
const uint8_t hid_report_descriptor[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(HID_ITF_PROTOCOL_KEYBOARD)),
    TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(HID_ITF_PROTOCOL_MOUSE))
};

/**
 * @brief String descriptor
 */
const char* hid_string_descriptor[5] = {
    // array of pointer to string descriptors
    (char[]){0x09, 0x04},  // 0: is supported language is English (0x0409)
    "TinyUSB",             // 1: Manufacturer
    "TinyUSB Device",      // 2: Product
    "123456",              // 3: Serials, should use chip ID
    "Example HID interface",  // 4: HID
};

/**
 * @brief Configuration descriptor
 *
 * This is a simple configuration descriptor that defines 1 configuration and 1 HID interface
 */
static const uint8_t hid_configuration_descriptor[] = {
    // Configuration number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, TUSB_DESC_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    // Interface number, string index, boot protocol, report descriptor len, EP In address, size & polling interval
    TUD_HID_DESCRIPTOR(0, 4, false, sizeof(hid_report_descriptor), 0x81, 16, 10),
};

/********* TinyUSB HID callbacks ***************/

// Invoked when received GET HID REPORT DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
// We use only one interface and one HID report descriptor, so we can ignore parameter 'instance'
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) { return hid_report_descriptor; }

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {}

void HID_config() {
    ESP_LOGI(TAG, "USB initialization");
    const tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .string_descriptor = hid_string_descriptor,
        .string_descriptor_count = sizeof(hid_string_descriptor) / sizeof(hid_string_descriptor[0]),
        .external_phy = false,
#if (TUD_OPT_HIGH_SPEED)
        .fs_configuration_descriptor = hid_configuration_descriptor, // HID configuration descriptor for full-speed and high-speed are the same
        .hs_configuration_descriptor = hid_configuration_descriptor,
        .qualifier_descriptor = NULL,
#else
        .configuration_descriptor = hid_configuration_descriptor,
#endif // TUD_OPT_HIGH_SPEED
    };

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
    ESP_LOGI(TAG, "USB initialization DONE");
}

// adds a word to be written on the queue
void HID_write(char* word) {
    xQueueSend(HIDQueue, &word, 100/portTICK_PERIOD_MS);
}

void TaskHID(void* args) {
    HID_config();

    // Queue initialization
    HIDQueue = xQueueCreate(1, sizeof(char*));
    char* word = (char*) malloc(BUFF_SIZE);

    while (true) {
        xQueueReceive(HIDQueue, &word, portMAX_DELAY);
        int i = 0;
        uint8_t chr = (uint8_t) word[i];

        uint8_t const conv_table[128][2] =  { HID_ASCII_TO_KEYCODE };
        
        if (tud_mounted()) {
            while(chr != 0) {
                uint8_t keycode[6] = { 0 };
                uint8_t modifier = 0;
                if ( conv_table[chr][0] ) modifier = KEYBOARD_MODIFIER_LEFTSHIFT;

                keycode[0] = conv_table[chr][1];

                tud_hid_keyboard_report(HID_ITF_PROTOCOL_KEYBOARD, modifier, keycode);
                vTaskDelay(pdMS_TO_TICKS(15));
                tud_hid_keyboard_report(HID_ITF_PROTOCOL_KEYBOARD, 0, NULL);
                vTaskDelay(pdMS_TO_TICKS(15));

                chr = (uint8_t) word[++i];
            }
        }
    }
}

void HID_init(void) {
    xTaskCreate(&TaskHID, "HID", 4096, NULL, 2, NULL);
}