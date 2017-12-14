/* Simple monitor commands */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "common.h"



void CmdR(int mode)
{
  uint32_t addr;

  if(mode != CMD_INTERACTIVE) return;

  fetch_uint32_arg(&addr);

  printf("0x%08X: 0x%08X\n",(unsigned int)addr, 
	 (unsigned int)(*((uint32_t *)addr)));
}


void CmdW(int mode)
{
  uint32_t addr,data;

  if(mode != CMD_INTERACTIVE) return;

  fetch_uint32_arg(&addr);
  fetch_uint32_arg(&data);

  *((uint32_t *)addr) = data;
}


ADD_CMD("r",CmdR,    "<ADDR>          Read Memory")
ADD_CMD("w",CmdW,    "<ADDR> <DATA>   Write Memory")

