// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "FreeRTOS.h"

#if ENABLE_RTOS_XSCOPE_TRACE == 1
#include <stdarg.h>
#include <ctype.h>
#include <string.h>

static void reverse_array(char buf[], unsigned size)
{
  int begin = 0;
  int end = size - 1;
  int tmp;
  for (;begin < end; begin++,end--) {
    tmp = buf[begin];
    buf[begin] = buf[end];
    buf[end] = tmp;
  }
}

static int itoa(unsigned n, char *buf, unsigned base, int fill)
{
  static const char digits[] = "0123456789ABCDEF";
  unsigned i = 0;

  if (n == 0)
    fill += 1;

  while (n > 0) {
    unsigned next = n / base;
    unsigned cur  = n % base;
    buf[i] = digits[cur];
    i += 1;
    fill--;
    n = next;
  }
  for (;fill > 0; fill--) {
    buf[i] = '0';
    i++;
  }
  reverse_array(buf, i);
  return i;
}

#define MAX_XSCOPE_INT_STRING_SIZE 10

void traceFreeRTOS_to_xscope(char* fmt, ...)
{
	int intArg;
	unsigned int uintArg;
	char * strArg;
	va_list args;

	unsigned char buf[ xcoretraceconfigXSCOPE_TRACE_BUFFER ];
	unsigned char *end = &buf[ xcoretraceconfigXSCOPE_TRACE_BUFFER - 1 - MAX_XSCOPE_INT_STRING_SIZE ];

	va_start( args, fmt );
	unsigned char *p = buf;

	while( *fmt )
	{
		if( p > end )
		{
			break;
		}
		switch( *fmt )
	    {
	    	case '%':
				fmt++;
	    		switch( tolower( *( fmt ) ) )
#if ( xcoretraceconfigXSCOPE_TRACE_RAW_BYTES == 0 )
	    		{
	    	    	case 'd':
						intArg = va_arg(args, int);
						if (intArg < 0) {
							*p++ = '-';
							intArg = -intArg;
						}
						p += itoa(intArg, p, 10, 0);
						break;
	    	    	case 'u':
						uintArg = va_arg(args, int);
						p += itoa(uintArg, p, 10, 0);
						break;
	    	    	case 'p':
	    	    	case 'x':
						uintArg = va_arg(args, int);
						p += itoa(uintArg, p, 16, 0);
						break;
	    	    	case 'c':
						intArg = va_arg(args, int);
						*p++ = intArg;
						break;
	    	    	case 's':
				        strArg = va_arg(args, char *);
				        int len = strlen(strArg);
				        if (len > (end - buf)) {
				        	p = buf;
				        	break;
				        }
				        if (len > (end - buf))
				        	len = end - buf;
				        memcpy(p, strArg, len);
				        p += len;
				        break;
					default:
						break;
				}
#else	/* xcoretraceconfigXSCOPE_TRACE_RAW_BYTES == 1 */
	    		{
	    	    	case 'd':
	    	    	case 'u':
	    	    	case 'p':
	    	    	case 'x':
						intArg = va_arg(args, int);
						*p++ = ( intArg >> 24 ) & 0xFF;
						*p++ = ( intArg >> 16 ) & 0xFF;
						*p++ = ( intArg >>  8 ) & 0xFF;
						*p++ = ( intArg >>  0 ) & 0xFF;
						break;
	    	    	case 'c':
						intArg = va_arg(args, int);
						*p++ = ( intArg >>  0 ) & 0xFF;
						break;
	    	    	case 's':
				        strArg = va_arg(args, char *);
				        int len = strlen(strArg);
				        if (len > (end - buf)) {
				        	p = buf;
				        	break;
				        }
				        if (len > (end - buf))
				        	len = end - buf;
				        memcpy(p, strArg, len);
				        p += len;
				        break;
					default:
						break;
				}
#endif /* xcoretraceconfigXSCOPE_TRACE_RAW_BYTES */
	    		break;

			default:
				/* Ignore spaces in the fmt string */
				if( *fmt != 0x20 )
				{
					*p++ = *fmt;
				}
	    }
		fmt++;
	}
	xscope_core_bytes( FREERTOS_TRACE, p - buf , buf );

	va_end(args);
}

#endif /* xcoretraceconfigASCII == 1 */
