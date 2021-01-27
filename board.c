#include "s3c2410.h"
#include "armboot.h"
#include "net/arp.h"
#include "net/net.h"
#include "Flash.h"

/* for 10 ms clock period @ 50 MHz with 4 bit divider = 1/2 (default) */
/* and prescaler = 24+1 */
#define TIMER_LOAD_VAL 10000
#define READ_TIMER (rTCNTO4 & 0xffff)

static char * delete_char (char *p, int *colp, int *np);
int readline ();
extern void reset_cpu(ulong addr);

static unsigned long timestamp; 
static unsigned long lastdec; 

static char erase_seq[] = "\b \b";		/* erase sequence	*/
bd_t bd;
char        console_buffer[CFG_CBSIZE];		/* console I/O buffer	*/
unsigned char send_buff[2000];
unsigned char recive_buff[2000];

char arq_d[42]={
0xff,0xff, 0xff,0xff, 0xff,0xff, 0xf0, 0x99, 0xbf, 0x5d, 0x45, 0xbc,
0x08,0x06, 0x01,0x00, 0x00,0x08, 0x04,0x06, 0x01,0x00, 
0xf0, 0x99, 0xbf, 0x5d, 0x45, 0xbc,
0xa8,0xc0, 0x02,0x01, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0xa8,0xc0, 0x01,0x01
};

void phex(int hex, char units) 
{
	char i;
	putc('0');
	putc('x');
	do{
		i = ((hex >> --units * 4) & 0xf);
		(i < 10) ? putc('0'+i) : putc('A'+i-10);
	}while(units);
}

void serial_puts(const char *s)
{
    while (*s) {
        serial_putc (*s++);
    }
}

void reset_timer_masked(void)
{
    /* reset time */
    lastdec = READ_TIMER;
    timestamp = 0;
}

ulong get_timer_masked(void)
{
	ulong now = READ_TIMER;

	if (lastdec >= now) {
	/* normal mode */
		timestamp += lastdec - now;
	} else {
		/* we have an overflow ... */
		timestamp += lastdec + TIMER_LOAD_VAL - now;
	}
	lastdec = now;

	return timestamp;
}

void udelay(unsigned long usec)
{
    reset_timer_masked();

    while(get_timer_masked() < usec)
      /*NOP*/;
}

void timer_init(void)
{

	/* use PWM Timer 4 because it has no output */
    /* prescaler for Timer 4 is 16 */
    rTCFG0 = (24<<8);
    /* load value for 10 ms timeout, assumes PCLK is 50 MHz !! */
    rTCNTB4 = TIMER_LOAD_VAL;
    /* auto load, manual update of Timer 4 */
    rTCON = 0x600000;
    /* auto load, start Timer 4 */
    rTCON = 0x500000;
    timestamp = 0;
	// TIMER4 interrupt enable
	rINTMSK &= ~BIT_TIMER4;
}

void arp_req(void)
{
	printf("arp_req command\n");

	NetOurIP = 0xc0a80102;
	NetOurGatewayIP = 0;
	NetServerIP = 0xc0a80101;
	NetTxPacket = send_buff;
	// eth_send(arq_d, 42);
	ArpRequest();

	for(;;) {
		eth_rx();
	}
}

void start_armboot(void) 
{
	int len;

	// LEDs config output
	rGPBCON = 0x00015400; 
	rGPBDAT = (0b1010 << 5);

/*
	// KEYs config EINT 0 1 2 4
	rGPFCON = 0x0000022A; 
	// EINT4 enable interrupt
	rEINTMASK &= ~(1<<4);
	// EINT4 clear pend flag
	rEINTPEND |= (1<<4);
	// EINT4_7 enable
	rINTMSK &= ~BIT_EINT4_7;

	timer_init();

*/
	// UART config GPH2-5 as RX, TX port
	rGPHCON = 0x00000AA0;
	// disable pull up fuction
	rGPHUP = 0x3c;
	serial_init(&bd);

	NetRxPackets[0] = recive_buff;

	// eth_init(&bd);
//	nand_init();
	puts("nand flash id: ");
	phex(nand_read_id(), 8);
	putc('\n');
	puts("1. arp req\n");
	puts("2. eth_rx\n");
	puts("3. nand read\n");
	puts("4. nand prog\n");

	while (1) {
		len =  readline();
		if (len > 0) {
			switch(console_buffer[0]){
			case '1':
				arp_req();
				break;
			case '2':
				eth_rx();
				break;
			case '3':
			//	nand_read((int*)0, (int*)0x30000000, 20 * 1024);
				puts("nand flash read\n");
				break;
			case '4':
			//	nand_prog((int*)0x30000000, (int*)0, 20 * 1024);
				puts("nand flash program\n");
				break;
			}
		}
	}	
}

void do_irq(void) 
{
	static unsigned int cnt = 50;
	static unsigned int kflag = 0; 

	if (rINTPND == (BIT_EINT4_7)) {
		// EINT4 disable
		rEINTMASK |= (1<<4);
		rEINTPEND = (1<<4);
		rSRCPND = (BIT_EINT4_7);
		rINTPND = (BIT_EINT4_7);

		rGPBDAT ^= (0b1111 << 5);
		kflag = 1;
	}

	if (rINTPND == (BIT_TIMER4)) {
		rSRCPND = (BIT_TIMER4);
		rINTPND = (BIT_TIMER4);
		timestamp++;
		// for 1 sec
		if ((timestamp % 100) == 0) {
//			rGPBDAT ^= (0b1111 << 5);
		}
		if (kflag) {
			if (!--cnt){
				cnt = 50;
				kflag = 0;
				rEINTPEND = (1<<4);
				rSRCPND = (BIT_EINT4_7);
				rINTPND = (BIT_EINT4_7);
				// EINT4 enable
				rEINTMASK &= ~(1<<4);
			}
		}
	}
//	reset_cpu(0);
}


/*
 * Prompt for input and read a line.
 * If  CONFIG_BOOT_RETRY_TIME is defined and retry_time >= 0,
 * time out when time goes past endtime (timebase time in ticks).
 * Return:	number of read characters
 *		-1 if break
 *		-2 if timed out
 */
int readline ()
{
	char   *p = console_buffer;
	int	n = 0;				/* buffer index		*/
	char *prompt = "boot:";
	int	plen = 5;		/* prompt length	*/
	int col;
	char	c;

	/* print prompt */
	if (prompt)
		puts (prompt);
	col = plen;

	for (;;) {
#ifdef CONFIG_BOOT_RETRY_TIME
		while (!tstc()) {	/* while no incoming data */
			if (retry_time >= 0 && get_ticks() > endtime)
				return (-2);	/* timed out */
		}
#endif
		c = getc();

		/*
		 * Special character handling
		 */
		switch (c) {
		case '\r':				/* Enter		*/
		case '\n':
			*p = '\0';
			puts ("\r\n");
			return (p - console_buffer);

		case 0x03:				/* ^C - break		*/
			console_buffer[0] = '\0';	/* discard input */
			return (-1);

		case 0x15:				/* ^U - erase line	*/
			while (col > plen) {
				puts (erase_seq);
				--col;
			}
			p = console_buffer;
			n = 0;
			continue;

		case 0x17:				/* ^W - erase word 	*/
			p=delete_char(p, &col, &n);
			while ((n > 0) && (*p != ' ')) {
				p=delete_char(p, &col, &n);
			}
			continue;

		case 0x08:				/* ^H  - backspace	*/
		case 0x7F:				/* DEL - backspace	*/
			p=delete_char(p, &col, &n);
			continue;

		default:
			/*
			 * Must be a normal character then
			 */
			if (n < CFG_CBSIZE-2) {
				++col;		/* echo input		*/
				putc (c);
				*p++ = c;
				++n;
			} else {			/* Buffer full		*/
				putc ('\a');
			}
		}
	}
}

/****************************************************************************/

static char * delete_char (char *p, int *colp, int *np)
{

	if (*np == 0) {
		return (p);
	}

	puts (erase_seq);
	(*colp)--;
	(*np)--;
	p--;
	return (p);
}

