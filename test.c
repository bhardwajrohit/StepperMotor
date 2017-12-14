#include <stdint.h>
#include <stdio.h>
#include "stm32f3xx_hal.h"
#include "common.h"

void gpioinit()
{
__GPIOE_CLK_ENABLE();
GPIO_InitTypeDef  GPIO_InitStruct;
GPIO_InitStruct.Pin = (GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10);
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
GPIO_InitStruct.Alternate = 0;
HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
return;
}

ADD_CMD("gpio",gpioinit,"             initalize GPIO Pin")

void gpio(int mode)
{
  if(mode != CMD_INTERACTIVE)
  {
    return;
  }

uint32_t pin;
uint32_t state;

fetch_uint32_arg(&pin);
fetch_uint32_targ(&state);

if((state != 0) && (state != 1))
{
printf("enter 0 or 1\n");
}
else
{
switch(pin)
{
case 0:
HAL_GPIO_WritePin(GPIOE,GPIO_PIN_0,state);
break;

case 1:
HAL_GPIO_WritePin(GPIOE,GPIO_PIN_1,state);
break;

case 2:
HAL_GPIO_WritePin(GPIOE,GPIO_PIN_2,state);
break;

case 3:
HAL_GPIO_WritePin(GPIOE,GPIO_PIN_3,state);
break;

case 4:
HAL_GPIO_WritePin(GPIOE,GPIO_PIN_4,state);
break;

case 5:
HAL_GPIO_WritePin(GPIOE,GPIO_PIN_5,state);
break;

case 6:
HAL_GPIO_WritePin(GPIOE,GPIO_PIN_6,state);
break;

case 7:
HAL_GPIO_WritePin(GPIOE,GPIO_PIN_7,state);
break;

case 8:
HAL_GPIO_WritePin(GPIOE,GPIO_PIN_8,state);
break;

case 9:
HAL_GPIO_WritePin(GPIOE,GPIO_PIN_9,state);
break;

case 10:
HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,state);
break;

case 11:
HAL_GPIO_WritePin(GPIOE,GPIO_PIN_11,state);
break;

case 12:
HAL_GPIO_WritePin(GPIOE,GPIO_PIN_12,state);
break;

case 13:
HAL_GPIO_WritePin(GPIOE,GPIO_PIN_13,state);
break;

case 14:
HAL_GPIO_WritePin(GPIOE,GPIO_PIN_14,state);
break;

case 15:
HAL_GPIO_WritePin(GPIOE,GPIO_PIN_15,state);
break;

default:
printf("Enter between 0-15\n");
break;
}
}
}

ADD_CMD("gpioinit",gpioinit,"             initalize GPIO Pin");
ADD_CMD("gpio",gpio,"             Glow the led")



