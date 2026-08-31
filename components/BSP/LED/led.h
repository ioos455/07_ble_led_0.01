#ifndef __LED_H
#define __LED_H

#include "driver/gpio.h"

#define LED0_GPIO_PIN GPIO_NUM_43
#define LED1_GPIO_PIN GPIO_NUM_44

#define LED0(x) do{x ? gpio_set_level(LED0_GPIO_PIN, 1) : \
  gpio_set_level(LED0_GPIO_PIN, 0);} while (0);
#define LED1(x) do{x ? gpio_set_level(LED1_GPIO_PIN, 1) : \
   gpio_set_level(LED1_GPIO_PIN, 0);} while (0);

#define LED0_TOGGLE() do{gpio_set_level(LED0_GPIO_PIN, \
  !gpio_get_level(LED0_GPIO_PIN));} while (0);
#define LED1_TOGGLE() do{gpio_set_level(LED1_GPIO_PIN, \
  !gpio_get_level(LED1_GPIO_PIN));} while (0);

void led_init(void);

#endif /* __LED_H */
