// hid-interface.h
//
//
// developed by: Bruno Castro (github: bdCastro)

#include "freertos/task.h"

#define BUFF_SIZE 128

typedef struct HIDRequest {
    char* word;
    uint8_t length;
} HIDRequest_t;

void HID_init();
void HID_write(char*, uint16_t length);