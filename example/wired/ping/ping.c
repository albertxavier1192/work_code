//
// PING.C -- Ping program using ICMP and RAW Sockets
//http://serious-code.net/moin.cgi/ICMP

#include <stdio.h>
#include <string.h>
#ifndef WIN32
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#else
#include <windows.h>
#include <winsock.h>
#endif

#include "ping.h"

// Internal Functions
int Ping(LPCSTR pstrHost);
void ReportError(LPCSTR pstrFrom);
int  WaitForEchoReply(SOCKET s);
u_short in_cksum(u_short *addr, int len);

// ICMP Echo Request/Reply functions
int     SendEchoRequest(SOCKET, LPSOCKADDR_IN);
DWORD   RecvEchoReply(SOCKET, LPSOCKADDR_IN, u_char *);


// main()
void main(int argc, char **argv)
{
#ifdef WIN32
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(1,1);
	int nRet;


	// Init WinSock
	nRet = WSAStartup(wVersionRequested, &wsaData);
	if (nRet)
	{
		printf("\nError initializing WinSock\n");
		return;
	}

	// Check version
	if (wsaData.wVersion != wVersionRequested)
	{
		printf("\nWinSock version not supported\n");
		return;
	}
#endif
	// Go do the ping
	printf("result %d \n", Ping("10.51.0.1"));

	// Free WinSock
#ifdef WIN32
	WSACleanup();
#endif
	while(1);
}


// Ping()
// Calls SendEchoRequest() and
// RecvEchoReply() and prints results
int Ping(LPCSTR pstrHost)
{
	SOCKET    rawSocket;
	LPHOSTENT lpHost;
	struct    sockaddr_in saDest;
	struct    sockaddr_in saSrc;
	DWORD     dwTimeSent;
	u_char    cTTL;
	int       nLoop;
	int       nRet;
	int 	  result=0;

	// Create a Raw socket
	rawSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (rawSocket == -1) 
	{
		ReportError("socket()");
		return 0;
	}

	// Lookup host
	lpHost = gethostbyname(pstrHost);
	if (lpHost == NULL)
	{
		printf("\nHost not found: %s\n", pstrHost);
		return 0;
	}

	// Setup destination socket address
#ifndef WIN32
	memcpy((void *)(&saDest.sin_addr), lpHost->h_addr, sizeof(saDest.sin_addr));
#else
	saDest.sin_addr.s_addr = *((u_long FAR *) (lpHost->h_addr));
#endif
	saDest.sin_family = AF_INET;
	saDest.sin_port = 0;

	// Tell the user what we're doing
	printf("\nPinging %s [%s] with %d bytes of data:\n",
			pstrHost,
			inet_ntoa(saDest.sin_addr),
			REQ_DATASIZE);

	// Ping multiple times
	for (nLoop = 0; nLoop < 4; nLoop++)
	{
		// Send ICMP echo request
		SendEchoRequest(rawSocket, &saDest);

		// Use select() to wait for data to be received
		nRet = WaitForEchoReply(rawSocket);
		if (nRet ==  -1)
		{
			ReportError("select()");
			result=0;
			break;
		}
		if (!nRet)
		{
			printf("Timeout\n");
			result=0;
			break;
		}

		// Receive reply
		dwTimeSent = RecvEchoReply(rawSocket, &saSrc, &cTTL);
		if(!memcmp(&saSrc.sin_addr, &saDest.sin_addr, sizeof(saDest.sin_addr))){
			result=1;
			break;
		}

		result=0;

		// Calculate elapsed time
		printf("\nReply from: %s: bytes=%d TTL=%d\n", 
				inet_ntoa(saSrc.sin_addr), 
				REQ_DATASIZE,
				cTTL);
	}

#ifndef WIN32
	close(rawSocket);
	
#else
	closesocket(rawSocket);
#endif
	return result;
}


// SendEchoRequest()
// Fill in echo request header
// and send to destination
int SendEchoRequest(SOCKET s,LPSOCKADDR_IN lpstToAddr) 
{
	static ECHOREQUEST echoReq;
	static nId = 1;
	static nSeq = 1;
	int nRet;

	// Fill in echo request
	echoReq.icmpHdr.Type        = ICMP_ECHOREQ;
	echoReq.icmpHdr.Code        = 0;
	echoReq.icmpHdr.Checksum    = 0;
	echoReq.icmpHdr.ID          = nId++;
	echoReq.icmpHdr.Seq         = nSeq++;

	// Fill in some data to send
	for (nRet = 0; nRet < REQ_DATASIZE; nRet++)
		echoReq.cData[nRet] = ' '+nRet;

	// Save tick count when sent
#ifndef WIN32
	gettimeofday((struct timeval *)(&echoReq.dwTime), (void *)0);
#else
	echoReq.dwTime              = GetTickCount();
#endif

	// Put data in packet and compute checksum
	echoReq.icmpHdr.Checksum = in_cksum((u_short *)&echoReq, sizeof(ECHOREQUEST));

	// Send the echo request                                  
	nRet = sendto(s,                        /* socket */
			(LPSTR)&echoReq,           /* buffer */
			sizeof(ECHOREQUEST),
			0,                         /* flags */
			(LPSOCKADDR)lpstToAddr, /* destination */
			sizeof(SOCKADDR_IN));   /* address length */

	if (nRet == -1) 
		ReportError("sendto()");
	return (nRet);
}


// RecvEchoReply()
// Receive incoming data
// and parse out fields
DWORD RecvEchoReply(SOCKET s, LPSOCKADDR_IN lpsaFrom, u_char *pTTL) 
{
	ECHOREPLY echoReply;
	int nRet;
	int nAddrLen = sizeof(struct sockaddr_in);

	// Receive the echo reply   
	nRet = recvfrom(s,                  // socket
			(LPSTR)&echoReply,  // buffer
			sizeof(ECHOREPLY),  // size of buffer
			0,                  // flags
			(LPSOCKADDR)lpsaFrom,   // From address
			&nAddrLen);         // pointer to address len

	// Check return value
	if (nRet == -1) 
		ReportError("recvfrom()");

	// return time sent and IP TTL
	*pTTL = echoReply.ipHdr.TTL;
	return(echoReply.echoRequest.dwTime);           
}

// What happened?
void ReportError(LPCSTR pWhere)
{
	printf("error %s\n",pWhere);
}


// WaitForEchoReply()
// Use select() to determine when
// data is waiting to be read
int WaitForEchoReply(SOCKET s)
{
	struct timeval Timeout;
	fd_set readfds;

	FD_ZERO(&readfds); FD_SET(s, &readfds);

#if 0
	readfds.fd_count = 1;
	readfds.fd_array[0] = s;
#endif
	Timeout.tv_sec = 3;
	Timeout.tv_usec = 0;

#ifndef WIN32
	return(select(s+1, &readfds, NULL, NULL, &Timeout));
#else
	return(select(1, &readfds, NULL, NULL, &Timeout));
#endif
}


//
// Mike Muuss' in_cksum() function
// and his comments from the original
// ping program
//
// * Author -
// *    Mike Muuss
// *    U. S. Army Ballistic Research Laboratory
// *    December, 1983

/*
 *          I N _ C K S U M
 *
 * Checksum routine for Internet Protocol family headers (C Version)
 *
 */
u_short in_cksum(u_short *addr, int len)
{
	register int nleft = len;
	register u_short *w = addr;
	register u_short answer;
	register int sum = 0;

	/*
	 *  Our algorithm is simple, using a 32 bit accumulator (sum),
	 *  we add sequential 16 bit words to it, and at the end, fold
	 *  back all the carry bits from the top 16 bits into the lower
	 *  16 bits.
	 */
	while( nleft > 1 )  {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if( nleft == 1 ) {
		u_short u = 0;

		*(u_char *)(&u) = *(u_char *)w ;
		sum += u;
	}

	/*
	 * add back carry outs from top 16 bits to low 16 bits
	 */
	sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
	sum += (sum >> 16);         /* add carry */
	answer = ~sum;              /* truncate to 16 bits */
	return (answer);
}
