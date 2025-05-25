// display.c
//
//
// developed by: Bruno Castro (github: bdCastro)

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "display.h"
#include "demo/clock.h"

void TaskDisplay(void* args) {
    clock_demo(); // calls the clock demo
}

void display_init() {
    xTaskCreate(&TaskDisplay, "display", 4096, NULL, 2, NULL);
}