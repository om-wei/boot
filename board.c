#include "s3c2410.h"
#include "armboot.h"
#include "net/net.h"
#include "Flash.h"

/*Timer input clock Frequency = PCLK / {prescaler value} / {divider value}
{prescaler value} = 0~255	{divider value} = 2, 4, 8, 16 */
/* for 10 ms clock period @ 50 MHz with 4 bit divider = 1/2 (default) */
/* and prescaler = 25 */
#define TIMER_LOAD_VAL 10000
#define READ_TIMER (rTCNTO4 & 0xffff)

extern void main_loop(void);
static char * delete_char (char *p, int *colp, int *np);
//void relocate_ram(void);
//extern void reset_cpu(ulong addr);

static unsigned long timestamp; 
static unsigned long lastdec; 
bd_t bd;
volatile char flag_timer;

static char erase_seq[] = "\b \b";		/* erase sequence	*/
char        console_buffer[CFG_CBSIZE];		/* console I/O buffer	*/
unsigned char net_send_buff[2048];
unsigned char net_recive_buff[2048];


/* Nand Flash */
extern int *Ram_pr;

int read_from_host(void)
{
	return *(Ram_pr++);
}

void write_to_host(int word)
{
	*Ram_pr++ = word;
}

void nand_read_from_zero(int *to_ram, int length)
{
	int b = 0, p = 0;

	//b = (int)from_nand / NAND_BLOCK_SIZE;
	//p = ((int)from_nand % NAND_BLOCK_SIZE) / NAND_PAGE_SIZE;

	Ram_pr = to_ram;
	while (length > 0) {
		nand_read_page(b, p);
		p++;
		if (p == NAND_BLOCK_PAGE_NUM){
			p = 0;
			b++;
		}
		length = length - NAND_PAGE_SIZE;
	}
}

void nand_prog_to_zero(int *from_ram, int length)
{
	int b = 0, p = 0;
	int i;

	//b = (int)to_nand / NAND_BLOCK_SIZE;
	//p = ((int)to_nand % NAND_BLOCK_SIZE) / NAND_PAGE_SIZE;
	Ram_pr = from_ram;

	i = nand_erase_block(b);
	if (i == 1) {
		puts("nand_erase not success\n");
	}
	while (length > 0) {
		i = nand_program_page(b, p);
		if (i == 1) {
			puts("nand_program not success\n");
		}
		puts("nand_program\n");
		p++;
		if (p == NAND_BLOCK_PAGE_NUM){
			p = 0;
			b++;
			nand_erase_block(b);
			puts("nand_erase\n");
		}
		length = length - NAND_PAGE_SIZE;
	}
}

/* relocate_ram  function run in 4k Stepping-Stone SRAM, 
   so do _not_ call anther thing which beyond frist 4K in boot.bin */
void relocate_ram(void)
{
	nand_init();
	nand_read_from_zero((int*)0x30000000, 32 * 1024);
}

void phex(int hex, char units) 
{
	char i;
	putc('0');
	putc('x');
	do{
		i = ((hex >> (--units * 4)) & 0xf);
		(i < 10) ? putc('0' + i) : putc('A' + (i - 10));
	}while(units);
	putc('\n');
}

/* serial */
/*
void serial_puts(const char *s)
{
    while (*s) {
        serial_putc (*s++);
    }
}
*/

/*
 * timer without interrupts
 */

void reset_timer_masked(void)
{
    /* reset time */
    lastdec = READ_TIMER;
    timestamp = 0;
}

void set_timer (ulong t)
{
    timestamp = t;
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

ulong get_timer (ulong base)
{
    return get_timer_masked() - base;
}

/*
void udelay(unsigned long usec)
{
    ulong tmo;

    tmo = usec / 1000;
    tmo *= CFG_HZ;
    tmo /= 1000;

    tmo += get_timer(0);

    while(get_timer_masked() < tmo)
     //NOP;
}
*/

void udelay(unsigned long usec)
{
    reset_timer_masked();

    while(get_timer_masked() < usec)
      /*NOP*/;
}

void timer_init(void)
{
	/* use PWM Timer 4 because it has no output */
	/* prescaler for Timer 4 is 24+1 */
	rTCFG0 = (24<<8);
	/* load value for 10 ms timeout, assumes PCLK is 50 MHz !! */
	rTCNTB4 = TIMER_LOAD_VAL;
	/* auto load, manual update of Timer 4 */
	rTCON = 0x600000;
	/* auto load, start Timer 4 */
	rTCON = 0x500000;
	timestamp = 0;
	// TIMER4 interrupt enable
	// rINTMSK &= ~BIT_TIMER4;
}

void start_armboot(void) 
{
	// LEDs config output
	rGPBCON = 0x00015400; 
	rGPBDAT = (0b1010 << 5);

	// UART config GPH2-5 as RX, TX port
	rGPHCON = 0x00000AA0;
	// disable pull up fuction
	rGPHUP = 0x3c;
	serial_init(&bd);
	puts("enter in start_armboot\n");
	
	// timmer init must before eth_int which use udelay
	timer_init();

	/* network init */
	NetRxPackets[0] = net_recive_buff;
	NetTxPacket = net_send_buff;
	eth_init(&bd);
	
	/* NAND Flash */
	nand_init();
	puts("nand flash id: ");
	phex(nand_read_id(), 8);

	// TIMER4 interrupt enable
	rINTMSK &= ~BIT_TIMER4;

	main_loop();
}

void do_irq(void) 
{
	static unsigned int cnt = 0;
//	static unsigned int kflag = 0; 

	if (rINTPND == (BIT_TIMER4)) {
		// clear PND by set 1, SRCPND first rINTPND later
		rSRCPND = (BIT_TIMER4);
		rINTPND = (BIT_TIMER4);
		// flag used for eth_rx 
		// for 1 sec
		flag_timer = 1;
		if (flag_timer) {
			flag_timer = 0;
			eth_rx();
		}
		if ((cnt++ % 100) == 0) {
			// led 
			rGPBDAT ^= (0b1111 << 5);
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

