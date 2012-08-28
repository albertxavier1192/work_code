/* gcc arp_injection.c -lpcap */
#include <stdio.h>
#include <pcap/pcap.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#include <linux/ip.h>
#include <linux/udp.h>

#define ETH_HW_ADDR_LEN 6
#define IP_ADDR_LEN 4
#define ARP_FRAME_TYPE 0x0806
#define ETHER_HW_TYPE 1
#define IP_PROTO_TYPE 0x0800
#define OP_ARP_REQUEST 1
#define OP_ARP_REPLY   2
struct arp_packet {
	u_char dest_hw_addr[ETH_HW_ADDR_LEN];
	u_char src_hw_addr[ETH_HW_ADDR_LEN];
	u_short frame_type;
	u_short hw_type;
	u_short prot_type;
	u_char hw_addr_size;
	u_char prot_addr_size;
	u_short op; 
	u_char sndr_hw_addr[ETH_HW_ADDR_LEN];
	u_char sndr_ip_addr[IP_ADDR_LEN];
	u_char targ_hw_addr[ETH_HW_ADDR_LEN];
	u_char targ_ip_addr[IP_ADDR_LEN];
	//u_char padding[18];
};

#define MAC_LEN 6

void arp_send(pcap_t *fd)
{
	struct arp_packet pkt;
	static unsigned char src_mac[MAC_LEN] =
	{ 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 };
	static unsigned char dest_mac[MAC_LEN] =
	{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

	unsigned char src_ip[IP_ADDR_LEN] = { 110,76,140,88};
	unsigned char dest_ip[IP_ADDR_LEN] = { 10, 51, 201, 1};
	int ret;

	memset(&pkt, 0, sizeof(pkt));
	pkt.frame_type = htons(ARP_FRAME_TYPE);
	pkt.hw_type = htons(ETHER_HW_TYPE);
	pkt.prot_type = htons(IP_PROTO_TYPE);
	pkt.hw_addr_size = ETH_HW_ADDR_LEN;
	pkt.prot_addr_size = IP_ADDR_LEN;

	pkt.op = htons(OP_ARP_REPLY);

	memcpy(pkt.src_hw_addr, src_mac, MAC_LEN);
	memcpy(pkt.dest_hw_addr, dest_mac, MAC_LEN);

	memcpy(pkt.sndr_ip_addr, src_ip, IP_ADDR_LEN);
	memcpy(pkt.targ_ip_addr, dest_ip, IP_ADDR_LEN);

	ret = pcap_inject(fd, (char *)&pkt, sizeof(pkt));

	printf("ret %d\n", ret);
}

#define MESG_LENGTH 5
char MESG[MESG_LENGTH] = { 0x10, 0x14, 0xff, 0x12, 0x42 };
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
        sum = (sum >> 16) + (sum & 0xffff);     /* add hi 16 to low 16 */
        sum += (sum >> 16);     /* add carry */
        answer = ~sum;          /* truncate to 16 bits */
        return (answer);
}

static void ip_gen(unsigned char *packet, unsigned char protocol,
                   struct in_addr saddr, struct in_addr daddr,
                   unsigned short length)
{

#define IPVERSION 4
#define DEFAULT_TTL 60          // Just hard code the ttl in the ip header.

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
		unsigned short dport) 
{
	struct in_addr saddr, daddr;
	unsigned short size = sizeof(struct iphdr) + sizeof(struct udphdr) + 
		MESG_LENGTH;
	int offset = 0;

	saddr.s_addr = inet_addr(s_ip);
	daddr.s_addr = inet_addr(d_ip);

	ip_gen(packet, IPPROTO_UDP, saddr, daddr, size);
	offset += sizeof(struct iphdr);
	udp_gen(packet + offset, sport, dport, 
			(sizeof(struct udphdr) + MESG_LENGTH ));
	offset += sizeof(struct udphdr);

	strncpy((char *)packet + offset, MESG, MESG_LENGTH);
	offset += MESG_LENGTH;

	printf("%d %d %d %d\n",sizeof(struct ether_header), sizeof(struct iphdr),
			sizeof(struct udphdr), MESG_LENGTH);

	return offset;
}

void send_udp(pcap_t *fd)
{
	char buf[1000];
	struct ether_header *ether = (struct ether_header*)buf;
	int offset = 0;
	int ret;

	char s_ip[] = {"10.51.48.9"};
	char d_ip[] = {"239.255.255.250"};
	unsigned short sport = 1900;
	unsigned short dport = 1900;

	static unsigned char dest_mac[MAC_LEN] =
	{ 0x01, 0x00, 0x5e, 0x7f,0xff, 0xfa };
	static unsigned char src_mac[MAC_LEN] =
	{ 0x08, 0x2e, 0x5f, 0x15, 0xda, 0x0e };

	memcpy(ether->ether_dhost, dest_mac, MAC_LEN);
	memcpy(ether->ether_shost, src_mac, MAC_LEN);
	ether->ether_type = ntohs(ETH_P_IP);

	offset += sizeof(struct ether_header);

	offset += create_udp_packet(buf+offset, s_ip, sport, d_ip, dport);

	ret = pcap_inject(fd, buf, offset);
	printf("ret %d %d\n",ret, offset);
}

void main()
{
	char errbuf[PCAP_ERRBUF_SIZE];
	char dev[] = "eth0";

	pcap_t *fd;

	fd = pcap_create(dev, errbuf);
	pcap_activate(fd);
	pcap_datalink(fd);

	arp_send(fd);

	send_udp(fd);
}
