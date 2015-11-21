/* Open Sensor Platform Project
 * https://github.com/sensorplatforms/open-sensor-platform
 *
 * Copyright (C) 2015 Audience Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include "Common.h"

#pragma import(__use_no_semihosting_swi)

#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

int ser_putchar (int c);
int ser_getchar (void);

struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
FILE __stdin;


PUTCHAR_PROTOTYPE
{
    return (ser_putchar(ch));
}

int fgetc (FILE *f)         { return (ser_getchar()); }


int ferror(FILE *f) {
    /* Your implementation of ferror */
    return EOF;
}


void _ttywrch(int ch)       { ser_putchar(ch); }


void _sys_exit(int return_code) {
label:  goto label;  /* endless loop */
}

/*----------------------------------------------------------------------------
 Write character to Serial Port (blocking)
 *----------------------------------------------------------------------------*/
int ser_putchar (int byte) {

    while (!(DBG_IF_UART->STAT & UART_STAT_TXRDY));
    DBG_IF_UART->TXDAT = (uint32_t)(byte & 0xFF);

    return byte;
}


/*----------------------------------------------------------------------------
 Read character from Serial Port   (blocking read)
 *----------------------------------------------------------------------------*/
int ser_getchar (void) {

    while (!(DBG_IF_UART->STAT & UART_STAT_RXRDY));

    return ((int)(DBG_IF_UART->RXDAT & 0x000000FF));
}
