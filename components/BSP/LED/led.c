#include "led.h"

void led_init(void)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE; /* 禁用中断 */
    io_conf.mode = GPIO_MODE_INPUT_OUTPUT; /* 设置为输出模式 */
    io_conf.pin_bit_mask = (1ULL << LED0_GPIO_PIN) | (1ULL << LED1_GPIO_PIN); /* 配置LED引脚 */
    io_conf.pull_down_en = 0; /* 禁用下拉 */
    io_conf.pull_up_en = 0; /* 禁用上拉 */

    gpio_config(&io_conf); /* 应用配置 */

    LED1(1); /* 初始化LED1为开启状态 */
    LED0(1); /* 初始化LED0为开启状态 */
}