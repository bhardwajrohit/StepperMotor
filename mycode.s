@@ mycode.s :
@@ Test code for STM32 and linking assembly to C
 

    .code   16
    .text                   @@ - Code section (text -> ROM)

@@ <function block>
    .align  2               @@ - 2^n alignment (n=2)
    .syntax unified
    .global mytest          @@ - Symbol name for function
    .code   16              @@ - 16bit THUMB code (BOTH are required!)
    .thumb_func             @@ /
    .type   mytest, %function   @@ - symbol type (not req)
@@ Declaration : int mytest(int x)
@@ Uses r0 for param 0
@@   r0: x
mytest:
    push {lr}
    push {r1}
    push {r0-r7}
    ldr  r0, =0
    bl   BSP_LED_Toggle           @@ call BSP function
    pop  {r0-r7}
    ldr  r1, =myTickCount
    ldr  r0, [r1]
    pop  {r1} 
    pop  {pc}
    .size   mytest, .-mytest    @@ - symbol size (not req)

@@ <function block>
    .align  2               @@ - 2^n alignment (n=2)
    .syntax unified
    .global my_Tick          @@ - Symbol name for function
    .code   16              @@ - 16bit THUMB code (BOTH are required!)
    .thumb_func             @@ /
    .type   my_Tick, %function   @@ - symbol type (not req)
@@ Declaration : void my_Tick( void )
@@ Uses nothing
my_Tick:
    push {lr}
    push {r0-r1}

    ldr  r1, =myTickCount
    ldr  r0, [r1]
    add  r0, r0, #1
    str  r0, [r1]
    pop {r0-r1}
    pop  {pc}
    .size   my_Tick, .-my_Tick    @@ - symbol size (not req)

@@ <function block>
    .align  2               @@ - 2^n alignment (n=2)
    .syntax unified
    .global my_Loop          @@ - Symbol name for function
    .code   16              @@ - 16bit THUMB code (BOTH are required!)
    .thumb_func             @@ /
    .type   my_Loop, %function   @@ - symbol type (not req)
@@ Declaration : void my_Loop( void )
@@ Uses nothing
my_Loop:
    push {lr}
    push {r0-r1}
    ldr r1, =myTickCount            @@ - load the address of myTickCount in r1
    ldr r0, [r1]                    @@ - load the value of r1 in r0
    cmp r0, #1000                   @@ - Compare the value of r0 with 1000
    bge HERE                        @@ - if branch is greater than or equals to 1000 jump to a label named HERE
    pop {r0-r1}
    pop {pc}                        @@ - Cause the function to return
HERE: 
    ldr r5,=myTickCount             @@ - load the address of myTickCount in r5
    mov r6,#0                       @@ - set r6 value to zero
    str r6,[r5]                     @@ - store the value of r6 in the address pointed by r5(reset the myTickCount to zero)

    ldr r0, =0                      @@ - passing the parameter in the function BSP_LED_Toggle (r0 = 0)
    bl   BSP_LED_Toggle             @@ - Call BSP function
    ldr r0, =1                      @@ - passing the parameter in the function BSP_LED_Toggle (r0 = 1)
    bl   BSP_LED_Toggle             @@ - Call BSP function
    ldr r0, =2                      @@ - passing the parameter in the function BSP_LED_Toggle (r0 = 2)
    bl   BSP_LED_Toggle             @@ - Call BSP function
    ldr r0, =3                      @@ - passing the parameter in the function BSP_LED_Toggle (r0 = 3)
    bl   BSP_LED_Toggle             @@ - Call BSP function
    ldr r0, =4                      @@ - passing the parameter in the function BSP_LED_Toggle (r0 = 4)
    bl   BSP_LED_Toggle             @@ - Call BSP function
    ldr r0, =5                      @@ - passing the parameter in the function BSP_LED_Toggle (r0 = 5)
    bl   BSP_LED_Toggle             @@ - Call BSP function
    ldr r0, =6                      @@ - passing the parameter in the function BSP_LED_Toggle (r0 = 6)
    bl   BSP_LED_Toggle             @@ - Call BSP function
    ldr r0, =7                      @@ - passing the parameter in the function BSP_LED_Toggle (r0 = 7)
    bl   BSP_LED_Toggle             @@ - Call BSP function

    .size   my_Loop, .-my_Loop    @@ - symbol size (not req)
@@ <function block>
    .align  2               @@ - 2^n alignment (n=2)
    .syntax unified
    .global my_Init          @@ - Symbol name for function
    .code   16              @@ - 16bit THUMB code (BOTH are required!)
    .thumb_func             @@ /
    .type   my_Init, %function   @@ - symbol type (not req)
@@ Declaration : void my_Init( void )
@@ Uses nothing
my_Init:
    push {lr}
    pop  {pc}
    .size   my_Init, .-my_Init    @@ - symbol size (not req)

    .data
    .global myTickCount
myTickCount:
    .word  1         /* A 32-bit variable named myTickCount */


    .end

