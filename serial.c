/*
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "armboot.h"
//#include "arm920t.h"
#include "s3c2410.h"

#define CONFIG_SERIAL1

unsigned int br[] = {1562, 780, 390, 194, 32, 15};
void serial_setbrg(bd_t *bd, int baudrate)
{
    int i;
    unsigned int reg = 0;

    /* this assumes a PCLK of 50.7 MHz */
    /* value is calculated so : (int)(PCLK/16./baudrate) -1 */
    if (baudrate == 1200)        reg = 2639;
    else if (baudrate == 9600)   reg = 329;
    else if (baudrate == 19200)  reg = 164;
    else if (baudrate == 38400)  reg = 82;
    else if (baudrate == 57600)  reg = 54;
    else if (baudrate == 115200) reg = 27;
    //else hang();

#ifdef CONFIG_SERIAL1
    /* FIFO enable, Tx/Rx FIFO clear */
    rUFCON0 = 0x06;
    rUMCON0 = 0x0;
    /* Normal,No parity,1 stop,8 bit */
    rULCON0 = 0x3;
    /*
     * tx=level,rx=edge,disable timeout int.,enable rx error int.,
     * normal,interrupt or polling
     */
    rUCON0 = 0x05;
    rUBRDIV0 = reg;

    for(i=0;i<100;i++);
#elif CONFIG_SERIAL2
    /* FIFO enable, Tx/Rx FIFO clear */
    rUFCON1 = 0x06;
    rUMCON1 = 0x0;
    /* Normal,No parity,1 stop,8 bit */
    rULCON1 = 0x3;
    /*
     * tx=level,rx=edge,disable timeout int.,enable rx error int.,
     * normal,interrupt or polling
     */
//    rUCON1 = 0x245;
    rUCON1 = 0x05;
    rUBRDIV1 = reg;

    for(i=0;i<100;i++);
#else
#error "Bad: you didn't configure serial ..."
#endif
}

/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 *
 */
void serial_init(bd_t *bd)
{
/*    const char *baudrate;

    if ((baudrate = getenv(bd, "baudrate")) != 0)
      bd->bi_baudrate = simple_strtoul(baudrate, NULL, 10);

    serial_setbrg(bd, bd->bi_baudrate);
*/
    serial_setbrg(bd, 115200);
}

/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
int serial_getc(void)
{
#ifdef CONFIG_SERIAL1
    while(!(rUTRSTAT0 & 0x1))
    	;

    return rURXH0 & 0xff;
#elif CONFIG_SERIAL2
    while(!(rUTRSTAT1 & 0x1))
    	;

    return rURXH1 & 0xff;
#endif
}


/*
 * Output a single byte to the serial port.
 */
void serial_putc(const char c)
{
#ifdef CONFIG_SERIAL1
    /* wait for room in the tx FIFO on SERIAL1 */
    while(!(rUTRSTAT0 & 0x2))
    	;

    rUTXH0 = c;
#elif CONFIG_SERIAL2
    /* wait for room in the tx FIFO on SERIAL2 */
    while(!(rUTRSTAT1 & 0x2))
    	;

    rUTXH1= c;
#endif

    /* If \n, also do \r */
    if(c == '\n')
      serial_putc('\r');
}		

/*
 * Test whether a character is in the RX buffer
 */
int serial_tstc(void)
{
#ifdef CONFIG_SERIAL1
    return rUTRSTAT0 & 0x1;
#elif CONFIG_SERIAL2
    return rUTRSTAT1 & 0x1;
#endif
}

