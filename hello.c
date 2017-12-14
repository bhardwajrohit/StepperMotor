#include <stdint.h>
#include <stdio.h>
#include "stm32f3xx_hal.h"
#include "common.h"

void CmdHello(int mode)
{
  if(mode != CMD_INTERACTIVE) {
    return;
  }

  printf("Hello World!\n");
}
ADD_CMD("hello",CmdHello,"             Print Hello")
