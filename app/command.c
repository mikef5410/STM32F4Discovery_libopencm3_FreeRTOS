#include "OSandPlatform.h"
#include "debug_shell.h"

#ifndef COUNTOF
#define COUNTOF(A) (sizeof(A)/sizeof(A[0]))
#endif


#ifdef BUILD_INFO
static int cmd_buildInfo(int argc, char **argv)
{
  (void) argc;
  (void) argv;
  myprintf("%s\r\n", build_info);
  return (0);
}
#endif

#ifdef BUILD_SHA1
static int cmd_build_sha1(int argc, char **argv)
{
  (void) argc;
  (void) argv;
  myprintf("%s\r\n", build_sha1);
  return (0);
}
#endif

static int cmd_hardfault(int argc, char **argv)
{
  (void) argc;
  (void) argv;

  TRIG_HARDFAULT; //trigger a hard fault
  return(0);
}

static int cmd_timer(int argc, char **argv)
{
  (void) argc;
  (void) argv;

  for (int j=0; j<3; j++) {
    gpio_toggle(GPIOC, GPIO7);
    volatile uint64_t start=hiresTimer_getTime();
    vTaskDelay(10/portTICK_RATE_MS);
    volatile int64_t delta = hiresTimer_getTime() - start;
    //gpio_clear(GPIOC, GPIO7);
    myprintf(" 10 ms = %d us \n", (int)tics2us(delta));
  }
  return(0);
}


dispatchEntry mainCommands[] = {
//Context, Command,        ShortHelp,                                          command proc,  help proc
#ifdef BUILD_INFO
  {"","buildInfo",        "                      Show build info", cmd_buildInfo, NULL},
#endif
#ifdef BUILD_SHA1
  {"","build_sha1",       "                      Show SHA1 info", cmd_build_sha1, NULL},
#endif
  {"","hardfault",        "                      Cause a hard fault", cmd_hardfault, NULL},
  {"","timer",            "                      Test the hires timer", cmd_timer, NULL},
    //LAST ENTRY
  {NULL, NULL, NULL, NULL, NULL}
};


// Add your command table here ... most general last
dispatchEntry* dispatchTableTable[] = {
    &(mainCommands[0]),     //command.c
    &(commonCommands[0]),  //debug_shell.c
    NULL
};
