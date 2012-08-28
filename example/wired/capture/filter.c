/* gcc filter.c -lpcap */
#include "pcap.h"
/* 패킷이 캡처 됬을때, 호출되는 콜백 함수 */
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);
main()
{
	pcap_if_t *alldevs;
	pcap_if_t *d;
	int inum;
	int i=0;
	pcap_t *adhandle;
	char errbuf[PCAP_ERRBUF_SIZE];

	//필터룰 지정 
	char *filter = "port 80";
	struct bpf_program fcode;
	bpf_u_int32 NetMask;

	/* 네트워크 다바이스 목록을 가져온다. */
	/* alldevs에 리스트 형태로 저장되며, 에러시 errbuf에 에러 내용 저장 */
	if(pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		fprintf(stderr,"Error in pcap_findalldevs: %s\n", errbuf);
		exit(1);
	}

	/* 네트워크 다바이스명을 출력한다. */
	for(d=alldevs; d; d=d->next)
	{
		printf("%d. %s", ++i, d->name);
		if (d->description)
			printf(" (%s)\n", d->description);
		else
			printf(" (No description available)\n");
	}

	/* 에러 처리 */
	if(i==0)
	{
		printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
		return -1;
	}

	/* 캡처할 네트워크 디바이스 선택 */
	printf("Enter the interface number (1-%d):",i);
	scanf("%d", &inum);

	/* 입력값의 유효성 판단 */
	if(inum < 1 || inum > i)
	{
		printf("\nInterface number out of range.\n");
		/* 장치 목록 해제 */
		pcap_freealldevs(alldevs);
		return -1;
	}

	/* 사용자가 선택한 장치목록 선택 */
	for(d=alldevs, i=0; i< inum-1 ;d=d->next, i++);

	/* 실제 네트워크 디바이스를 오픈 */
	if ((adhandle= pcap_open_live(d->name, // 디바이스명
					65536,   // 최대 캡처길이 
					// 65536 -> 캡처될수 있는 전체 길이 
					1,    // 0: 자신에게 해당되는 패킷만 캡처
					// 1: 들어오는 모든 패킷 캡처
					1000,   // read timeout 
					errbuf   // 에러내용 저장변수 
				     )) == NULL)
	{
		fprintf(stderr,"\nUnable to open the adapter. %s is not supported by WinPcap\n", d->name);
		/* 장치 목록 해제 */
		pcap_freealldevs(alldevs);
		return -1;
	}

	printf("\nlistening on %s...\n", d->description);

	/* 넷마스크 지정, 이부분은 아직 잘 모르겠음 */
	NetMask=0xffffff;
	// 사용자가 정의한 필터룰 컴파일
	if(pcap_compile(adhandle, &fcode, filter, 1, NetMask) < 0)
	{
		fprintf(stderr,"\nError compiling filter: wrong syntax.\n");
		pcap_close(adhandle);
		return -3;
	}
	// 사용자가 정의한 필터룰 적용
	if(pcap_setfilter(adhandle, &fcode)<0)
	{
		fprintf(stderr,"\nError setting the filter\n");
		pcap_close(adhandle);
		return -4;
	}
	/* 장치 목록 해제 */
	pcap_freealldevs(alldevs);

	/* 캡처 시작 */
	pcap_loop(adhandle,      // pcap_open_live통해 얻은 네트워크 디바이스 핸들
			0,     // 0 : 무한루프
			// 양의정수 : 캡처할 패킷수 
			packet_handler,  // 패킷이 캡처됬을때, 호출될 함수 핸들러 
			NULL);           // 콜백함수로 넘겨줄 파라미터 

	pcap_close(adhandle);    // 네트워크 디바이스 핸들 종료
	return 0;
}

/* 패킷이 캡처 됬을때, 호출되는 콜백 함수 */
void packet_handler(u_char *param,                    //파라미터로 넘겨받은 값 
		const struct pcap_pkthdr *header, //패킷 정보 
		const u_char *pkt_data)           //실제 캡처된 패킷 데이터
{

	struct tm *ltime;
	char timestr[16];
	time_t local_tv_sec;

	//출력 포맷 설정 
	local_tv_sec = header->ts.tv_sec;
	ltime=localtime(&local_tv_sec);
	strftime( timestr, sizeof timestr, "%H:%M:%S", ltime);

	// port 80에 해당되는 패킷만 캡처된다.
	printf("%s,%.6d len:%d\n", timestr, header->ts.tv_usec, header->len);
}

