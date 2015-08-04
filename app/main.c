#define GLOBAL_OSandPlatform
#include "OSandPlatform.h"

#include "debug_shell.h"

#include <stdio.h>

uint32_t SystemCoreClock;
xTaskHandle *xLED1TaskHandle;
xTaskHandle *xLED2TaskHandle;
xTaskHandle *xUSBCDCACMTaskHandle;
xTaskHandle *xDebugShellTaskHandle;

extern void Board_UARTPutSTR(const char *str);

void Delay(volatile uint32_t nCount);

extern portTASK_FUNCTION(vUSBCDCACMTask, pvParameters); //in cdcacm.c

static portTASK_FUNCTION(vLEDTask1, pvParameters)
{
  (void)(pvParameters);//unused params
  uint32_t cnt=0;
  while(1) {
    cnt++;
    vTaskDelay(300/portTICK_RATE_MS);
    gpio_set(GPIOD, GPIO12);
    vTaskDelay(20/portTICK_RATE_MS);
    gpio_clear(GPIOD, GPIO12);
  }
}


static portTASK_FUNCTION(vLEDTask2, pvParameters)
{
  (void)(pvParameters);//unused params
  while(1) {
    vTaskDelay(500/portTICK_RATE_MS);
    gpio_set(GPIOD, GPIO12);
    vTaskDelay(30/portTICK_RATE_MS);
    gpio_clear(GPIOD, GPIO12);
  }
}

void greenOn(int on)
{
  if (on) {
    gpio_set(GPIOD,GPIO12);
  } else {
    gpio_clear(GPIOD,GPIO12);
  }
  return;
}

void orangeOn(int on)
{
  if (on) {
    gpio_set(GPIOD,GPIO13);
  } else {
    gpio_clear(GPIOD,GPIO13);
  }
  return;
}

void redOn(int on)
{
  if (on) {
    gpio_set(GPIOD,GPIO14);
  } else {
    gpio_clear(GPIOD,GPIO14);
  }
  return;
}

void blueOn(int on)
{
  if (on) {
    gpio_set(GPIOD,GPIO15);
  } else {
    gpio_clear(GPIOD,GPIO15);
  }
  return;
}

#define TEST96
#ifdef TEST96
const clock_scale_t testClock = {
  //96 MHz, 8MHz crystal
  .pllm = 2,
  .plln = 48,
  .pllp = 2,
  .pllq = 4, 
  .hpre = RCC_CFGR_HPRE_DIV_NONE,
  .ppre1 = RCC_CFGR_PPRE_DIV_4,
  .ppre2 = RCC_CFGR_PPRE_DIV_2,
  .flash_config = FLASH_ACR_ICE | FLASH_ACR_DCE |
  FLASH_ACR_LATENCY_3WS,
  .apb1_frequency = 24000000,
  .apb2_frequency = 48000000,
};
#endif

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
#ifndef TEST96  
  // Discovery is 8MHz crystal, use 120MHz core
  rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_120MHZ]);
  SystemCoreClock = 120000000;
#else
  // Use 96MHz core
  rcc_clock_setup_hse_3v3(&testClock);
  SystemCoreClock = 96000000;
#endif

  // Setup USBOTG Clocking and pins
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_OTGFS);
  
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE,
                  GPIO9 | GPIO11 | GPIO12);
  gpio_set_af(GPIOA, GPIO_AF10, GPIO9 | GPIO11 | GPIO12);
  
  
  // Setup GPIO D for LEDs
  rcc_periph_clock_enable(RCC_GPIOD);
  rcc_periph_clock_enable(RCC_GPIOC);
  gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO12|GPIO13|GPIO14|GPIO15);
  gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO12|GPIO13|GPIO14|GPIO15);
  gpio_set(GPIOD, GPIO12|GPIO13|GPIO14|GPIO15);
  Delay(0x8FFFFF);
  gpio_clear(GPIOD, GPIO12|GPIO13|GPIO14|GPIO15);

  // 'Scope probe on GPIOC-7
  gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO7);
  gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO7);

  init_hiresTimer();
  
  
#if 1
  //Fixup NVIC for FreeRTOS ...
  const uint32_t interrupt_priority_group4 = (0x3 << 8); // 15 priority interrupts, no sub-priorities
  scb_set_priority_grouping(interrupt_priority_group4);
  for (int irqNum=0; irqNum<=NVIC_IRQ_COUNT ; irqNum++) {
    nvic_set_priority(irqNum, 0x6f);
  }
  nvic_set_priority(-4,0); //MMU Fault
  nvic_set_priority(-5,0); //Bus Fault
  nvic_set_priority(-6,0); //Usage Fault
  nvic_set_priority(-11,0); //SVCall
  nvic_set_priority(-14,0); //PendSV
  nvic_set_priority(-15,0); //SysTick
  
#endif  
  // Create tasks
  // remember, stack size is in 32-bit words and is allocated from the heap ...
  qStatus = xTaskCreate(vLEDTask1, "LED Task 1", 64, NULL, (tskIDLE_PRIORITY + 1UL),
                        (xTaskHandle *) &xLED1TaskHandle);


  qStatus = xTaskCreate(vLEDTask2, "LED Task 2", 64, NULL, (tskIDLE_PRIORITY + 1UL),
                        (xTaskHandle *) &xLED2TaskHandle);


  qStatus = xTaskCreate(vUSBCDCACMTask, "USB Serial Task", 64, NULL, (tskIDLE_PRIORITY + 1UL),
                        (xTaskHandle *) &xUSBCDCACMTaskHandle);

  qStatus = xTaskCreate(vDebugShell, "Debug shell", 1024, NULL, (tskIDLE_PRIORITY + 1UL),
                        (xTaskHandle *) &xDebugShellTaskHandle);
  
  (void) qStatus;

  // start the scheduler
  vTaskStartScheduler();

  /* Control should never come here */
  //DEBUGSTR("Scheduler Failure\n");
  while (1) {}
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
