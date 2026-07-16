;/**************************************************************************//**
; * @file     startup_gpm32f13xx.s
; * @brief    Startup File for GPM32F13xx Series Device
; *           
; * @version  V0.0
; * @date     14th, Oct 2024
; *
; * @note
; *
; ******************************************************************************/
;/* Copyright (c) 2011 - 2012 ARM LIMITED
;
;   All rights reserved.
;   Redistribution and use in source and binary forms, with or without
;   modification, are permitted provided that the following conditions are met:
;   - Redistributions of source code must retain the above copyright
;     notice, this list of conditions and the following disclaimer.
;   - Redistributions in binary form must reproduce the above copyright
;     notice, this list of conditions and the following disclaimer in the
;     documentation and/or other materials provided with the distribution.
;   - Neither the name of ARM nor the names of its contributors may be used
;     to endorse or promote products derived from this software without
;     specific prior written permission.
;   *
;   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
;   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
;   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
;   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
;   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
;   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
;   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
;   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
;   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
;   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
;   POSSIBILITY OF SUCH DAMAGE.
;   ---------------------------------------------------------------------------*/
;/*
;//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
;*/



; <h> Stack Configuration
;   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Stack_Size      EQU     0x00000800

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp


; <h> Heap Configuration
;   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Heap_Size       EQU     0x00000000

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit


                PRESERVE8
                THUMB


; Vector Table Mapped to Address 0 at Reset

                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__Vectors       DCD     __initial_sp              ; Top of Stack
                DCD     Reset_Handler             ; Reset Handler
                DCD     NMI_Handler               ; NMI Handler
                DCD     HardFault_Handler         ; Hard Fault Handler
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     SVC_Handler               ; SVCall Handler
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     PendSV_Handler            ; PendSV Handler
                DCD     SysTick_Handler           ; SysTick Handler

                ; External Interrupts
                DCD     WDG_IRQHandler            ;  0: Watchdog Timer
                DCD     LVD_IRQHandler            ;  1: Low Voltage Detection Level Trigger Interrupt
                DCD     DMA_IRQHandler            ;  2: DMA Interrupt
                DCD     OVC0_IRQHandler           ;  3: Over Current 0 Interrupt
                DCD     0                         ;  4: Reserved
                DCD     ADC0_IRQHandler           ;  5: ADC0 Interrupt
                DCD     ADC1_IRQHandler           ;  6: ADC1 Interrupt
                DCD     eCCU60_IRQHandler         ;  7: eCCU60 Interrupt
                DCD     0                         ;  8: Reserved
                DCD     CCU40_IRQHandler          ;  9: CCU40 Interrupt
                DCD     0                         ; 10: Reserved
                DCD     POSIF0_IRQHandler         ; 11: POSIF0 Interrupt
                DCD     0                         ; 12: Reserved
                DCD     MATH_IRQHandler           ; 13: MATH Interrupt
                DCD     I2C0_IRQHandler           ; 14: I2C0 Interrupt
                DCD     0                         ; 15: Reserved
                DCD     UART0_IRQHandler      	  ; 16: UART0 Interrupt
                DCD     UART1_IRQHandler     	  ; 17: UART1 Interrupt
                DCD     UART2_IRQHandler          ; 18: UART2 Interrupt
                DCD     SPI0_IRQHandler           ; 19: SPI0 Interrupt
                DCD     SPI1_IRQHandler           ; 20: SPI1 Interrupt
                DCD     EXTI0_IRQHandler          ; 21: External Input 0 Interrupt
                DCD     EXTI1_IRQHandler          ; 22: External Input 1 Interrupt
                DCD     EXTI2_IRQHandler          ; 23: External Input 2 Interrupt
                DCD     EXTI3_IRQHandler          ; 24: External Input 3 Interrupt
                DCD     EXTI4_15_IRQHandler       ; 25: External Input 4 - 15 Interrupt
                DCD     TMU0_IRQHandler           ; 26: TMU0 Interrupt
                DCD     TMU1_IRQHandler           ; 27: TMU1 Interrupt
                DCD     ACMP0_IRQHandler          ; 28: Analog Comparator 0 Interrupt
                DCD     ACMP1_IRQHandler          ; 29: Analog Comparator 1 Interrupt
                DCD     CANFD80_IRQHandler        ; 30: CANFD80 Interrupt
                DCD     0       	              ; 31: Reserved
__Vectors_End

__Vectors_Size  EQU     __Vectors_End - __Vectors


                AREA    |.text|, CODE, READONLY


; Reset Handler

Reset_Handler   PROC
                EXPORT  Reset_Handler             [WEAK]
                IMPORT  SystemInit
                IMPORT  __main

                LDR     R0, =SystemInit
                BLX     R0
                LDR     R0, =__main
                BX      R0
                ENDP



; Dummy Exception Handlers (infinite loops which can be modified)

NMI_Handler     PROC
                EXPORT  NMI_Handler               [WEAK]
                B       .
                ENDP
HardFault_Handler\
                PROC
                EXPORT  HardFault_Handler         [WEAK]
                B       .
                ENDP
SVC_Handler     PROC
                EXPORT  SVC_Handler               [WEAK]
                B       .
                ENDP
PendSV_Handler  PROC
                EXPORT  PendSV_Handler            [WEAK]
                B       .
                ENDP
SysTick_Handler PROC
                EXPORT  SysTick_Handler           [WEAK]
                B       .
                ENDP

Default_Handler PROC

                EXPORT  WDG_IRQHandler             	[WEAK]
                EXPORT  LVD_IRQHandler             	[WEAK]
                EXPORT  DMA_IRQHandler          	[WEAK]
                EXPORT  OVC0_IRQHandler            	[WEAK]
                EXPORT  ADC0_IRQHandler            	[WEAK]
				EXPORT  ADC1_IRQHandler            	[WEAK]
                EXPORT  eCCU60_IRQHandler          	[WEAK]
                EXPORT  CCU40_IRQHandler           	[WEAK]
                EXPORT  POSIF0_IRQHandler        	[WEAK]
				EXPORT  MATH_IRQHandler	            [WEAK]
                EXPORT  I2C0_IRQHandler            	[WEAK]
                EXPORT  UART0_IRQHandler           	[WEAK]
                EXPORT  UART1_IRQHandler           	[WEAK]
                EXPORT  UART2_IRQHandler        	[WEAK]
                EXPORT  SPI0_IRQHandler            	[WEAK]
                EXPORT  SPI1_IRQHandler          	[WEAK]
                EXPORT  EXTI0_IRQHandler         	[WEAK]
                EXPORT  EXTI1_IRQHandler         	[WEAK]
                EXPORT  EXTI2_IRQHandler         	[WEAK]
                EXPORT  EXTI3_IRQHandler         	[WEAK]
                EXPORT  EXTI4_15_IRQHandler        	[WEAK]
                EXPORT  TMU0_IRQHandler          	[WEAK]
                EXPORT  TMU1_IRQHandler          	[WEAK]
                EXPORT  ACMP0_IRQHandler           	[WEAK]
                EXPORT  ACMP1_IRQHandler        	[WEAK]
                EXPORT  CANFD80_IRQHandler        	[WEAK]
                ; External Interrupts

WDG_IRQHandler
LVD_IRQHandler
DMA_IRQHandler
OVC0_IRQHandler
ADC0_IRQHandler
ADC1_IRQHandler
eCCU60_IRQHandler
CCU40_IRQHandler
POSIF0_IRQHandler
MATH_IRQHandler
I2C0_IRQHandler
UART0_IRQHandler
UART1_IRQHandler
UART2_IRQHandler
SPI0_IRQHandler
SPI1_IRQHandler
EXTI0_IRQHandler
EXTI1_IRQHandler
EXTI2_IRQHandler
EXTI3_IRQHandler
EXTI4_15_IRQHandler
TMU0_IRQHandler
TMU1_IRQHandler
ACMP0_IRQHandler
ACMP1_IRQHandler
CANFD80_IRQHandler
                B       .

                ENDP


                ALIGN


; User Initial Stack & Heap

                IF      :DEF:__MICROLIB

                EXPORT  __initial_sp
                EXPORT  __heap_base
                EXPORT  __heap_limit

                ELSE

                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap

__user_initial_stackheap PROC
                LDR     R0, =  Heap_Mem
                LDR     R1, =(Stack_Mem + Stack_Size)
                LDR     R2, = (Heap_Mem +  Heap_Size)
                LDR     R3, = Stack_Mem
                BX      LR
                ENDP

                ALIGN

                ENDIF


                END
			

