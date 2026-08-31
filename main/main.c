#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "myble.h"
#include "nvs_flash.h"

void app_main(void)
{
    nvs_flash_init(); /* 初始化NVS */
    led_init(); /* 初始化LED */
    ble_init(); /* 初始化蓝牙 */

        while(1)
        {
            vTaskDelay(pdMS_TO_TICKS(20));
        }
}
