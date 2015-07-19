#define GLOBAL_OSandPlatform
#include "OSandPlatform.h"

#include <libopencmsis/core_cm3.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/nvic.h>



#include <stdio.h>

uint32_t SystemCoreClock;
xTaskHandle *xLED1TaskHandle;
xTaskHandle *xLED2TaskHandle;

void Delay(volatile uint32_t nCount);

static portTASK_FUNCTION(vLEDTask1, pvParameters)
{
  (void)(pvParameters);//unused params
  while(1) {
    vTaskDelay(300/portTICK_RATE_MS);
    gpio_toggle(GPIOD, GPIO12);
  }
}


static portTASK_FUNCTION(vLEDTask2, pvParameters)
{
  (void)(pvParameters);//unused params
  while(1) {
    vTaskDelay(500/portTICK_RATE_MS);
    gpio_toggle(GPIOD, GPIO13);
  }
}




int main(void)
{
  portBASE_TYPE qStatus = pdPASS;   // = 1, this, and pdFAIL = 0, are in projdefs.h
  //Blue pushbutton (B1) is PA0
  //Black pushbutton (B2) is NRST
  /* Configure LEDs ... PD12, PD13, PD14 and PD15 in output pushpull mode */
  //PD12 - Green
  //PD13 - Orange
  //PD14 - Red
  //PD15 - Blue

  // Now setup the clocks ...
  // Discovery is 8MHz crystal, use 120MHz core
  rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_120MHZ]);
  SystemCoreClock = 120000000;

  // Setup GPIO D
  rcc_periph_clock_enable(RCC_GPIOD);
  gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO12|GPIO13|GPIO14|GPIO15);
  gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO12|GPIO13|GPIO14|GPIO15);
  gpio_set(GPIOD, GPIO12|GPIO13|GPIO14|GPIO15);
  Delay(0x8FFFFF);
  gpio_clear(GPIOD, GPIO12|GPIO13|GPIO14|GPIO15);

  // Create tasks
  qStatus = xTaskCreate(vLEDTask1, "LED Task 1", 2048, NULL, (tskIDLE_PRIORITY + 1UL),
                        (xTaskHandle *) &xLED1TaskHandle);


  qStatus = xTaskCreate(vLEDTask2, "LED Task 2", 2048, NULL, (tskIDLE_PRIORITY + 1UL),
                        (xTaskHandle *) &xLED2TaskHandle);

  
  (void) qStatus;

  
  // start the scheduler
  vTaskStartScheduler();
  /* Control should never come here */
  //DEBUGSTR("Scheduler Failure\n");
  while (1) {}

  return 1;


  /* this done last */
  //systick_interrupt_enable();

  
  while (1) {
    gpio_toggle(GPIOD, GPIO15|GPIO12|GPIO13|GPIO14);
    Delay(0x8FFFFF);
  }
}
  
void Delay(volatile uint32_t nCount)
{
  while (nCount--) {
    __asm__("nop");
  }
}



void vApplicationStackOverflowHook( xTaskHandle xTask __attribute__(( unused )), signed char *pcTaskName __attribute__(( unused )) )
{
  while (1) {
  }  
  return;
}

void vApplicationMallocFailedHook( void ) {
  while (1) {
  }  
  return;
}

//	vApplicationIdleHook() ...
// 
//  will only be called if configUSE_IDLE_HOOK is set to 1 in FreeRTOSConfig.h.
//  It will be called on each iteration of the idle task.
//
//	It is essential that code added to this hook function never attempts
//	to block in any way (for example, call xQueueReceive() with a block time
//	specified, or call vTaskDelay()).  If the application makes use of the
//	vTaskDelete() API function (as this demo application does) then it is also
//	important that vApplicationIdleHook() is permitted to return to its calling
//	function, because it is the responsibility of the idle task to clean up
//	memory allocated by the kernel to any task that has since been deleted.
//
void vApplicationIdleHook( void ) {
  // jjones:  I have configured to use the IdleHook, but don't
  //          really have anything for it do do... yet.
  //          The default behavior is just to surrender to the tick ...
  __WFI();
  // What is shown below is from an LWIP and USB-CDC demo, where the idle task
  // lazy-dumps characters out the USB port.  I include it as a tutorial on
  // using the TickCount (or some other higher-res timer) to  prevent the
  // idle task from churning, or sending any faster than necessary.

  //  static portTickType xLastTx = 0;
  //  char cTxByte;
  //  
  //  /* The idle hook simply sends a string of characters to the USB port.
  //  	 The characters will be buffered and sent once the port is connected. */
  //  if( ( xTaskGetTickCount() - xLastTx ) > mainUSB_TX_FREQUENCY ) {
  //  	xLastTx = xTaskGetTickCount();
  //  	for( cTxByte = mainFIRST_TX_CHAR; cTxByte <= mainLAST_TX_CHAR; cTxByte++ ) {
  //  	  vUSBSendByte( cTxByte );
  //  	}		
  //  }
} // end vApplicationIdleHook


// FreeRTOS application tick hook 
void vApplicationTickHook(void)
{}

void _exit(int status __attribute__(( unused )) )
{
  gpio_set(GPIOD, GPIO14);
  
  //TRIG_HARDFAULT;
  while (1) ;
}
