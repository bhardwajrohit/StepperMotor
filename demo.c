 //Example: To display character 'P' is 0101 0000 = 0x0050 on this LCD Character RAM.

 

//Below is the code written to display character 'www.eeherald.com' on JHD 162A.


#include "main.h"
#include "stm32f0xx_conf.h"
uint32_t TickValue=0;

#define RS GPIO_Pin_13 // RS is named as Port 13
#define RW GPIO_Pin_14 // RW is named as Port 14
#define EN GPIO_Pin_15 // EN is named as Port 15

//------------------------------------------------------------------------------
// Function Name : delay_ms
// Description : delay for some time in ms unit(accurate)
// Input : n_ms is how many ms of time to delay
//------------------------------------------------------------------------------
void TimingDelay_Decrement(void)
{
TickValue--;
}

void delay_ms(uint32_t n_ms)
{
SysTick_Config(8000*PLL_MUL_X - 30);
TickValue = n_ms;
while(TickValue == n_ms)
;
SysTick_Config(8000*PLL_MUL_X);
while(TickValue != 0)
;
}
//------------------------------------------------------------------------------
// Function Name : Init GPIO
// Description : pins ,port clock & mode initialization.
//------------------------------------------------------------------------------
void initgpio()
{
GPIO_InitTypeDef GPIO_InitStructure;
RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOC, ENABLE);

GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15 ;
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
GPIO_Init(GPIOC, &GPIO_InitStructure);

GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
GPIO_Init(GPIOA, &GPIO_InitStructure);


}
//------------------------------------------------------------------------------
// Function Name : s_init
// Description : Send Instruction Function (RS=0 & RW=0)
//------------------------------------------------------------------------------

void s_init()
{
GPIOC->BRR=RS;
GPIOC->BRR=RW;
}
//------------------------------------------------------------------------------
// Function Name : s_data
// Description : Send Data Select routine(RS=1 & RW=0)
//------------------------------------------------------------------------------

void s_data()
{
GPIOC->BSRR=RS;
GPIOC->BRR=RW;
}
//------------------------------------------------------------------------------
// Function Name : s_latch
// Description : Latch Data/Instruction on LCD Databus.
//------------------------------------------------------------------------------

void s_latch()
{
GPIOC->BSRR=EN;
delay_ms(10);
GPIOC->BRR=EN;
delay_ms(10);
}

/*******************************************************************************
* Function Name : main
* Description : Main program.
*******************************************************************************/
int main(void) //Main function
{

initgpio();

int k=0;
char a[]="WWW.EEHERALD.COM";
char b[]="EMBEDDED SYSTEMS";

GPIOC->BRR=RS; //Initialize RS=0 for selecting instruction Send
GPIOC->BRR=RW; // Select RW=0 to write Instruction/data on LCD
GPIOC->BSRR=EN; // EN=1 for unlatch. (used at initial condition)

delay_ms(10);

s_init(); //Call Instruction Select routine

/*GPIOC->BRR=RS;
GPIOC->BRR=RW;*/

GPIOA->ODR=0x0001; // Clear Display, Cursor to Home

s_latch(); //Latch the above instruction
/*GPIOC->BSRR=EN;
delay_ms(10);
GPIOC->BRR=EN;
delay_ms(10);*/

GPIOA->ODR=0x0038; // Display Function (2 rows for 8-bit data; small)
s_latch(); //Latch this above instruction 4 times
s_latch();
s_latch();
s_latch();
GPIOA->ODR=0x000E; // Display and Cursor on, Cursor Blink off
s_latch(); //Latch the above instruction
GPIOA->ODR=0x0010; // Cursor shift left
s_latch(); //Latch the above instruction
GPIOA->ODR=0x0006; // Cursor Increment, Shift off
s_data(); //Change the input type to Data.(before it was instruction input)
s_latch(); //Latch the above instruction

for(k=0;a[k];k++)
{
GPIOA->ODR=a[k]; //It will send a[0]='P' as = '0x0050' on Port A.
s_latch(); //Latch the above instruction only once. Or it will clone each character twice if you latch twice.
}
GPIOC->BRR=RS; //Initialize RS=0 for selecting instruction Send
GPIOC->BRR=RW; // Select RW=0 to write Instruction/data on LCD
GPIOC->BSRR=EN; // EN=1 for unlatch. (used at initial condition)

delay_ms(10);
GPIOA->ODR=0x00C0; // Move cursor to beginning of second row
s_latch(); //Latch the above instruction
s_data(); //Change the input type to Data.(before it was instruction input)
for(k=0;b[k];k++)
{
GPIOA->ODR=b[k]; //It will send b[0]='E' as = '0x0044' on Port A.
s_latch();//Latch the above instruction only once. Or it will clone each character twice if you latch twice.
}
s_init();
}
