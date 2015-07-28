/***********************************************************************/
/*                                                                     */
/*  SYSCALLS.C:  System Calls Remapping                                */
/*  for newlib                                                         */
/*                                                                     */
/***********************************************************************/
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include "OSandPlatform.h"
#include <libopencm3/usb/usbd.h>

#define EOF (-1)
extern usbd_device *CDCACM_dev;
int Board_UARTGetChar(void);
void Board_UARTPutChar(char ch);
void Board_UARTPutSTR(char *str);

static char *heap_ptr = 0;	/* Points to current end of the heap.   */
extern char __heap_start;	/* Setup in our linker script */
extern char __heap_end;
/************************** _sbrk_r *************************************/
/*  Support function.  Adjusts end of heap to provide more memory to    */
/* memory allocator. Simple and dumb with no sanity checks.             */
/*  struct _reent *r    -- re-entrancy structure, used by newlib to     */
/*                      support multiple threads of operation.          */
/*  ptrdiff_t nbytes    -- number of bytes to add.                      */
/*  Returns pointer to start of new heap area.                          */
/*  Note:  This implementation is not thread safe (despite taking a     */
/* _reent structure as a parameter).                                    */
/*  Since _s_r is not used in the current implementation, the following */
/* messages must be suppressed.                                         */
/************************************************************************/
void *_sbrk_r(struct _reent *_s_r
	      __attribute__ ((unused)), ptrdiff_t nbytes)
{
  char *base;			/*  errno should be set to  ENOMEM on error     */
  char *new_heap_ptr = 0;

  if (!heap_ptr) {		/*  Initialize if first time through.           */
    heap_ptr = &__heap_start;
  }
  base = heap_ptr;		/*  Point to end of heap.                       */

  new_heap_ptr = (char *) (((unsigned int) (heap_ptr + nbytes) + 7) & ~7);	//Align to 8-byte boundary
  if (new_heap_ptr > (&__heap_end)) {
    //errno=ENOMEM;
    return ((void *) -1);	/* Failure out of mem, We should set errno to ENOMEM, too */
  }

  heap_ptr = new_heap_ptr;	/*  Increase heap.                              */

  return base;			/*  Return pointer to start of new heap area.   */
}

//Seems that newlib-nano wants an _sbrk ... sheesh
void *_sbrk(ptrdiff_t nbytes)
{
  char *base;			/*  errno should be set to  ENOMEM on error     */
  char *new_heap_ptr = 0;

  if (!heap_ptr) {		/*  Initialize if first time through.           */
    heap_ptr = &__heap_start;
  }
  base = heap_ptr;		/*  Point to end of heap.                       */

  new_heap_ptr = (char *) (((unsigned int) (heap_ptr + nbytes) + 7) & ~7);	//Align to 8-byte boundary
  if (new_heap_ptr > (&__heap_end)) {
    //errno=ENOMEM;
    return ((void *) -1);	/* Failure out of mem, We should set errno to ENOMEM, too */
  }

  heap_ptr = new_heap_ptr;	/*  Increase heap.                              */

  return base;			/*  Return pointer to start of new heap area.   */
}


int _close(int file __attribute__ ((unused)))
{
  return -1;
}

int _fstat(int file __attribute__ ((unused)), struct stat *st)
{
  st->st_mode = S_IFCHR;
  return 0;
}

int _isatty(int file __attribute__ ((unused)))
{
  return 1;
}

int _lseek(int file __attribute__ ((unused)), int ptr
	   __attribute__ ((unused)), int dir __attribute__ ((unused)))
{
  return 0;
}

int _open(const char *name __attribute__ ((unused)), int flags
	  __attribute__ ((unused)), int mode __attribute__ ((unused)))
{
  return -1;
}


extern xQueueHandle UARTinQ;

int _read(int file __attribute__ ((unused)), char *ptr, int len)
{
  int todo;
  int inchar;
  uint8_t rxin;

  if (len == 0)
    return 0;
  if (UARTinQ) {		// Is the FreeRTOS input Queue setup?
    for (todo = 1; todo <= len; todo++) {
      xQueueReceive(UARTinQ, &rxin, portMAX_DELAY);
      *ptr++ = (char) rxin;
    }
  } else {
    for (todo = 1; todo <= len; todo++) {
      inchar = Board_UARTGetChar();
      if (inchar == EOF) {
	return (0);
      }
      *ptr++ = (char) inchar;
    }
  }
  return todo;
}

// Simple ring buffer for _write to use when there's no place
// to put the outgoing string ...
#define BUFSIZ 64
struct ringBuffer {
  char buffer[BUFSIZ];
  uint32_t startPos;
  uint32_t writePos;
  uint32_t size;
} ringBuffer;

void enq(char *ptr, int len)
{
  for (int k=0; k<len; k++) {
    ringBuffer.buffer[(ringBuffer.writePos++) % BUFSIZ]=*(ptr + k);
    ringBuffer.size = (ringBuffer.size + 1) % BUFSIZ;
  }
}

int deq(char *ch)
{
  if (ringBuffer.size) {
    *ch = ringBuffer.buffer[ringBuffer.startPos];
    ringBuffer.size--;
    ringBuffer.startPos = (ringBuffer.startPos + 1) % BUFSIZ;
    return(1);
  } else {
    return(-1);
  }
}

int sendPacket(void)
{
  int rval=0;
  rval = usbd_ep_write_packet(CDCACM_dev, 0x82, ringBuffer.buffer + ringBuffer.startPos, ringBuffer.size);
  if (rval != 0 ) { //Then the packet went out ...
    ringBuffer.startPos=0;
    ringBuffer.writePos=0;
    ringBuffer.size=0;
  }
  return(rval);
}

//Write to CDCACM device ...
#define MAXDELAY 10
int _write(int file __attribute__ ((unused)), char *ptr, int len)
{
  int todo;
  
  //Our max packet size is 64 bytes. We'll chunk into 64 byte pieces
  //and send it out.
  
  //  Chip_UART_Send(DEBUG_UART, ptr, len, BLOCKING);
  //  return len;
  for (todo = 0; todo < len; todo++) {
    if (*(ptr + todo) == '\n') {	// ONLCR output line discipline
      enq("\r",1);
      if (ringBuffer.size==64) { //Did we just fill a packet??
        sendPacket();
      }
    }
    enq((ptr+todo),1);
    if (ringBuffer.size==64) { //Did we just fill a packet??
      sendPacket();
    }
  }
  //Final partial-full write
  sendPacket();
  return len; //We wrote it all
}


/* Gets a character from the UART, returns EOF if no character is ready */
int Board_UARTGetChar(void)
{
  char inch;
  portBASE_TYPE rval;
  if (uxQueueMessagesWaiting(UARTinQ)) {
    rval = xQueueReceive(UARTinQ,(void *)&inch,0);
    if (rval == pdPASS) return(inch);
    if (rval == errQUEUE_EMPTY) return(EOF);
  }
  return(EOF);
}

void Board_UARTPutChar(char val)
{
  char buf[64];
  buf[0]=val;
  while (usbd_ep_write_packet(CDCACM_dev, 0x82, buf, 1) == 0) ;
}


/* Sends a character on the UART */
//void Board_UARTPutChar(char ch)
//{

//  ch++;
  //while (Chip_UART_SendByte(DEBUG_UART, (uint8_t) ch) == ERROR) {}
//}

/* Outputs a string on the debug UART */
void Board_UARTPutSTR(char *str)
{
  _write(0, str, strlen(str));
}
