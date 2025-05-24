// display.h
//
//
// developed by: Bruno Castro (github: bdCastro)

#include "freertos/task.h"

#define BUFF_SIZE 128

void HID_init();
void HID_write(char*);