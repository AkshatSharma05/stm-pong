#include <stdint.h>
#include "regs.h"
#include "system.h"
#include "handlers.h"
#include "pal_io.h"

uint32_t last = 0;

int main(void)
{
    sysInit();

    //LED Blink
    for (;;) {
        // if(sysTicks - last > 500){
        //     last = sysTicks;
        //     GPIOC_ODR ^= (1U << 13);
        // }

        if(!button_pressed( )){
            GPIOC_ODR |= (1U << 13); //off
        } else {
            GPIOC_ODR &= ~(1U << 13); //on
        }
    }

    return 0;
}
