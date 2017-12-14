/* Terminal I/O services for STM32F3 discovery board.
 *
 *  Implement the low-level _write_r and _read_r functions needed for
 *  newlib. Use the underlying HAL calls to communicate with
 *  peripheral drivers.
 */

#include <stdio.h>
#include <string.h>
#include "stm32f3xx_hal.h"
#include "stm32f3_discovery.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h" 
#include "usbd_cdc_interface.h"
#include "common.h"


#ifdef USE_UART
/* UART handler declaration */
UART_HandleTypeDef UartHandle;
#endif
#ifdef USE_USB
USBD_HandleTypeDef hUSBDDevice;

/* Storage for driver supplied function */
uint8_t  (*USBDDataIn)(struct _USBD_HandleTypeDef *pdev , uint8_t epnum);   

/* Replacement for USBDataIn from CDC... so we can get notified of Tx Complete
 */
static uint8_t  USBD_CDC_DataIn (USBD_HandleTypeDef *pdev, 
                                 uint8_t epnum);

#endif


#define TERMINALBUFFERSIZE 128
#define TERMINALINCR(x) (((x)+1) % TERMINALBUFFERSIZE)
#define TERMINALINCRBY(x,y) (((x)+(y)) % TERMINALBUFFERSIZE)

typedef struct _TerminalStats_s {
  /* Statistics counters */
  uint32_t  writeTooBig;
  uint32_t  writeBlocked;
  uint32_t  written;
  uint32_t  receiveTooBig;
  uint32_t  received;
} TerminalStats_t;  

typedef struct _TerminalState_s {
  char      inBuffer[TERMINALBUFFERSIZE];
  volatile uint16_t  inHead, inTail, inCount;  /* Head and tail indexes */
  char      outBuffer[TERMINALBUFFERSIZE];
  volatile uint16_t  outHead, outTail, outCount;
  volatile uint16_t  outSending;
  
  TerminalStats_t stats;


} TerminalState_t;

/* allocate Terminal State structures */
static TerminalState_t TerminalState[INDEX_MAX];

/* Private functions */
uint32_t TerminalOutputBufferWrite(uint32_t index, char *p, uint32_t len);
uint32_t TerminalInputBufferWrite(uint32_t index, char *p, uint32_t len);

void TerminalInit(void)
{
#ifdef USE_UART
  GPIO_InitTypeDef  GPIO_InitStruct;
#endif

  /* Set all stream I/O to non-buffered */
  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);

#ifdef USE_UART
  /* Configure the GPIO pins for the UART */
  __GPIOC_CLK_ENABLE();
  GPIO_InitStruct.Pin       = GPIO_PIN_5;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = 7;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin       = GPIO_PIN_4;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = 7;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* NVIC for USART */
  HAL_NVIC_SetPriority(USART1_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(USART1_IRQn);

 /*##-1- Configure the UART peripheral ######################################*/
  __USART1_CLK_ENABLE();
  /* Put the USART peripheral in the Asynchronous mode (UART Mode) */
  /* UART configured as follows:
      - Word Length = 8 Bits
      - Stop Bit    = One Stop bit
      - Parity      = ODD parity
      - BaudRate    = UARTBAUDRATE baud
      - Hardware flow control disabled (RTS and CTS signals) */
  UartHandle.Instance        = USART1;

  UartHandle.Init.BaudRate   = UARTBAUDRATE;
  UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits   = UART_STOPBITS_1;
  UartHandle.Init.Parity     = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode       = UART_MODE_TX_RX;

  if (HAL_UART_Init(&UartHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* Enable the UART Rx interrupt transfers, when a transfer
   * completes, it will call us back with the Rx Complete callback.
   * We will 'expect' only a single character.  When it arrives, we
   * will stuff it in the input buffer, and then call back for another
   * character.
   */
  if(HAL_UART_Receive_IT(&UartHandle,
			 (uint8_t *)(&(TerminalState[0]
				       .inBuffer[TerminalState[0]
						 .inHead])),
			 1) != HAL_OK) {
    Error_Handler();
  }
#endif

#ifdef USE_USB
  /* Initialize USB interface */
  /* Init Device Library */
  USBD_Init(&hUSBDDevice, &VCP_Desc, 0);
  
  /* Add Supported Class */
  /* Save, and replace DataIn callback with our own...*/
  USBDDataIn = USBD_CDC.DataIn;
  USBD_CDC.DataIn = USBD_CDC_DataIn;
  USBD_RegisterClass(&hUSBDDevice, &USBD_CDC);
  
  /* Add CDC Interface Class */
  USBD_CDC_RegisterInterface(&hUSBDDevice, &USBD_CDC_fops);
  
  /* Start Device Process */
  USBD_Start(&hUSBDDevice);
#endif

}

int _write_r(void *reent, int fd, char *ptr, size_t len)
{
  uint32_t count;
  /* Frob buffer to add appropriate carriage returns and newlines */
  count = len;
  while(count != 0) {
    if(*ptr == '\n') {
      /* XXX handle buffer overflow */
#ifdef USE_UART
      TerminalOutputBufferWrite(INDEX_UART,"\r",1);
#endif
#ifdef USE_USB
      TerminalOutputBufferWrite(INDEX_USB,"\r",1);
#endif
    }
#ifdef USE_UART
    TerminalOutputBufferWrite(INDEX_UART,ptr,1);
#endif
#ifdef USE_USB
    TerminalOutputBufferWrite(INDEX_USB,ptr,1);
#endif

    ptr++;
    count--;
  }

#if 0
  /* Transmit Terminal buffer... */
  if (HAL_UART_Transmit(&UartHandle, outputBuffer,
			i, HAL_MAX_DELAY) != HAL_OK)
  {
    /* Transfer error in transmission process */
    Error_Handler();
  }
#endif
  return len;
}

int _read_r( void *r, int fd, char *ptr, int len )
{
  uint32_t count;
  count = TerminalRead(0, (uint8_t *)ptr, len);

  return count;
}

/* Attempt to read a block of data from the Terminal buffer
 * return the actual number of bytes read.
 */
uint32_t TerminalRead(uint32_t index, uint8_t *ptr, uint32_t len)
{
  uint32_t count, tail;
  /* try to read len characters from the input buffer */

  /* Crticial section begin */
  __disable_irq();
  count = 0;
  while(TerminalState[index].inCount && (count < len)) {
    tail   = TerminalState[index].inTail;
    *ptr++ = TerminalState[index].inBuffer[tail];
    TerminalState[index].inTail = TERMINALINCR(tail);
    TerminalState[index].inCount--;
    count++;
  }
  /* Critical section end */
  __enable_irq();

  return count;

}

/* Attempt to read a single character from the Terminal buffer(s), return 1
 * if none are available. Return 0 if there is a character available.
 */
int TerminalReadNonBlock(uint32_t index, char *c)
{
  /* Check for a character to be ready */
  if(TerminalRead(index,(uint8_t*)c,1) == 0) {
    /* Nope, just return */
    return 1;
  }
  return 0;
}

/* Scan through all possible terminal input buffers and return if
 * there is a character available.
 */
int TerminalReadAnyNonBlock(char *c)
{
  uint32_t i;
  int rc;

  for(i=0; i<INDEX_MAX; i++) {
    rc = TerminalReadNonBlock(i, c);
    if(rc==0) {
      return rc;
    }
  }
  return 1;
}


/* Write a block to the given terminal buffer, assume interrupts can
 * be disabled.
 */
 uint32_t TerminalOutputBufferWrite(uint32_t index, char *p, uint32_t len)
{
#ifdef USE_UART
  HAL_StatusTypeDef rc;
#endif
#ifdef USE_USB
  uint8_t cdcRc;
#endif
  uint32_t head, tail, count;
  /* Check that our block will ever fit, if not, return fail */
  if(len >= TERMINALBUFFERSIZE) {
    /* Keep track of how many times this happens */
    TerminalState[index].stats.writeTooBig++;
    return 1;
  }

  if((TerminalState[index].outCount+len) >= TERMINALBUFFERSIZE) {
    /* Keep track of how many times we block */
    TerminalState[index].stats.writeBlocked++;
  }

  /* Block until there is room in the buffer */
  while((TerminalState[index].outCount+len) >= TERMINALBUFFERSIZE) {} 

  /* Critical section begin */
  __disable_irq();
  /* Copy characters into the buffer */
  TerminalState[index].stats.written += len;  /* gather statistics */
  while((len != 0) 
	&& (TerminalState[index].outCount<TERMINALBUFFERSIZE)) {
    head = TerminalState[index].outHead;
    TerminalState[index].outBuffer[head] = *p++;
    TerminalState[index].outHead = TERMINALINCR(head);
    TerminalState[index].outCount++;
    len--;
  }
  
  /* Trigger output from this buffer */
  tail = TerminalState[index].outTail;
  count = TerminalState[index].outCount;
  /* Only allow a transfer to the end of the buffer */
  if((count + tail) >= TERMINALBUFFERSIZE) {
    count = TERMINALBUFFERSIZE - tail;
  }
  switch(index) {
#ifdef USE_UART
  case INDEX_UART:  /* The UART */
    rc = HAL_UART_Transmit_IT(&UartHandle,
			      (uint8_t*)&(TerminalState[index].outBuffer[tail]),
			      count);
    if(rc == HAL_OK) {
      TerminalState[index].outSending = count;
    } else if(rc != HAL_BUSY) {
      Error_Handler();
    }
    break;
#endif
#ifdef USE_USB
  case INDEX_USB: /* USB */
    if(hUSBDDevice.dev_state == USBD_STATE_CONFIGURED) {
      USBD_CDC_SetTxBuffer(&hUSBDDevice,
			   (uint8_t*)&(TerminalState[index].outBuffer[tail]),
			   count);
      cdcRc = USBD_CDC_TransmitPacket(&hUSBDDevice);
      if(cdcRc == USBD_OK) {
	/* CDC was not busy, and we are now sending */
	TerminalState[index].outSending = count;
      }
    } 
    break;
#endif
  }
    

  /* Critical section end */
  __enable_irq();
  return 0;
}

uint32_t TerminalInputBufferWrite(uint32_t index, char *p, uint32_t len)
{
  uint32_t head;

  /* Critical Section begin */
  //__disable_irq();
  /* Check that our block will fit, if not, return fail */
  if((TerminalState[index].inCount + len) >= TERMINALBUFFERSIZE) {
    //__enable_irq();
    TerminalState[index].stats.receiveTooBig++;
    return 1;
  }
  /* Copy characters into the buffer */
  TerminalState[index].stats.received += len;
  while(len != 0) {
    head = TerminalState[index].inHead;
    TerminalState[index].inBuffer[head] = *p++;
    TerminalState[index].inHead = TERMINALINCR(head);
    TerminalState[index].inCount++;
    len--;
  }
  
  /* Critical section end */
  //__enable_irq();
  return 0;
}




#ifdef USE_UART
/**
  * @brief  Rx Transfer completed callback
  * @param  UartHandle: UART handle
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *ThisUartHandle)
{
  uint32_t index=INDEX_UART, head;
  uint8_t *p;
  if(ThisUartHandle == &UartHandle) {
    head =  TerminalState[index].inHead;
    if((TerminalState[index].inCount + 1) < TERMINALBUFFERSIZE) {
      /* Do accounting for character */
      TerminalState[index].inHead = TERMINALINCR(head);
      TerminalState[index].inCount++;

    }
    
    head =  TerminalState[index].inHead;
    p = (uint8_t *)(uint8_t *)(&(TerminalState[index].inBuffer[head]));
    /* Re-arm interrupt */
    if(HAL_UART_Receive_IT(&UartHandle,
			   p,
			   1) != HAL_OK) {
      Error_Handler();
    }

  } 
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *ThisUartHandle)
{
  HAL_StatusTypeDef rc;
  uint32_t index=INDEX_UART, tail, count;
  if(ThisUartHandle == &UartHandle) {
    /* Update head and tail pointers, we just sent outSending bytes */
    tail = TerminalState[index].outTail;
    count = TerminalState[index].outSending;
    TerminalState[index].outTail = TERMINALINCRBY(tail,count);
    TerminalState[index].outCount -= count;
    TerminalState[index].outSending = 0;

    /* Re - Trigger output from this buffer */
    tail = TerminalState[index].outTail;
    count = TerminalState[index].outCount;
    if(count != 0) {
      /* Only allow a transfer to the end of the buffer */
      if((count + tail) >= TERMINALBUFFERSIZE) {
	count = TERMINALBUFFERSIZE - tail;
      }
      rc = HAL_UART_Transmit_IT(&UartHandle,
				(uint8_t*)&(TerminalState[index].outBuffer[tail]),
				count);
      if(rc == HAL_OK) {
	TerminalState[index].outSending = count;
      } else if(rc != HAL_BUSY) {
	Error_Handler();
      }
    }
  }
}
#endif

#ifdef USE_USB
/* USB IN endpoint Transmission complete Callback */
static uint8_t  USBD_CDC_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  uint8_t rc;
  uint32_t index=INDEX_USB, tail, count;

  /* Call the original function */
  rc = (*USBDDataIn)(pdev,epnum);

  if(pdev == &hUSBDDevice) {
    /* Update head and tail pointers, we just sent outSending bytes */
    tail = TerminalState[index].outTail;
    count = TerminalState[index].outSending;
    TerminalState[index].outTail = TERMINALINCRBY(tail,count);
    TerminalState[index].outCount -= count;
    TerminalState[index].outSending = 0;

    /* Re - Trigger output from this buffer */
    tail = TerminalState[index].outTail;
    count = TerminalState[index].outCount;
    if(count != 0) {
      /* Only allow a transfer to the end of the buffer */
      if((count + tail) >= TERMINALBUFFERSIZE) {
	count = TERMINALBUFFERSIZE - tail;
      }
      USBD_CDC_SetTxBuffer(&hUSBDDevice,
			   (uint8_t*)&(TerminalState[index].outBuffer[tail]),
			   count);
      USBD_CDC_TransmitPacket(&hUSBDDevice);
      TerminalState[index].outSending = count;
    }
  }
    
  return rc;
}
#endif

#ifdef USE_UART
/* IRQ handler trampoline for HAL UART Driver */
void USART1_IRQHandler(void)
{
  HAL_UART_IRQHandler(&UartHandle);
}
#endif
#ifdef USE_USB
/**
  * @brief  This function handles USB Handler.
  * @param  None
  * @retval None
  */
extern PCD_HandleTypeDef hpcd;
#if defined (USE_USB_INTERRUPT_DEFAULT)
void USB_LP_CAN_RX0_IRQHandler(void)
#elif defined (USE_USB_INTERRUPT_REMAPPED)
void USB_LP_IRQHandler(void)
#endif
{
  HAL_PCD_IRQHandler(&hpcd);
}

#endif

/* Dump the terminal statistics */
void CmdStats(int mode)
{
  uint32_t i;
  TerminalStats_t s[INDEX_MAX];

  if(mode != CMD_INTERACTIVE) return;

  /* Copy terminal stats to local storage, and reset counters atomincally */
  __disable_irq();
  for(i=0; i<INDEX_MAX; i++) {
    s[i] = TerminalState[i].stats;
    bzero(&TerminalState[i].stats, sizeof(TerminalState[i].stats));
  }
  __enable_irq();

  /* Display stats for the user */
  for(i=0; i<INDEX_MAX; i++) {
    printf("Terminal #%u:\n"
	   "   writeTooBig:   %u\n"
	   "   writeBlocked:  %u\n"
	   "   written:       %u\n"
	   "   receiveTooBig: %u\n"
	   "   received:      %u\n",
	   (unsigned int)i,
	   (unsigned int)(s[i].writeTooBig),
	   (unsigned int)(s[i].writeBlocked),
	   (unsigned int)(s[i].written),
	   (unsigned int)(s[i].receiveTooBig),
	   (unsigned int)(s[i].received));
  }

}

ADD_CMD("termstat", CmdStats, "                Dump terminal statistics");

  
