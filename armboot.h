/* _ARMBOOT_H_ */
#ifndef	_ARMBOOT_H_  
#define	_ARMBOOT_H_
#include <stdarg.h>

#define NULL	0
#define CFG_CBSIZE		256
#define CONFIG_NR_DRAM_BANKS	1

/* the PWM TImer 4 uses a counter of 10000 for 10 ms, so we need */
/* it to wrap 100 times (total 1000000) to get 1 sec. */
#define CFG_HZ                  1000000

extern char *tftp_filename;
extern unsigned long load_addr;
extern volatile char flag_timer;


typedef unsigned short ushort;
typedef unsigned char uchar;

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

typedef unsigned int uint;
typedef unsigned long ulong;

typedef unsigned short __u16;
typedef unsigned long size_t;

typedef struct bd_info {
    int         bi_baudrate;    /* serial console baudrate */
    unsigned long   bi_ip_addr; /* IP Address */
    unsigned char   bi_enetaddr[6]; /* Ethernet adress */
/*    env_t          *bi_env; */
    ulong           bi_arch_number; /* unique id for this board */
    ulong           bi_boot_params; /* where this board expects params */
    struct              /* RAM configuration */
    {
    ulong start;
    ulong size;
    }           bi_dram[CONFIG_NR_DRAM_BANKS];
} bd_t;


/* $(CPU)/serial.c */
void    serial_init (bd_t *);
void    serial_setbrg   (bd_t *, int);
void    serial_putc (const char);
void    serial_puts (const char *);
int serial_getc (void);
int serial_tstc (void);


/* arm/string.c */
char *	strcpy		(char * dest,const char *src);
char *	strncpy		(char * dest,const char *src, size_t count);
size_t	strlen		(const char *);
size_t	strnlen		(const char * s, size_t count);
int	strncmp		(const char * cs, const char * ct, size_t count);
int	strcmp		(const char * cs, const char * ct);
void *	memcpy		(void * dest, const void *src, size_t count);
int	memcmp		(const void * dest, const void *src, size_t count);
void *	memset		(void * s, char c, size_t count);
void *	memmove		(void * dest, const void *src, size_t count);
char *	strchr		(const char * s, int c);

/* arm/vsprintf.c */
ulong	simple_strtoul	(const char *cp,char **endp,unsigned int base);
long	simple_strtol	(const char *cp,char **endp,unsigned int base);
void	panic		(const char *fmt, ...);
int	sprintf		(char * buf, const char *fmt, ...);
int 	vsprintf	(char *buf, const char *fmt, va_list args);

/* board.c */
void nand_read_from_zero(int *to_ram, int length);
void nand_prog_to_zero(int *from_ram, int length);
void arp_req(void);
void tftp_req(char *fname, long raddr);
int readline(void);

void udelay(unsigned long usec);
/* cpu/.../interrupt.c */
ulong get_timer(ulong base);

/*
 * STDIO based functions (can always be used)
 */

/* serial stuff */
void	serial_printf (const char *fmt, ...);

/* stdin */
int	getc(void);
int	tstc(void);

/* stdout */
void	putc(const char c);
void	puts(const char *s);
void	printf(const char *fmt, ...);

/* stderr */
#define eputc(c)		fputc(stderr, c)
#define eputs(s)		fputs(stderr, s)
#define eprintf(fmt,args...)	fprintf(stderr,fmt ,##args)


/* Byte swapping stuff */
#define SWAP16(x)	((((x) & 0xff) << 8) | ((x) >> 8))
#define SWAP16c(x)	((((x) & 0xff) << 8) | ((x) >> 8))
#define SWAP32(x)       ( \
	                (((x) >> 24) & 0x000000ff) | \
        		(((x) >>  8) & 0x0000ff00) | \
		        (((x) <<  8) & 0x00ff0000) | \
		        (((x) << 24) & 0xff000000) )


#endif	/* _ARMBOOT_H_ */
