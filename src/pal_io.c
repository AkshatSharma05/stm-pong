#include "regs.h"

uint8_t button_pressed( ){
    return !(GPIOB_IDR & (1U << 5));
}