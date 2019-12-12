/*
 *	Copied from Linux Monitor (LiMon) - Networking.
 *
 *	Copyright 1994 - 2000 Neil Russell.
 *	(See License)
 *	Copyright 2000 Roland Borde
 *	Copyright 2000 Paolo Scaffardi
 *	Copyright 2000, 2001 Wolfgang Denk
 * 
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 */

/*
 * General Desription:
 *
 * The user interface supports commands for BOOTP, RARP, and TFTP.
 * Also, we support ARP internally. Depending on available data,
 * these interact as follows:
 *
 * BOOTP:
 *
 *	Prerequisites:	- own ethernet address
 *	We want:	- own IP address
 *			- TFTP server IP address
 *			- name of bootfile
 *	Next step:	ARP
 *
 * RARP:
 *
 *	Prerequisites:	- own ethernet address
 *	We want:	- own IP address
 *			- TFTP server IP address
 *	Next step:	ARP
 *
 * ARP:
 *
 *	Prerequisites:	- own ethernet address
 *			- own IP address
 *			- TFTP server IP address
 *	We want:	- TFTP server ethernet address
 *	Next step:	TFTP
 *
 * DHCP:
 *
 *     Prerequisites:   - own ethernet address
 *     We want:         - IP, Netmask, ServerIP, Gateway IP
 *                      - bootfilename, lease time
 *     Next step:       - TFTP
 *
 * TFTP:
 *
 *	Prerequisites:	- own ethernet address
 *			- own IP address
 *			- TFTP server IP address
 *			- TFTP server ethernet address
 *			- name of bootfile (if unknown, we use a default name
 *			  derived from our own IP address)
 *	We want:	- load the boot file
 *	Next step:	none
 */


#include "../armboot.h"
// #include <command.h>
#include "net.h"
// #include "bootp.h"
// #include "tftp.h"
// #include "rarp.h"
// #include "arp.h"
#define ET_DEBUG

IPaddr_t NetOurIP = 0;
IPaddr_t NetOurGatewayIP = 0;
IPaddr_t NetServerIP = 0;
volatile uchar *NetTxPacket = 0;
volatile uchar * NetRxPackets[PKTBUFSRX];/* Receive packets		*/
uchar NetOurEther[6]={0xf0, 0x99, 0xbf, 0x5d, 0x45, 0xbc};
uchar NetServerEther[6];
uchar NetBcastAddr[6]={0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

volatile uchar *NetRxPkt;		/* Current receive packet		*/
int		NetRxPktLen;		/* Current rx packet length		*/

unsigned NetIPID;
static rxhand_f *packetHandler;		/* Current RX packet handler		*/
// static uchar *packetHandler;		/* Current RX packet handler		*/


void
NetSetHandler(rxhand_f * f)
{
	packetHandler = f;
}

int
NetCksumOk(uchar * ptr, int len)
{
	return !((NetCksum(ptr, len) + 1) & 0xfffe);
}

unsigned
NetCksum(uchar * ptr, int len)
{
	ulong	xsum;

	xsum = 0;
	while (len-- > 0)
		//xsum += *((ushort *)ptr)++;
		xsum += *(ptr)++;
	xsum = (xsum & 0xffff) + (xsum >> 16);
	xsum = (xsum & 0xffff) + (xsum >> 16);
	return (xsum & 0xffff);
}

void ip_to_string (IPaddr_t x, char *s)
{
//    sprintf (s,"%d.%d.%d.%d",
    printf (s,"%d.%d.%d.%d",
    	(int)((x >> 24) & 0xff),
	(int)((x >> 16) & 0xff),
	(int)((x >>  8) & 0xff),
	(int)((x >>  0) & 0xff)
    );
}

void print_IPaddr (IPaddr_t x)
{
    char tmp[12];

    ip_to_string(x, tmp);

    puts(tmp);
}

void
NetSetEther(volatile uchar * xet, uchar * addr, uint prot)
{
	volatile Ethernet_t *et = (Ethernet_t *)xet;

	NetCopyEther(et->et_dest, addr);
	NetCopyEther(et->et_src, NetOurEther);
	et->et_protlen = SWAP16(prot);
}

void
NetCopyEther(volatile uchar * to, uchar * from)
{
	int	i;

	for (i = 0; i < 6; i++)
		*to++ = *from++;
}

void
NetWriteIP(volatile uchar * to, IPaddr_t ip)
{
	int	i;

	for (i = 0; i < 4; i++) 
	{
		*to++ = ip >> 24;
	    	ip <<= 8;
	}
}

IPaddr_t
NetReadIP(volatile uchar * from)
{
	IPaddr_t ip;
	int i;

	ip = 0;
	for (i = 0; i < 4; i++) 
		ip = (ip << 8) | *from++;

	return ip;
}

void
NetPrintEther(volatile uchar * addr)
{
	int	i;
	
	for (i = 0; i < 6; i++)
	{
		//printf("%02x", *(addr+i));
		phex(*(addr+i), 2);
		
		if (i != 5)
			printf(":");
	}
	printf("\n");
}

void
NetSetIP(volatile uchar * xip, IPaddr_t dest, int dport, int sport, int len)
{
	volatile IP_t *ip = (IP_t *)xip;

	/*
	 *	If the data is an odd number of bytes, zero the
	 *	byte after the last byte so that the checksum
	 *	will work.
	 */
	if (len & 1)
		xip[IP_HDR_SIZE + len] = 0;

	/*
	 *	Construct an IP and UDP header.
			(need to set no fragment bit - XXX)
	 */
	ip->ip_hl_v  = 0x45;		/* IP_HDR_SIZE / 4 (not including UDP) */
	ip->ip_tos   = 0;
	ip->ip_len   = SWAP16(IP_HDR_SIZE + len);
	// ip->ip_id    = SWAP16(NetIPID++);
	NetIPID++;
	ip->ip_id    = SWAP16(NetIPID);
	ip->ip_off   = SWAP16c(0x4000);	/* No fragmentation */
	ip->ip_ttl   = 255;
	ip->ip_p     = 17;		/* UDP */
	ip->ip_sum   = 0;
	NetWriteIP((uchar*)&ip->ip_src, NetOurIP);
	NetWriteIP((uchar*)&ip->ip_dst, dest);
	ip->udp_src  = SWAP16(sport);
	ip->udp_dst  = SWAP16(dport);
	ip->udp_len  = SWAP16(8 + len);
	ip->udp_xsum = 0;
	ip->ip_sum   = ~NetCksum((uchar *)ip, IP_HDR_SIZE_NO_UDP / 2);
}

void
NetSendPacket(volatile uchar * pkt, int len)
{
	(void) eth_send(pkt, len);
}

void
NetReceive(volatile uchar * pkt, int len)
{
	Ethernet_t *et;
	IP_t	*ip;
	ARP_t *arp;
	int x;

	NetRxPkt = pkt;
	NetRxPktLen = len;
	et = (Ethernet_t *)pkt;

	x = SWAP16(et->et_protlen);

	if (x < 1514) {
		/*
		 *	Got a 802 packet.  Check the other protocol field.
		 */
		x = SWAP16(et->et_prot);
		ip = (IP_t *)(pkt + E802_HDR_SIZE);
		len -= E802_HDR_SIZE;
#ifdef ET_DEBUG
		printf("Receive 802 type ET, protocol 0x%x\n", x);
#endif

	} else {
		ip = (IP_t *)(pkt + ETHER_HDR_SIZE);
		len -= ETHER_HDR_SIZE;
#ifdef ET_DEBUG
		printf("Receive non-802 type ET\n");
#endif
	}

	switch(x) {
	case PROT_ARP:
		arp = (ARP_t *)ip;

		switch(SWAP16(arp->ar_op)) {
		case ARPOP_REQUEST:		/* reply with our Ether address	*/
#ifdef ET_DEBUG
			printf("Got ARP REQUEST, return our EtherAddr.\n");
#endif
			NetSetEther((uchar *)et, et->et_src, PROT_ARP);
			arp->ar_op = SWAP16(ARPOP_REPLY);
			NetCopyEther(&arp->ar_data[0],  NetOurEther);
			NetWriteIP(  &arp->ar_data[6],  NetOurIP);
			NetCopyEther(&arp->ar_data[10], NetOurEther);
			NetWriteIP(  &arp->ar_data[16], NetOurIP);
			NetSendPacket((uchar *)et,((uchar *)arp-pkt)+ARP_HDR_SIZE);
			return;
		case ARPOP_REPLY:		/* set TFTP server eth addr	*/
#ifdef ET_DEBUG
			printf("Got ARP REPLY, set server/gtwy eth addr\n");
#endif
			printf("local eth\n");
			NetPrintEther(NetOurEther);
			/* check if target ether is ours */
			if ( memcmp(NetOurEther, &(arp->ar_data[10]), 6) != 0 )
			{
#ifdef ET_DEBUG
				printf("  Reply is not for us. Ignoring it...\n");
#endif
				return;
			}			
			NetCopyEther(NetServerEther, &arp->ar_data[0]);
			printf("server eth addr: ");
			NetPrintEther(NetServerEther);
			// (*packetHandler)(0,0,0,0);	/* start TFTP */
			return;
		}

	case PROT_IP:
#ifdef ET_DEBUG
		printf("Got IP\n");
#endif
		if (len < IP_HDR_SIZE) {
			// debug ("len bad %d < %d\n", len, IP_HDR_SIZE);
			printf ("len bad %d < %d\n", len, IP_HDR_SIZE);
			return;
		}
		if (len < SWAP16(ip->ip_len)) {
			printf("len bad %d < %d\n", len, SWAP16(ip->ip_len));
			return;
		}
		len = SWAP16(ip->ip_len);
#ifdef ET_DEBUG
		printf("len=%d, v=%02x\n", len, ip->ip_hl_v & 0xff);
#endif
		if ((ip->ip_hl_v & 0xf0) != 0x40) {
			printf("version bad %x\n", ip->ip_hl_v & 0xf0);
			return;
		}
		if (ip->ip_off & SWAP16c(0x1fff)) { /* Can't deal w/ fragments */
			printf("can't deal with fragments\n");
			return;
		}
		if (!NetCksumOk((uchar *)ip, IP_HDR_SIZE_NO_UDP / 2)) {
			printf("checksum bad\n");
			return;
		}
		if (NetOurIP) {
		    IPaddr_t ipaddr = NetReadIP((uchar*)&ip->ip_dst);
		    if (ipaddr != NetOurIP && ipaddr != 0xFFFFFFFF)
		    {
#ifdef ET_DEBUG
			printf("ip packet not for us\n");
			print_IPaddr(ipaddr);
#endif
			return;
		    }
		}
		/*
		 * watch for ICMP host redirects
		 *
                 * There is no real handler code (yet). We just watch
                 * for ICMP host redirect messages. In case anybody
                 * sees these messages: please contact me
                 * (wd@denx.de), or - even better - send me the
                 * necessary fixes :-)
		 *
                 * Note: in all cases where I have seen this so far
                 * it was a problem with the router configuration,
                 * for instance when a router was configured in the
                 * BOOTP reply, but the TFTP server was on the same
                 * subnet. So this is probably a warning that your
                 * configuration might be wrong. But I'm not really
                 * sure if there aren't any other situations.
		 */
		if (ip->ip_p == IPPROTO_ICMP) {
			ICMP_t *icmph = (ICMP_t *)&(ip->udp_src);

			if (icmph->type != ICMP_REDIRECT)
				return;
			if (icmph->code != ICMP_REDIR_HOST)
				return;
			puts (" ICMP Host Redirect to ");
			print_IPaddr(icmph->un.gateway);
			putc(' ');
		} else if (ip->ip_p != IPPROTO_UDP) {	/* Only UDP packets */
			return;
		}

		/*
		 *	IP header OK.  Pass the packet to the current handler.
		 */
		(*packetHandler)((uchar *)ip +IP_HDR_SIZE,
						SWAP16(ip->udp_dst),
						SWAP16(ip->udp_src),
						SWAP16(ip->udp_len) - 8);

		break;
	default:
#ifdef ET_DEBUG
		printf("Got unknown protocol %04x\n",x);
#endif
	}
}

