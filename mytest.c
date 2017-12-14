/* mytest.c:
 *
 *  Test C to assembly interface 
 */

#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#include "common.h"

int mytest( int x );

void CmdTest(int action)
{

  if(action==CMD_SHORT_HELP) return;
  if(action==CMD_LONG_HELP) {
    printf("testasm\n\n"
	   "This command tests the C to assembler interface\n"
	   );

    return;
  }
  printf("ret val = %d\n", mytest( 77  ) );
}

ADD_CMD("cmdtest",CmdTest,"Test C to asm")
