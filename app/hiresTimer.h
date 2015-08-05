#ifndef _HIRESTIMER_INCLUDED
#define _HIRESTIMER_INCLUDED

#include "OSandPlatform.h"

#ifdef GLOBAL_HIRESTIMER
#define HIRESTIMERGLOBAL
#define HIRESTIMERPRESET(A) = (A)
#else
#define HIRESTIMERPRESET(A)
#ifdef __cplusplus
#define HIRESTIMERGLOBAL extern "C"
#else
#define HIRESTIMERGLOBAL extern
#endif  /*__cplusplus*/
#endif                          /*GLOBAL_HIRESTIMER */

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------
// PRIVATE API AND SUBJECT TO CHANGE!
// ----------------------------------------------------------------
#define HIRES_TIMER TIM2
#define RCC_HIRES_TIMER RCC_TIM2

  
HIRESTIMERGLOBAL uint32_t hiresTimer_upperWord;
HIRESTIMERGLOBAL uint32_t hiresTimer_lastReading;

// ----------------------------------------------------------------
// PUBLIC API definition
// ----------------------------------------------------------------


HIRESTIMERGLOBAL   void     init_hiresTimer(void);
HIRESTIMERGLOBAL   uint64_t hiresTimer_getTime(void);
HIRESTIMERGLOBAL   uint32_t us2ticks(uint32_t us);
HIRESTIMERGLOBAL   uint32_t ns2ticks(uint32_t ns);
HIRESTIMERGLOBAL   int32_t  tics2us(int64_t delta_tics);
HIRESTIMERGLOBAL   int32_t  tics2ms(int64_t delta_tics);


#ifdef __cplusplus
}
#endif


#endif                          //_HIRESTIMER_INCLUDED
