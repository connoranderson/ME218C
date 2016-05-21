#ifndef STEERING_H
#define STEERING_H

// the common headers for C99 types
#include <stdint.h>
#include <stdbool.h>

void Steering_HWInit(void);
void Steering_Reset(uint8_t channel);
void Steering_Write(uint16_t NewPos, uint8_t channel);

#endif //STEERING_H
