/* gcc pcap_dump2.c -lpcap */
#include <stdio.h>
#include <pcap.h>

pcap_dumper_t *dumpfile;
pcap_t *fd;
static void wifi_callback(u_char * user, const struct pcap_pkthdr *pkthdr,
		const u_char *packet){

	static int i;
	printf("hi\n");
	pcap_dump((u_char*)dumpfile, pkthdr, packet);

	if(++i > 10)
		pcap_breakloop(fd);
}

void main(int argc, char **argv)
{
	char errbuf[PCAP_ERRBUF_SIZE];
	int ret;
	if(argc < 3){
		printf("<interfac> <filename>\n");
		return;
	}

	if(!(fd = pcap_create(argv[1], errbuf))) {
		printf("create fault\n");
		return;
	}

	if((ret = pcap_activate(fd)) != 0) {
		printf("activate fault\n");
		return;
	}

	FILE *fp =  fopen(argv[2],"a+");

	dumpfile = pcap_dump_fopen(fd, fp);

	if(dumpfile == NULL){
		printf("dumpfile NULL\n");
		return;
	}

	printf("start\n");
	pcap_loop(fd, -1, wifi_callback, NULL);

	pcap_dump_close(dumpfile);

	close(fp);

	pcap_close(fd);

}
