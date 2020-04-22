// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include <stdint.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

static void freertos_task1( void* arg )
{
	int in = (int) arg;

	int32_t vr_in[8] = {11, 22, 33, 44, 55, 66, 77, 88};
	int32_t vd_in[8] = {111, 222, 333, 444, 555, 666, 777, 888};
	int32_t vc_in[8] = {1111, 2222, 3333, 4444, 5555, 6666, 7777, 8888};

	int32_t vr_out[8] = {0};
	int32_t vd_out[8] = {0};
	int32_t vc_out[8] = {0};

	rtos_printf("Task 1 got input %d\n", in);

    asm volatile (
    		"mov r11, %0\r\n"
    		"vldr r11[0]\r\n"
            "vldd %1[0]\n"
            "vldc %2[0]\n"
            :
            : "r"(vr_in), "r"(vd_in), "r"(vc_in)
    );

    rtos_printf("Task 1 VPU regs set\n");

    taskYIELD();

    asm volatile (
    		"mov r11, %2\r\n"
    		"vstr %0[0]\r\n"
            "vstd %1[0]\n"
            "vstc r11[0]\n"
            :
            : "r"(vr_out), "r"(vd_out), "r"(vc_out)
    );

    rtos_printf("Task 1 VPU regs:\n");

    rtos_printf("vR: ");
    for (int i = 0; i < 8; i++) {
    	rtos_printf("%d, ", vr_out[i]);
    }
    rtos_printf("\n");
    rtos_printf("vD: ");
    for (int i = 0; i < 8; i++) {
    	rtos_printf("%d, ", vd_out[i]);
    }
    rtos_printf("\n");
    rtos_printf("vC: ");
    for (int i = 0; i < 8; i++) {
    	rtos_printf("%d, ", vc_out[i]);
    }
    rtos_printf("\n");

	vTaskDelete(NULL);
}

static void freertos_task2( void* arg )
{
	int in = (int) arg;

	int32_t vr_in[8] = {1, 2, 3, 4, 5, 6, 7, 8};
	int32_t vd_in[8] = {10, 20, 30, 40, 50, 60, 70, 80};
	int32_t vc_in[8] = {100, 200, 300, 400, 500, 600, 700, 800};

	int32_t vr_out[8] = {0};
	int32_t vd_out[8] = {0};
	int32_t vc_out[8] = {0};

	rtos_printf("Task 2 got input %d\n", in);

    asm volatile (
    		"mov r11, %0\r\n"
    		"vldr r11[0]\r\n"
            "vldd %1[0]\n"
            "vldc %2[0]\n"
            :
            : "r"(vr_in), "r"(vd_in), "r"(vc_in)
    );

    rtos_printf("Task 2 VPU regs set\n");

    taskYIELD();

    asm volatile (
    		"mov r11, %2\r\n"
    		"vstr %0[0]\r\n"
            "vstd %1[0]\n"
            "vstc r11[0]\n"
            :
            : "r"(vr_out), "r"(vd_out), "r"(vc_out)
    );

    rtos_printf("Task 2 VPU regs:\n");

    rtos_printf("vR: ");
    for (int i = 0; i < 8; i++) {
    	rtos_printf("%d, ", vr_out[i]);
    }
    rtos_printf("\n");
    rtos_printf("vD: ");
    for (int i = 0; i < 8; i++) {
    	rtos_printf("%d, ", vd_out[i]);
    }
    rtos_printf("\n");
    rtos_printf("vC: ");
    for (int i = 0; i < 8; i++) {
    	rtos_printf("%d, ", vc_out[i]);
    }
    rtos_printf("\n");

	vTaskDelete(NULL);
}

int main(void)
{
#if __XS3A__
	rtos_printf("FreeRTOS running on XS3\n");
#endif

	rtos_printf("1\n");

	xTaskCreate(freertos_task1, "task1", configMINIMAL_STACK_SIZE, (void *) 42, 15, NULL);
	rtos_printf("2\n");

	xTaskCreate(freertos_task2, "task2", configMINIMAL_STACK_SIZE, (void *) 314, 15, NULL);
	rtos_printf("3\n");

    vTaskStartScheduler();
}
void vApplicationMallocFailedHook(void)
{
    debug_printf("Malloc failed!\n");
    configASSERT(0);
}

