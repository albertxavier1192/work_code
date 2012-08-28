/* gcc pcap.c -lpcap */
#include <sys/time.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#include <pcap/pcap.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>

#define PROMISCUOUS 1
#define NONPROMISCUOUS 0
char dev[] = "eth0";

// IP ��� ����ü
struct ip *iph;

// TCP ��� ����ü
struct tcphdr *tcph;

// ��Ŷ�� �޾Ƶ��ϰ�� �� �Լ��� ȣ���Ѵ�.  
// packet �� �޾Ƶ��� ��Ŷ�̴�.
void callback(u_char *useless, const struct pcap_pkthdr *pkthdr, 
                const u_char *packet)
{
    static int count = 1;
    struct ether_header *ep;
    unsigned short ether_type;    
    int chcnt =0;
    int length=pkthdr->len;

    // �̴��� ����� �����´�. 
    ep = (struct ether_header *)packet;

    // IP ����� �������� ���ؼ� 
    // �̴��� ��� ũ�⸸ŭ offset �Ѵ�.   
    packet += sizeof(struct ether_header);

    // �������� Ÿ���� �˾Ƴ���. 
    ether_type = ntohs(ep->ether_type);

    // ���� IP ��Ŷ�̶�� 
    if (ether_type == ETHERTYPE_IP)
    {
        // IP ������� ����Ÿ ������ ����Ѵ�.  
        iph = (struct ip *)packet;
        printf("IP ��Ŷ\n");
        printf("Version     : %d\n", iph->ip_v);
        printf("Header Len  : %d\n", iph->ip_hl);
        printf("Ident       : %d\n", ntohs(iph->ip_id));
        printf("TTL         : %d\n", iph->ip_ttl); 
        printf("Src Address : %s\n", inet_ntoa(iph->ip_src));
        printf("Dst Address : %s\n", inet_ntoa(iph->ip_dst));

        // ���� TCP ����Ÿ ���
        // TCP ������ ����Ѵ�. 
        if (iph->ip_p == IPPROTO_TCP)
        {
            tcph = (struct tcp *)(packet + iph->ip_hl * 4);
            printf("Src Port : %d\n" , ntohs(tcph->source));
            printf("Dst Port : %d\n" , ntohs(tcph->dest));
        }

        // Packet ����Ÿ �� ����Ѵ�. 
        // IP ��� ���� ����Ѵ�.  
        while(length--)
        {
            printf("%02x", *(packet++)); 
            if ((++chcnt % 16) == 0) 
                printf("\n");
        }
    }
    // IP ��Ŷ�� �ƴ϶�� 
    else
    {
        printf("NONE IP ��Ŷ\n");
    }
    printf("\n\n");
}    

int main(int argc, char **argv)
{
    bpf_u_int32 netp;
    bpf_u_int32 maskp;
    char errbuf[PCAP_ERRBUF_SIZE];
    int ret;
    struct pcap_pkthdr hdr;
    struct in_addr net_addr, mask_addr;
    struct ether_header *eptr;
    const u_char *packet;

    struct bpf_program fp;     

    pcap_t *pcd;  // packet capture descriptor

    pcd = pcap_open_live(argv[2], BUFSIZ,  NONPROMISCUOUS, -1, errbuf);
    if (pcd == NULL)
    {
        printf("%s\n", errbuf);
        exit(1);
    }    

    // ������ �ɼ��� �ش�.
    if (pcap_compile(pcd, &fp, argv[2], 0, netp) == -1)
    {
        printf("compile error\n");    
        exit(1);
    }
    // ������ �ɼǴ�� ��Ŷ���� ���� �����Ѵ�. 
    if (pcap_setfilter(pcd, &fp) == -1)
    {
        printf("setfilter error\n");
        exit(0);    
    }

    // ������ Ƚ����ŭ ��Ŷĸ�ĸ� �Ѵ�. 
    // pcap_setfilter �� ����� ��Ŷ�� ���ð�� 
    // callback �Լ��� ȣ���ϵ��� �Ѵ�. 
    pcap_loop(pcd, atoi(argv[1]), callback, NULL);
}
		
