

#define GLOBAL_DEBUG_SHELL
#include "debug_shell.h"
#undef GLOBAL_DEBUG_SHELL

#include "OSandPlatform.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wuninitialized"

extern int _read(int file __attribute__((unused)), char *ptr, int len);
extern int _write(int file __attribute__((unused)), char *ptr, int len);

uint32_t cmd_term_get (char *buf, uint32_t maxlen) 
{
  uint32_t j=0;
  char rxbyte=0;

  do {
    if (j >= maxlen-1) {
      return(j);
    }

    _read(0, &rxbyte, 1);

    //input cr -> cr - lf translation
    if (rxbyte == 0x0d) { //cr input?
      _write(0,&rxbyte,1); //echo
      *(buf+j) = rxbyte; j++;
      rxbyte=0x0a;
      _write(0,&rxbyte,1); //echo
      *(buf+j) = rxbyte;
      *(buf+j+1)=0;
      return(j+1);
    }

    //Backspace or DEL?
    if (rxbyte == 0x08 || rxbyte == 0x7f) {
      _write(0,"\b \b",3);
      j--;
      continue;
    }
 
    _write(0,&rxbyte,1); //echo
    *(buf+j) = rxbyte;
    j++;
  } while(1);  
  return(j);
}

int cmd_exit(int argc, char **argv)
{
   strcpy(currentContext, "");
   return (0);
}

int cmd_help(int argc, char **argv)
{
   int cmdidx = 0;
   int hit = 0;
   char *command;
   int tableix = 0;
   dispatchEntry *table;

   if (argc == 1) {                // generic help
      while (dispatchTableTable[tableix] != 0) {
         table = dispatchTableTable[tableix];
         cmdidx = 0;
         while ((table[cmdidx].cmd != 0)) {
            if ((0 == strcmp(currentContext, table[cmdidx].context))
                || (table[cmdidx].context)[0] == '*') {
               myprintf("%15s ", table[cmdidx].cmd);
               if (table[cmdidx].shortHelp) {
                  myprintf("%s", table[cmdidx].shortHelp);
               }
               myprintf("\r\n");
            }
            cmdidx++;
         }
         tableix++;
      }
   } else {                        //specific command help
      command = argv[1];
      while (dispatchTableTable[tableix] != 0) {
         table = dispatchTableTable[tableix];
         cmdidx = 0;
         while (table[cmdidx].cmd != 0) {
            if ((0 == strcmp(command, table[cmdidx].cmd)) &&
                ((0 == strcmp(currentContext, table[cmdidx].context))
                 || (table[cmdidx].context)[0] == '*')) {
               hit = 1;
               myprintf("%15s ", command);
               if (table[cmdidx].shortHelp) {
                  myprintf("%s", (table[cmdidx].shortHelp));
               }
               myprintf("\r\n");
               if (table[cmdidx].helpfunc) {        //Execute help function
                  (*(table[cmdidx].helpfunc)) ();
               }
               break;
            }
            cmdidx++;
         }
         tableix++;
      }
      if (!hit) {
         myprintf("No command named: %s\r\n", command);
         return (1);
      }
   }
   return (0);
}


// Main entrypoint. This is the debug command processor. It tokenizes input, looks up commands, and 
// dispatches.
static char *args[DEBUG_SHELL_MAXARGS];
void cmdParse(char *p_cmd)
{
   char *cmdstr;
   signed short jj, kk, argi, argstart;
   signed short wholeLength;
   int cmdidx, tableix;
   int rval = 0;
   char hit = 0;
   dispatchEntry *table;
   int len = 0;

   cmdErr=0;
   cmdstr = p_cmd;
   bzero(args, sizeof(char *) * DEBUG_SHELL_MAXARGS);

   //Now do a grunt tokenize
   argi = 0;
   argstart = (-1);
   wholeLength =
       (strlen(cmdstr) ==
        COMMAND_SIZE) ? COMMAND_SIZE - 1 : strlen(cmdstr);
   for (jj = 0; jj < wholeLength; jj++) {
      switch (cmdstr[jj]) {
      case '"':
         //swallow a double quoted string
         len = strlen(cmdstr);
         for (kk = jj + 1; kk < len; kk++) {
            if (cmdstr[kk] == '"')
               break;
         }
         if (cmdstr[kk] != '"') {        //ran off the end
            myprintf("Command parse error, no matching \"\n");
            goto ERR_RET;
         }
         cmdstr[kk] = 0;
         args[argi++] = cmdstr + jj + 1;
         jj = kk + 1;                //advance pointer
         break;
      case '\'':
         //swallow a single quoted string
         len = strlen(cmdstr);
         for (kk = jj + 1; kk < len; kk++) {
            if (cmdstr[kk] == '\'')
               break;
         }
         if (cmdstr[kk] != '\'') {        //ran off the end
            myprintf("Command parse error, no matching \'\n");
            goto ERR_RET;
         }
         cmdstr[kk] = 0;
         args[argi++] = cmdstr + jj + 1;
         jj = kk + 1;                //advance pointer 
         break;
      case ' ':
      case '\t':
        //Handle delimiter
         if (argstart >= 0) {        //accumulating an arg?
            cmdstr[jj] = 0;        //terminate it.
            args[argi++] = cmdstr + argstart;
            argstart = (-1);
         }
         break;
      default:
         if (argstart < 0)
            argstart = jj;
      }
   }
   //Are we still accumulating an arg?
   if (argstart >= 0) {
      cmdstr[jj] = 0;
      args[argi++] = cmdstr + argstart;
   }

   //Now ... dispatch the command
   //We take the FIRST hit in the command dispatch table
   //So, scan the instrument specific table first, THEN the common table
   if ((!strlen(args[0])) || (unsigned char) (args[0][0]) == 0xb0)
      return;

   tableix = 0;
   while (dispatchTableTable[tableix] != 0) {
      table = dispatchTableTable[tableix];
      cmdidx = 0;
      while (table[cmdidx].cmd != 0) {
         if ((0 == strcmp(args[0], table[cmdidx].cmd)) &&
             ((0 == strcmp(currentContext, table[cmdidx].context))
              || (table[cmdidx].context)[0] == '*')) {
            hit = 1;
            if (table[cmdidx].cmdp) {
               rval = (*(table[cmdidx].cmdp)) (argi, args);
               cmdErr=rval;
            }
            break;
         }
         cmdidx++;
      }
      tableix++;
   }
   if (!hit) {
      myprintf("Command not recognized.\r\n");
      cmdErr=0xfe;
   }
   return;

 ERR_RET:
   cmdErr=0xff;
   return;
}

int cmd_read(int argc, char **argv)
{
    uint32_t a;
    uint8_t val8;
    uint16_t val16;
    uint32_t val32;
    
    a=strtoul(argv[1],NULL,0);
    if (strcmp(argv[0],"r8") == 0) {
      val8=*((volatile uint8_t *)a);
      myprintf("0x%02x\n",val8);
    } else if (strcmp(argv[0],"r16") == 0) {
      val16=*((volatile uint16_t *)a);
      myprintf("0x%04x\n",val16);
    } else if (strcmp(argv[0],"r32") == 0) {
      val32=*((volatile uint32_t *)a);
      myprintf("0x%08x\n",(unsigned int)val32);
    } 
   return (0);
}

int cmd_write(int argc, char **argv)
{
    uint32_t a;
    uint32_t val32;

    a=strtoul(argv[1],NULL,0);
    val32=strtoul(argv[2],NULL,0);

    if (strcmp(argv[0],"w8") == 0) {
      *((volatile uint8_t *)a)=(uint8_t)val32;
    } else if (strcmp(argv[0],"w16") == 0) {
      *((volatile uint16_t *)a)=(uint16_t)val32;
    } else if (strcmp(argv[0],"w32") == 0) {
      *((volatile uint32_t *)a)=(uint32_t)val32;
    } 
    return(0);
}

int cmd_stack(int argc, char **argv)
{
  char *outbuf;
  outbuf=(char *)malloc(2048);
  if (outbuf) {
    myprintf("Name                  State   Priority Stack   Num\n");
    myprintf("------------------    -----   -------- -----   ---\n");
    vTaskList(outbuf);
    myprintf("%s\n", outbuf);  
    free(outbuf);
  }
  return (0);
}

int cmd_taskstats(int argc, char **argv)
{
  char *outbuf;
  outbuf=(char *)malloc(2048);
  if (outbuf) {
    myprintf("Name                    Run (us)       Percent\n");
    myprintf("------------------      ----------     -------\n");
    vTaskGetRunTimeStats(outbuf);
    myprintf("%s\n",outbuf);
    free(outbuf);
  }
  return(0);
}

extern char __heap_start;	/* Setup in our linker script */
extern char __heap_end;
int cmd_mallocstats(int argc, char **argv)
{
  struct mallinfo current;
  current=mallinfo();
  myprintf("Heap memory currently in use: %d bytes\n", current.uordblks);
  myprintf("Total free space: %d bytes\n",current.fordblks);
  myprintf("Heap start: 0x%08x, Heap end: 0x%08x, size: %d\n",(unsigned int)&__heap_start,
           (unsigned int)&__heap_end,
           (int)(&__heap_end - &__heap_start) );

#if 0  
  int tot=0;
  for (;;) {
    myprintf("Allocating 1k bytes: ");
    void *leak=malloc(1024);
    myprintf(leak==0 ? "FAIL\n" : "OK\n");
    if (leak==0) break;
    tot++;
  }
  printf("Total: %d\n",tot);
#endif  
  return(0);
}

extern void selfReset(void);
int cmd_reboot(int argc, char **argv)
{
  scb_reset_system();
  return(0);
}

int cmd_identify(int argc, char **argv)
{
  myprintf("STM32F4Discovery running FreeRTOS\n");
  return(0);
}

//Our common-to-all-instruments dispatch table

/* *INDENT-OFF* */
dispatchEntry commonCommands[] = {
//Context, Command,        ShortHelp,                                          command proc,  help proc
    {"", "i",             "                      Identify the board and build",     cmd_identify, NULL},
    {"", "r8",            "Addr                  read one byte from Addr", cmd_read, NULL},
    {"", "r16",           "Addr                  read one short from Addr", cmd_read, NULL},
    {"", "r32",           "Addr                  read one word from Addr", cmd_read, NULL},
    {"", "w8",            "Addr DD               write byte DD to Addr", cmd_write, NULL},
    {"", "w16",           "Addr DDDD             write short DDDD to Addr", cmd_write, NULL},
    {"", "w32",           "Addr DDDDDDDD         write word DDDDDDDD to Addr", cmd_write, NULL},
    {"", "stack",         "                      Dump stack stats.", cmd_stack, NULL},
    {"", "tasks",         "                      Dump task stats.", cmd_taskstats, NULL},
    {"", "heap",          "                      Dump heap stats.", cmd_mallocstats, NULL},
    {"", "reboot",        "                      Reboot.", cmd_reboot, NULL},
    {"*", "exit",         "                      Exit the current context (no-op if at top)", cmd_exit, NULL},                      
    {"*", "help",         "[cmd]                 This help, or help on a specific command", cmd_help, NULL},                      
    {"*", "h",            "[cmd]                 This help, or help on a specific command", cmd_help, NULL},                      
    {"*", "?",            "[cmd]                 This help, or help on a specific command", cmd_help, NULL},                      

//example context
#if 0
    { "", "TC",            "                      Switch to TC test context", cmd_tc, NULL},
    { "TC", "x",           "                      same as i", cmd_identify, NULL},
    { "TC", "TC",          "                      leave", cmd_tc, NULL},
#endif
//LAST ENTRY
    {NULL, NULL, NULL, NULL, NULL}
};

#pragma GCC diagnostic pop

// Debug command shell thread
//#if ENABLE_DEBUG_TASK
portTASK_FUNCTION(vDebugShell, pvParameters) {
  (void)pvParameters;
  char *line;
  uint32_t len;

  line = (char *)malloc(1024);
  if (! line) {
    myprintf("!\n");
    return;
  }
  //vTaskSetApplicationTaskTag( NULL, ( void * ) 2 ); 
  strcpy(currentContext, "");

  while (1) {
    myprintf("%s> ",currentContext);
    len=cmd_term_get(line, 1023);
    line[len-2]=0; //strip off cr-lf
    cmdParse(line);
    vTaskDelay(100/portTICK_RATE_MS);
  }
}
//#endif
