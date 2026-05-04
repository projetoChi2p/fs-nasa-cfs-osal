#ifndef TICKS_H
#define TICKS_H

#include <stdint.h>

#define LED_PIN 25

#ifdef __cplusplus
extern "C" {
#endif

void setup_ticks();

void delayus(uint32_t us);

void delayms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* TICKS_H */