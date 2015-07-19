#ifndef _OSandPlatform_INCLUDED
#define _OSandPlatform_INCLUDED

#ifdef GLOBAL_OSandPlatform
#define OSandPlatformGLOBAL
#define OSandPlatformPRESET(A) = (A)
#else
#define OSandPlatformPRESET(A)
#ifdef __cplusplus
#define OSandPlatformGLOBAL extern "C"
#else
#define OSandPlatformGLOBAL extern
#endif	/*__cplusplus*/
#endif				/*GLOBAL_OSandPlatform */

#ifdef __cplusplus
extern "C" {
#endif


/// \addtogroup lowlevel
/// @{
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-sign"
#include <timers.h>
#pragma GCC diagnostic pop

#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <strings.h>
#include <limits.h>


//#define CEXCEPTION_USE_CONFIG_FILE
//#include "CException.h"

#ifdef __cplusplus
  extern "C" {
#endif

//Chip driver status return values
    typedef int32_t ChipDriverStatus_t;
#define SUCCESS 0
#define FAILURE 1
#define DO_NOT_TX -1
#define SATURATED 101

#ifndef RELEASE
#define ASSERT(EXPR) assert(EXPR)
#else
#define ASSERT(EXPR)
#endif

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

//Trigger a HARD FAULT by writing to a FLASH ROM location
//#define TRIG_HARDFAULT *((volatile int*)(0x14017644))=0

//We'll define this later
    typedef void BaseSequentialStream;

// HACK HACK HACK
// TEMPORARY HACK TO MAKE PRINTF WORK WITHIN FREERTOS TASKS
    char pbuf[1024];
    extern int myprintf(const char *format, ...);
    extern int mysprintf(char *out, const char *format, ...);
    extern int mysnprintf(char *buf, unsigned int count,
			  const char *format, ...);

#ifdef RELEASE
#define DPRINTF(...)
#define FDPRINTF(FILE,...)
#define dprintf(...)
#else
#define DPRINTF(...) myprintf(__VA_ARGS__)
#define FDPRINTF(FILE,...) myprintf(__VA_ARGS__)
#define dprintf(...) myprintf(__VA_ARGS__)
#endif


#define delayms(x) do { if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) { \
    vTaskDelay((x)/portTICK_RATE_MS);                                   \
  } else { \
    MSleep(x); \
  }  } while(0)

#ifdef __cplusplus
  }
#endif
  typedef struct task_info {
    uint32_t ExceptionTaskID;
  } task_info_t;

  OSandPlatformGLOBAL uint32_t numRegisteredTasks;	//main.c


#ifdef __cplusplus
}
#endif
extern char __StackTop;
extern char __StackLimit;
extern char __heap_start;
extern char __heap_end;
#endif				//OSandPlatform_INCLUDED

/// @}
