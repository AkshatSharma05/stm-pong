#include <stdint.h>
#include "regs.h"
#include "system.h"
#include "handlers.h"
#include "pal_io.h"
#include "sh1106.h"

uint32_t last = 0;

int main(void)
{
    sysInit();

    return 0;
}
