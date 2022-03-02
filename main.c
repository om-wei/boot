#include <ctype.h>
//#include <stdlib.h>
#include "armboot.h"
#include "net/net.h"
#include "net/tftp.h"
#include "net/arp.h"


#define PARSE_SIZE 	4
#define CMD_NUM		4

extern int atoi(char *);
extern char console_buffer[CFG_CBSIZE];		/* console I/O buffer	*/

typedef void goto_f(void);
goto_f *run;

char *tftp_filename;
unsigned long load_addr;


int parse_line (char *parse[], char *line, char sep)
{
	char *c;
	char **p = parse;

	for(c = line; *c != '\0'; c++) {
		if(! (isalnum(*c) || *c == ' ' || *c == '.')) {
			puts("line contain unknow character\n");
			return -1;
		}
	}

	c = line;
	do {
		// skip blanks ahead words
		while(*c == ' ')
			c++;
		if (*c == '\0') {
			goto end;
		}
		// make one word
		*p++ = c;
		while(*c != sep) {
			if (*c++ == '\0')
				goto end;
		}
		*c++ = '\0';
	}while(p < parse + PARSE_SIZE);

end:	
	return (p - parse);
}

void arp_req(void)
{
	ArpRequest();
}

void tftp_req(char *fname, long raddr)
{
	tftp_filename = fname;
	load_addr = raddr;

	arp_req();
	TftpStart();
}

void ip_info(void) 
{
	char ip_str[16];
	ip_to_string(NetOurIP, ip_str);
	printf("local IP: %s\n", ip_str);
	ip_to_string(NetServerIP, ip_str);
	printf("server IP: %s\n", ip_str);
}

IPaddr_t string_to_ip(char *ipstr)
{
	char *ip_parr[4];
	unsigned char ip_arr[4];
	int i;

	i = parse_line(ip_parr, ipstr, '.');
	if(i != 4) {
		puts("ip format error");
		return 0;
	}
	for(i = 0; i < 4; i++) {
		ip_arr[i] = atoi(ip_parr[i]);
	}

	return NetReadIP(ip_arr);
}

// tftp test 32000000
// tftp filename sdram_addr
// ip [(local|server ip_addr) | info]	
// run sdram_addr
void run_command(char *command)
{
	char *cmds[CMD_NUM] = {"tftp", "ip", "run", "nand"};
	int cmd_id = -1; 
	char *parse[PARSE_SIZE];

	int n = parse_line(parse, command, ' ');

	if (n < 1) {
		return;
	}
	// cmd
	int i;
	for(i = 0; i < CMD_NUM; i++) {
		if(strcmp(parse[0], cmds[i]) == 0) {
			cmd_id = i;
			break;
		}
	}
	
	switch(cmd_id) {
	case 0://tftp filename addr
		if (n == 3) {
			//printf("tftp require %s in %s", parse[1], parse[2]);
			tftp_req(parse[1], simple_strtoul(parse[2], (char **)0, 16));
		}
		break;
	case 1://ip
		if((n == 2) && (strcmp(parse[1], "info") == 0)) { //ip info
			ip_info();
		}
		if (n != 3)
			return;
		if(strcmp(parse[1], "local") == 0) { 	//ip local addr
			NetOurIP = string_to_ip(parse[2]);
			//print_IPaddr(NetOurIP);
		}
		if(strcmp(parse[1], "server") == 0) {	//ip server addr
			NetServerIP = string_to_ip(parse[2]);
			//print_IPaddr(NetServerIP);
		}
		break;
	case 2://run addr
		if (n == 2) {
			unsigned long addr = simple_strtoul(parse[1], (char **)0, 16);
			printf("run a pragram from 0x%x in SDRAM\n", addr);
			//addr = 0x30000000;
                        run = (void *)addr;
                        run();
		}
		break;
	default:
		break;
	}
}

void main_loop(void)
{
	char cmd_buff[CFG_CBSIZE];

	puts("enter commmand:\n");
	puts("tftp filename sdram_addr\n");
	puts("ip ([local|server] ip_addr) | info\n");
	puts("run sdram_addr\n\n");

	puts("or quickly, enter 1-5\n");
	puts("1. arp req\n");
	puts("2. eth_rx\n");
	puts("3. read nand to 31000000\n");
	puts("4. prog nand from 31000000\n");
	puts("5. tftp test 32000000\n");

	NetOurIP = 0xc0a80107; // default local ip address is 192.168.1.7
	NetServerIP = 0xc0a80108; // default server ip 192.168.1.8
	NetOurGatewayIP = 0;
	NetOurSubnetMask = 0;	/* Our subnet mask (0 = unknown)*/
	tftp_filename = "test";
	load_addr = 0x32000000;

	while (1) {
		int len =  readline();
		if (len > 0) {
			strcpy(cmd_buff, console_buffer);
			run_command(cmd_buff);
			switch(console_buffer[0]){
			case '1':
				arp_req();
				break;
			case '2':
				eth_rx();
				break;
			case '3':
				puts("nand flash read\n");
				nand_read_from_zero((int *)0x31000000, 32 * 1024);
				break;
			case '4':
				puts("nand flash program\n");
				nand_prog_to_zero((int *)0x31000000, 32 * 1024);
				break;
			case '5':
				tftp_req("test", 0x32000000);
				break;
			}
		}

	}	
}
