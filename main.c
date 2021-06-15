
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "usrcmd.h"
#include "cy_log.h"

volatile int uxTopUsedPriority ;
TaskHandle_t blinkTaskHandle;


void blink_task(void *arg)
{
    cyhal_gpio_init(CYBSP_USER_LED,CYHAL_GPIO_DIR_OUTPUT,CYHAL_GPIO_DRIVE_STRONG,0);

    for(;;)
    {
        cy_log_msg(CYLF_DEF,CY_LOG_INFO,"Blink Info\n");
    	cyhal_gpio_toggle(CYBSP_USER_LED);
    	vTaskDelay(500);
    }
}


int main(void)
{
    uxTopUsedPriority = configMAX_PRIORITIES - 1 ; // enable OpenOCD Thread Debugging

    /* Initialize the device and board peripherals */
    cybsp_init() ;
    __enable_irq();

    cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);

    cy_log_init(CY_LOG_INFO,0,0);


    // Stack size in WORDs
    // Idle task = priority 0
    xTaskCreate(blink_task, "blinkTask", configMINIMAL_STACK_SIZE*2,0 /* args */ ,0 /* priority */, &blinkTaskHandle);
    xTaskCreate(usrcmd_task, "usrcmd_task", configMINIMAL_STACK_SIZE*4,0 /* args */ ,0 /* priority */, 0);
    vTaskStartScheduler();
}

/* [] END OF FILE */
