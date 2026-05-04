#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*gpio_irq_callback_t)(uint32_t pin, uint32_t event);

void gpio_set_function(uint32_t pin, uint32_t func);

void gpio_set_input_enabled(uint32_t pin, uint32_t enable);

void gpio_set_slew(uint32_t pin, uint32_t fast);

void gpio_set_schmitt(uint32_t pin, uint32_t enable);

void gpio_set_pullup(uint32_t pin);

void gpio_disable_pulls(uint32_t pin);

void gpio_set_drive(uint32_t pin, uint32_t strength);

void pin_output(uint32_t pin);

void pin_input_pu(uint32_t pin);

void pin_high(uint32_t pin);

void pin_low(uint32_t pin);

void pin_toggle(uint32_t pin);

uint32_t pin_get(uint32_t pin);

void gpio_enable_common_irq(void);

void gpio_disable_common_irq(void);

uint32_t gpio_enable_interrupt(uint32_t pin, gpio_irq_callback_t callback, uint32_t events);

void gpio_disable_interrupt(uint32_t pin);

#ifdef __cplusplus
}
#endif

#endif /* GPIO_H */