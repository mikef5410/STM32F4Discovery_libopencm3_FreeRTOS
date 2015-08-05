#define GLOBAL_HIRESTIMER
#include "hiresTimer.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

//Our timer is clocked by core clock/2
#define CORE_CLOCK_SCALE 4UL

void init_hiresTimer(void)
{
  rcc_periph_clock_enable(RCC_HIRES_TIMER);
  timer_reset(HIRES_TIMER);
  /* Timer global mode:
   * - No divider
   * - Alignment edge
   * - Direction up
   */
  timer_set_mode(HIRES_TIMER, TIM_CR1_CKD_CK_INT,
                 TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
  timer_set_prescaler(HIRES_TIMER,1); //Divide-by-2
  timer_disable_preload(HIRES_TIMER);
  timer_continuous_mode(HIRES_TIMER);
  timer_generate_event(HIRES_TIMER, TIM_EGR_UG); //Force register load
  timer_disable_update_event(HIRES_TIMER);
  timer_enable_counter(HIRES_TIMER);

}



uint64_t hiresTimer_getTime(void) 
{
  volatile uint32_t current = timer_get_counter(HIRES_TIMER);
  if (current < hiresTimer_lastReading) hiresTimer_upperWord++;
  hiresTimer_lastReading=current;
  return(((uint64_t)hiresTimer_upperWord)<<32 | current);
}

uint32_t us2ticks(uint32_t us)
{
  return( ((10 * us)*(SystemCoreClock/(CORE_CLOCK_SCALE*1000UL)))/10000UL );
}

uint32_t ns2ticks(uint32_t ns)
{
  return( (ns * (SystemCoreClock/(CORE_CLOCK_SCALE*10000UL)))/100000UL );
}

// convert hires timer deltas to uSec and/or mSec
// NOTES: integer math only, values to nearest 1 uSec or mSec
//        minimum return set to 1 to avoid DIV-BY-ZERO
int32_t tics2us(int64_t delta_tics) {
  uint32_t rval = 1;
  if(delta_tics < 0) delta_tics -= delta_tics;
  // SystemCoreClock = 202 MHz = 202000000UL == 202000000 tics/sec.
  // It takes at least 202 tics for 1 uSec  or 202000000/1000000
  // if(delta_tics > SystemCoreClock) {
  if(delta_tics > (SystemCoreClock/(CORE_CLOCK_SCALE*1000000UL))) {
        rval = (delta_tics*1000000UL*CORE_CLOCK_SCALE)/(SystemCoreClock);
  }
  return(rval);
}

int32_t tics2ms(int64_t delta_tics) {
  uint32_t rval = 1;
  if(delta_tics < 0) delta_tics -= delta_tics;
  // takes at least 1000 clk-rate-hz tics for 1 mSec
  // if(delta_tics > (1000UL * SystemCoreClock)) { 
  // SystemCoreClock = 202 MHz = 202000000UL == 202000000 tics/sec.
  // 1 mSec, in tics, would be 1000 times less, or 202000000/1000
  if(delta_tics > (SystemCoreClock/(CORE_CLOCK_SCALE*1000))) {
    // rval = (delta_tics*1000000000UL)/(SystemCoreClock);
    rval = (delta_tics*1000UL*CORE_CLOCK_SCALE)/SystemCoreClock;
  }
  return(rval);
}
