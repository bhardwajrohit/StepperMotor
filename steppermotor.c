/*
   FILE          : steppermotor.c
   PROJECT       : Stepper Motor/Stm32f3 Discovery Board/Linux
   PROGRAMMER    : Rohit Bhardwaj
   DESCRIPTION   : This program configures GIPO port D,A,F to send binary data to control stepper motor in clockwise and 
                   anti-clockwise directions with two modes each:Half-step and Full-step modes respectively.
		           Timer 15 is configured to generate the required delay and also an Interrupt flag(UIF) is monitored to check
		           the update event.
	
	The program make use of HAL(Hardware Abstraction Layer) which is C code that implements basic drivers for all the peripherals 
	in the STM32 Family. It makes the code more portable over STM32 Family.
*/

#include <stdint.h>
#include <stdio.h>
#include "stm32f3xx_hal.h"
#include "common.h"

// FUNCTION      : gpioinit1()
// DESCRIPTION   : The function enables the GPIO Pins of port D
// PARAMETERS    : The function accepts an int value
// RETURNS       : returns nothing
void gpioinit1(int mode)
{
/* Turn on clocks to I/O */

__GPIOD_CLK_ENABLE();

/* Configure GPIO pins */
GPIO_InitTypeDef  GPIO_InitStruct;

GPIO_InitStruct.Pin = (GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | 
                       GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
GPIO_InitStruct.Alternate = 0;
HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
return;
}

ADD_CMD("gpioinit1",gpioinit1,"              Initialize the GPIO Pins");


/*Global Variable*/
static DAC_HandleTypeDef hdac;

// FUNCTION      : dacinit()
// DESCRIPTION   : The function enables the GPIO Pin 4 of port A and also initializes the DAC
// PARAMETERS    : The function accepts an int value
// RETURNS       : returns nothing
void dacinit(int mode)
{
/* Turn on clocks to I/O */
__GPIOA_CLK_ENABLE();
__DAC1_CLK_ENABLE();


/* Configure GPIO pins */
GPIO_InitTypeDef  GPIO_InitStruct;

GPIO_InitStruct.Pin = (GPIO_PIN_4 );
GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
GPIO_InitStruct.Alternate = 0;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); 

/*Local Variable*/
DAC_ChannelConfTypeDef DacConfig;

/* Initialize DAC */
hdac.Instance=DAC1;

uint32_t rc;

rc = HAL_DAC_Init(&hdac);
if(rc != HAL_OK)
 {
  printf("Unable to initialize ""DAC, rc=%d\n",(unsigned)rc);
  return;
 }
DacConfig.DAC_Trigger = DAC_TRIGGER_NONE;
DacConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
rc = HAL_DAC_ConfigChannel(&hdac,&DacConfig,DAC_CHANNEL_1);
if(rc != HAL_OK) 
 {
  printf("Unable to configure DAC ""channel 1, rc=%d\n",(unsigned)rc);
  return;
 }
/* Enable the output */
 __HAL_DAC_ENABLE(&hdac,DAC_CHANNEL_1);
}

ADD_CMD("dacinit",dacinit,"              Initialize the DAC");

// FUNCTION      : writeDAC()
// DESCRIPTION   : The function sets the DAC to the value requested
// PARAMETERS    : The function accepts an int value
// RETURNS       : returns nothing
void writeDAC (int mode)
{
  if(mode != CMD_INTERACTIVE)
  {
    return;
  }
HAL_StatusTypeDef rc;

uint32_t value;

fetch_uint32_arg(&value);     // Fetch the DAC values from the cmd

/* Set initial values */
rc = HAL_DAC_SetValue(&hdac,DAC_CHANNEL_1,
                 DAC_ALIGN_12B_R,value);
if(rc != HAL_OK)
 {
  printf("Unable to initial value on DAC ""channel 1, rc=%d\n",rc);
  return;
 }
  printf("DAC Value: %u\n",(unsigned)value);
  return ;
}
ADD_CMD("writeDAC" , writeDAC, "        Sets the DAC to the value requested");


/*Global Handle Structure*/
TIM_HandleTypeDef tim15;

// FUNCTION      : timerinit()
// DESCRIPTION   : The function initializes and starts the timer 15 on channel 2
// PARAMETERS    : The function accepts an int value
// RETURNS       : returns nothing
void timerinit(int mode)
{
/* Turn on clocks to I/O */
__GPIOF_CLK_ENABLE();

/* Configure GPIO pins */
GPIO_InitTypeDef  GPIO_InitStruct;

GPIO_InitStruct.Pin = (GPIO_PIN_10);
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
GPIO_InitStruct.Alternate = 3;
HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

uint32_t rc;

/* Initialize PWM */
 __TIM15_CLK_ENABLE();
tim15.Instance = TIM15; 
tim15.Init.Prescaler = HAL_RCC_GetPCLK2Freq()*2/1000000;//144
tim15.Init.CounterMode   = TIM_COUNTERMODE_UP; 
tim15.Init.Period        = 1000; //ARR
tim15.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1; 
tim15.Init.RepetitionCounter = 0; 
HAL_TIM_Base_Init(&tim15);

/*Configure output:*/
TIM_OC_InitTypeDef sConfig; 
sConfig.OCMode       = TIM_OCMODE_PWM1; 
sConfig.Pulse        = 500; 
sConfig.OCPolarity   = TIM_OCPOLARITY_HIGH; 
sConfig.OCNPolarity  = TIM_OCNPOLARITY_LOW; 
sConfig.OCFastMode   = TIM_OCFAST_DISABLE; 
sConfig.OCIdleState  = TIM_OCIDLESTATE_RESET; 
sConfig.OCNIdleState =TIM_OCNIDLESTATE_RESET; 
HAL_TIM_PWM_ConfigChannel(&tim15,&sConfig,TIM_CHANNEL_2);

/* Start the PWM output: */
HAL_TIM_PWM_Start(&tim15,TIM_CHANNEL_2); 

/* Stop the PWM output: */
//HAL_TIM_PWM_Stop(&tim15,TIM_CHANNEL_2);

/*Initalize the Timer*/
rc = HAL_TIM_Base_Init(&tim15);
if(rc != HAL_OK) 
 {
  printf("Unable to initalize Timer, rc=%d\n",(unsigned)rc);
  return;
 }

/*Start the timer*/
 rc = HAL_TIM_Base_Start(&tim15);
 if(rc != HAL_OK) 
 {
  printf("Unable to start the timer, rc=%d\n",(unsigned)rc);
  return;
 }
}

ADD_CMD("timerinit",timerinit,"              Initialize the Timer");


// FUNCTION      : stepperinit()
// DESCRIPTION   : The function initializes Stepper Motor I/O Pins
// PARAMETERS    : The function accepts an int value
// RETURNS       : returns nothing
void stepperinit(int mode)
{

HAL_GPIO_WritePin(GPIOD,GPIO_PIN_7,1);     //Reset

HAL_GPIO_WritePin(GPIOD,GPIO_PIN_8,1);	   //ps
HAL_GPIO_WritePin(GPIOD,GPIO_PIN_9,1);     //MD1

HAL_GPIO_WritePin(GPIOD,GPIO_PIN_10,1);    //Md2

HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,0);    //AT1

HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,0);    //AT2

HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,1);    //FR

}

ADD_CMD("stepperinit",stepperinit,"              Initialize the Stepper Motor I/O Pins");

// FUNCTION      : stepperEnable()
// DESCRIPTION   : The function enables or disables the stepper motor controller output
// PARAMETERS    : The function accepts an int value
// RETURNS       : returns nothing
void stepperEnable(int mode)
{
  if(mode != CMD_INTERACTIVE)
  {
    return;
  }

 uint32_t state;
 fetch_uint32_arg(&state);
 HAL_GPIO_WritePin(GPIOD,GPIO_PIN_11,state);
  printf("%d",(unsigned)state);   //Pin11  Output
}

ADD_CMD("stepperEnable",stepperEnable,"              Enable/Disable the Stepper Motor Controller Outputs");

// FUNCTION      : stepspeed()
// DESCRIPTION   : The function activates the stepper motor to operate
//                 in clockwise/Anticlockwise direction and also in 2 different modes:
//		           Half-step and Full-Step
// PARAMETERS    : The function accepts an int value
// RETURNS       : returns nothing
/*Adding Delay*/
void stepspeed(int mode)
{
  if(mode != CMD_INTERACTIVE)
  {
    return;
  }

 uint32_t dir;
 uint32_t delayVal;

 fetch_uint32_arg(&dir);
 fetch_uint32_arg(&delayVal);

 if(dir > 0)
 {
  if(delayVal > 0 && delayVal < 65001)
  {
   HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,0);    //FR clockwise
   /* Start the PWM output: */
   HAL_TIM_PWM_Start(&tim15,TIM_CHANNEL_2); 
   }
 }

 else if(dir < 0)
 {
  if(delayVal > 0 && delayVal < 65001)
  {
   HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,1);    //FR counterclockwise
   /* Start the PWM output: */
   HAL_TIM_PWM_Start(&tim15,TIM_CHANNEL_2); 
   }
 }

 else if (dir == 0)
 {
  if(delayVal > 0 && delayVal < 65001)
  {
   /* Stops the PWM output: */ 
   HAL_TIM_PWM_Stop(&tim15,TIM_CHANNEL_2);
  }
 }
}
ADD_CMD("stepspeed",stepspeed,"            Makes stepper step at a speed set by the delay value, continuously step<steps><delay>");



