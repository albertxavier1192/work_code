/* gcc wireless_icmp.c -lpcap */
#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

#include <ctype.h>
#include <pcap.h>
#include <string.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <net/ethernet.h>
#include <assert.h>



struct ctrl_cts_t {
    u_int16_t       fc;     
    u_int16_t       duration;
    u_int8_t        ra[6];  
};  

struct ctrl_rts_t {
    u_int16_t       fc;     
    u_int16_t       duration;
    u_int8_t        ra[6];  
    u_int8_t        ta[6];  
};  



pcap_t *fd;
char interface[]="mon0";

#define MESG_LENGTH 5
char MESG[MESG_LENGTH] = { 0x10, 0x14, 0xff, 0x12, 0x42 };

/* ###### temp ######*/
char s_ip[] = { "10.51.49.7" };
char d_ip[] = {"172.16.0.20"};
unsigned short sport = 7000;
unsigned short dport = 7000;

int init_pcap()
{
    char errbuf[PCAP_ERRBUF_SIZE];
    int ret;

    if(!(fd= pcap_create(interface, errbuf))) {
	perror("pcap_create");
	return -1; 
    }   

    ret = pcap_activate(fd);
    printf("%s pcap_activate: %d\n", interface, ret);
    ret = pcap_datalink(fd);
    printf("%s pcap_datalink: %d\n", interface, ret);

    return 0;
}

void dump_packet(const unsigned char* buf, int len)
{
    int i;
    for (i = 0; i < len; i++) {
	if ((i % 2) == 0)
	    printf(" ");
	if ((i % 16) == 0)
	    printf("\n");
	printf("%02x", buf[i]);
    }
    printf("\n\n");
}    

/*### wireless test ###*/
// in_cksum -- Checksum routine for Internet Protocol family headers (C Version)    
static unsigned short in_cksum(unsigned short *addr, int len)
{
	register int sum = 0;
	u_short answer = 0;
	register u_short *w = addr;
	register int nleft = len;

	/*                                                                      
	 * Our algorithm is simple, using a 32 bit accumulator (sum), we add    
	 * sequential 16 bit words to it, and at the end, fold back all the     
	 * carry bits from the top 16 bits into the lower 16 bits.              
	 */
	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1) {
		*(u_char *) (&answer) = *(u_char *) w;
		sum += answer;
	}

	/* add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);	/* add carry */
	answer = ~sum;		/* truncate to 16 bits */
	return (answer);
}

static void ip_gen(unsigned char *packet, unsigned char protocol,
		   struct in_addr saddr, struct in_addr daddr,
		   unsigned short length)
{

#define IPVERSION 4
#define DEFAULT_TTL 60		// Just hard code the ttl in the ip header.

	struct iphdr *iphdr;

	iphdr = (struct iphdr *)packet;
	memset((char *)iphdr, '\0', sizeof(struct iphdr));

	iphdr->ihl = 5;
	iphdr->version = IPVERSION;

	iphdr->tot_len = htons(length);
	iphdr->id = 10;
	iphdr->ttl = DEFAULT_TTL;
	iphdr->protocol = protocol;
	iphdr->saddr = saddr.s_addr;
	iphdr->daddr = daddr.s_addr;
	iphdr->check =
	    (unsigned short)in_cksum((unsigned short *)iphdr,
				     sizeof(struct iphdr));

	return;
}

static void udp_gen(unsigned char *packet, unsigned short sport,
		    unsigned short dport, unsigned short length)
{
	struct udphdr *udp;

	udp = (struct udphdr *)packet;
	udp->source = htons(sport);
	udp->dest = htons(dport);
	udp->len = htons(length);
	udp->check = 0;

	return;
}

static int create_udp_packet(unsigned char *packet, char *s_ip,
			     unsigned short sport, char *d_ip,
			     unsigned short dport, u_int8_t * bssid,
			     u_int8_t * station_mac)
{
	struct in_addr saddr, daddr;
	unsigned short size =
	    sizeof(struct iphdr) + sizeof(struct udphdr) + MESG_LENGTH +
	    2 * ETH_ALEN;
	int offset = 0;

	saddr.s_addr = inet_addr(s_ip);
	daddr.s_addr = inet_addr(d_ip);

	ip_gen(packet, IPPROTO_UDP, saddr, daddr, size);
	offset += sizeof(struct iphdr);
	udp_gen(packet + offset, sport, dport,
		(sizeof(struct udphdr) + MESG_LENGTH + 2 * ETH_ALEN));
	offset += sizeof(struct udphdr);

	strncpy((char *)packet + offset, MESG, MESG_LENGTH);
	offset += MESG_LENGTH;

	memcpy((char *)packet + offset, (char *)bssid, 6);
	memcpy((char *)packet + offset + ETH_ALEN, station_mac, ETH_ALEN);

	offset += (2 * ETH_ALEN);
	return offset;
}

static const u_char u8aRadiotapHeader[] = {
	0x00, 0x00,		// <-- radiotap version
	0x08, 0x00,		// <- radiotap header length
	0x00, 0x00, 0x00, 0x00,	// <-- bitmap
};

static const u_char u8aIeeeHeader[] = {
	0x08,			// <-- type/subtype
	0x02,			// <-- flags
	0x00, 0x00,		// <-- duration
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// <-- BSS id
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// <-- source address
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// <-- destination address
	0x10, 0x00,		// <-- frame number & sequence number
};

static const u_char LLC_Header[] = {
	0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00, 0x08, 0x00,
};

static const u_char icmp[] = { 
	0x45, 0x00, 0x00,
	0x3c, 0x5e, 0x07, 0x00, 0x00, 0x80, 0x01, 0xde, 0xd0, 0x0a, 0x33, 0x31, 0x07, 0xc0, 0xa8, 0x02, 0x07,
	0x08, 0x00, 0x37, 0x67, 0x00, 0x01, 0x15, 0xf4, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
	0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x61, 0x62, 0x63,
	0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
};


#define DESTINATION (udp_buff + 12)
#define BSSID (udp_buff + 18)
#define SOURCE (udp_buff + 24)
#define SEQ_CTRL (udp_buff + 30)
int injection()
{
    tcsendbreak(STDIN_FILENO, 100);
    char bssid[] = {0x08, 0x86, 0x3b, 0x11, 0x16, 0xf3};
    char src_mac[] = {0x08, 0x86, 0x3b, 0x11, 0x16, 0xf3};
    char dest_mac[] = {0x00, 0x26, 0x66, 0x44, 0xff, 0xec};

    while(1){
	char udp_buff[10000];
	int offset = 0;

	memset(udp_buff, 0, sizeof(udp_buff));

	memcpy(udp_buff, u8aRadiotapHeader, sizeof(u8aRadiotapHeader));
	offset += sizeof(u8aRadiotapHeader);
	memcpy(udp_buff + offset, u8aIeeeHeader, sizeof(u8aIeeeHeader));
	offset += sizeof(u8aIeeeHeader);
	memcpy(udp_buff + offset, LLC_Header, sizeof(LLC_Header));
	offset += sizeof(LLC_Header);

	memcpy(BSSID, bssid, ETH_ALEN);
	memcpy(SOURCE, src_mac, ETH_ALEN);
	memcpy(DESTINATION, dest_mac, ETH_ALEN);

	memcpy(udp_buff + offset, icmp, sizeof(icmp));

	offset += sizeof(icmp);

	int ret = pcap_inject(fd, udp_buff, offset);
	printf("ret %d %d\n",ret, offset);
	dump_packet(udp_buff, offset);

	break;
    }
    return 0;
}

int main()
{
    init_pcap();
    injection();
    return 0;
}

