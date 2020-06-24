// netutil.cpp: implementation of the netutil library
//
//////////////////////////////////////////////////////////////////////
//
// 2002.4.22 Hawke
// The following functions are added by Hawke.
// bool nu_set_send_timeout(int sock, int iTimeoutSecond);
// bool nu_set_recv_timeout(int sock, int iTimeoutSecond);
// bool nu_set_keep_alive(int sock, bool bEnable);
// bool nu_set_nonblock(int sock, bool bEnable);
// bool nu_set_reuse_addr(int sock, bool bEnable);
// bool nu_set_revbuf(int sock, int iBufferSize);
// bool nu_set_sndbuf(int sock, int iBufferSize);
// bool nu_get_rcvbuf(int sock, int *piBufferSize);
// bool nu_get_sndbuf(int sock, int *piBufferSize);
//
// 2003.4.29 Hawke
// bool nu_get_peer_ip_address(int connected_socket, unsigned char *ip_address_out);

#include "internal_CiUtils.h"	/* CiUtils.h includes CiGlobals.h hence windows.h */

#define CI_MAX_NIC_COUNT	10

#include "NetUtil.h"
#include "CiSafeString.h"

/* KSH for FIONBIO */
#ifndef _WIN32
#include <sys/ioctl.h>
#include <net/if.h>
#endif

#ifndef _WIN32
#include <sys/poll.h>
#endif

using namespace std;

#ifdef _WIN32
#include <Windows.h>
#include <Iphlpapi.h>
#pragma warning(disable : 4244 4389)

#define REGISTE_STRING _T("SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\%s\\Connection")

#endif

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

#ifdef _WIN32
bool nu_initialize_winsock2()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( 2, 2 );

	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		return false;
	}

	/* Confirm that the WinSock DLL supports 2.2.*/
	/* Note that if the DLL supports versions greater    */
	/* than 2.2 in addition to 2.2, it will still return */
	/* 2.2 in wVersion since that is the version we      */
	/* requested.                                        */

	if ( LOBYTE( wsaData.wVersion ) != 2 ||
		    HIBYTE( wsaData.wVersion ) != 2 ) {
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		WSACleanup( );
		return false;
	}

	return true;
}

bool nu_finalize_winsock2()
{
	return WSACleanup() == 0;
}
#endif

int nu_get_last_error()
{
#ifdef _WIN32
	return WSAGetLastError();
#else
	return errno;
#endif
}

bool nu_create_listen_socket(int *sock_out, unsigned short port, int listen_queue, const char* listen_ipaddr)
{
	int sock;
	int one = 1;
	struct sockaddr_in server_address;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if ( sock < 0 )
	{
		//fprintf(stdout,"nu_create_listen_socket : socket ERROR !!(%s)\n", strerror(errno));
		return false;
	}

	memset((void *)&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	if ( strlen(listen_ipaddr) > 0 && (strcmp(listen_ipaddr, "0.0.0.0") != 0))
	{
		char ip_addr[CI_MAX_IP_ADDRESS_LENGTH+1] = {0, };	
		ip_addr[CI_MAX_IP_ADDRESS_LENGTH] = '\0';
		strncpy(ip_addr, listen_ipaddr, CI_MAX_IP_ADDRESS_LENGTH);
		server_address.sin_addr.s_addr = inet_addr(ip_addr);
	}
	else
	{
		server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	server_address.sin_port = htons(port);

	if ( setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&one, sizeof(int)) < 0 ) {
		//fprintf(stdout,"nu_create_listen_socket : setsockopt ERROR !!(%s)\n", strerror(errno));
		nu_disconnect(sock);
		return false;
	}

	// sinma - 2006. 04. 05
	// listen socket 은 keep alive 옵션을 주는 것이 좋다.
	// 단 환경에 따라 keep alive 를 지원하지 않는 OS도 있으므로,
	// keep alive 의 설정 실패는 listen socket 생성 실패로 이어지지 않는다.
	nu_set_keep_alive(sock, true);

	if ( bind(sock, (struct sockaddr *)&server_address, sizeof(server_address)) != 0 ) {
		//fprintf(stdout,"nu_create_listen_socket : bind ERROR !! (%s)\n", strerror(errno));
		nu_disconnect(sock);
		return false;
	}

	if ( listen(sock, listen_queue) != 0 ) {
		//fprintf(stdout,"nu_create_listen_socket : listen ERROR !! (%s)\n", strerror(errno));
		nu_disconnect(sock);
		return false;
	}

	*sock_out = sock;

	return true;
}

bool nu_create_tcp_socket(int *sock_out)
{
	int sock;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if ( sock < 0 ) {
		return false;
	}

	*sock_out = sock;

	return true;
}

bool nu_create_udp_socket(int *sock_out)
{
	int sock;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if ( sock < 0 ) {
		return false;
	}

	*sock_out = sock;

	return true;
}

bool nu_connect(int sock, const char *dest_addr, unsigned short port)
{
	// sinma - 2006. 04. 05
	// close 시 TIME_WAIT 이 발생하지 않도록 하기 위해
	// linger option 을 사용함. 단, 이 경우 graceful disconnect 가 아닌
	// hard disconnect 가 됨.
	linger ling = {1,5};
	setsockopt(sock, SOL_SOCKET, SO_LINGER, (char*)&ling, sizeof(linger));

	struct sockaddr_in server_address;

	memset((void *)&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
#ifdef _WIN32
	server_address.sin_addr.S_un.S_addr = inet_addr(dest_addr);
#else
	if ( inet_pton(AF_INET, dest_addr, &server_address.sin_addr) != 1 ) {
		return false;
	}
#endif

	if ( connect(sock, (const struct sockaddr *)&server_address, sizeof(server_address)) != 0 ) {
		return false;
	}

	return true;
}



bool nu_NonBlockConnect( int iSocketFD, const char* pszServerIPAddress, int iPortNumber, unsigned int uiTimeOUTInSeconds )
{
	// sinma - 2006. 04. 05
	// close 시 TIME_WAIT 이 발생하지 않도록 하기 위해
	// linger option 을 사용함. 단, 이 경우 graceful disconnect 가 아닌
	// hard disconnect 가 됨.
	linger ling = {1,5};
	setsockopt(iSocketFD, SOL_SOCKET, SO_LINGER, (char*)&ling, sizeof(linger));

	unsigned long cmd = 1;//NonBlocking Mode

#ifdef _WIN32
	if( ioctlsocket( iSocketFD, FIONBIO, &cmd ) != 0 )
		return false;
#else
	if( ioctl( iSocketFD, FIONBIO, &cmd ) != 0 )
		return false;
#endif
	// nu_connect
	struct sockaddr_in server_address;

	memset((void *)&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(static_cast<unsigned short>(iPortNumber));
#ifdef _WIN32
	server_address.sin_addr.S_un.S_addr = inet_addr(pszServerIPAddress);
#else
	if ( inet_pton(AF_INET, pszServerIPAddress, &server_address.sin_addr) != 1 )
	{
		return false;
	}
#endif

	if( connect( iSocketFD, (const struct sockaddr *)&server_address, sizeof(server_address)) != 0 )
	{
		int iError = nu_get_last_error();
#ifdef _WIN32
		if ( iError != WSAEWOULDBLOCK )
#else
		if ( iError != EWOULDBLOCK && iError != EINPROGRESS )
#endif
		{
			return false;
		}

#ifdef _WIN32
		struct timeval timeout = { uiTimeOUTInSeconds, 0 };//3초

		fd_set rset, wset;

		FD_ZERO(&rset);
		FD_SET(iSocketFD, &rset);

		wset = rset;

		int iNEvent = select( iSocketFD + 1, &rset, &wset, NULL, &timeout);

		if ( iNEvent <= 0 )
		{
			return false;
		}

		if ( !FD_ISSET( iSocketFD, &rset ) && !FD_ISSET( iSocketFD, &wset ) )
		{
			return false;
		}
#else
		pollfd pollfd;
		pollfd.fd = iSocketFD;
		pollfd.events = POLLOUT | POLLWRNORM | POLLWRBAND | POLLERR;
		pollfd.revents = 0;
		int iNEvent = 0;
		do
		{
			iNEvent = poll(&pollfd, 1, uiTimeOUTInSeconds*1000);
		}while(iNEvent < 0 && errno == EINTR);

		if ( iNEvent <= 0 )
		{
			return false;
		}
#endif

		int error = 0;
		socklen_t len = sizeof(error);

		if ( getsockopt( iSocketFD, SOL_SOCKET, SO_ERROR, (char *)&error, &len ) < 0 )
		{
			return false;
		}

		if ( error )
		{
			return false;
		}
	}

	cmd = 0;//Blocking Mode

#ifdef _WIN32
	if( ioctlsocket( iSocketFD, FIONBIO, &cmd ) != 0 )
		return false;
#else
	if( ioctl( iSocketFD, FIONBIO, &cmd ) != 0 )
		return false;
#endif
	return true;
}

bool nu_disconnect(int sock)
{
#ifdef _WIN32
	return closesocket(sock) == 0;
#else
	return close(sock) == 0;
#endif
}

bool nu_accept(int listen_socket, int *sock_out,
				char *szClientIP, unsigned short *pusClientPort)
{
	int sock;
	socklen_t len;
	struct sockaddr_in cliaddr;
	len = sizeof( cliaddr);
	sock = accept(listen_socket, (struct sockaddr*)&cliaddr, &len);

	if ( sock < 0 ) {
		return false;
	}

	*sock_out = sock;

	if ( szClientIP != NULL ) {
        strcpy(szClientIP, inet_ntoa(cliaddr.sin_addr));
    }

    if ( pusClientPort != NULL ) {
        *pusClientPort = ntohs(cliaddr.sin_port);
    }

	return true;
}

bool nu_bind(int sock, unsigned short port)
{
	struct sockaddr_in local_addr;

	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	local_addr.sin_port = htons(port);

	if ( bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0 ) {
		return false;
	}

	return true;
}

bool nu_sendto(int sock, const void *buf, int ntowrite, const char *dest_addr, unsigned short port, int *nwritten)
{
	struct sockaddr_in server_address;

	memset((void *)&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
#ifdef _WIN32
	server_address.sin_addr.S_un.S_addr = inet_addr(dest_addr);
#else
	if ( inet_pton(AF_INET, dest_addr, &server_address.sin_addr) != 1 ) {
		return false;
	}
#endif

#ifdef _WIN32
	if ( (*nwritten = sendto(sock, (const char *)buf, ntowrite, 0, (const struct sockaddr *)&server_address, sizeof(server_address)) ) < 0 ) {
#else
	if ( (*nwritten = sendto(sock, (const char *)buf, ntowrite, MSG_NOSIGNAL, (const struct sockaddr *)&server_address, sizeof(server_address)) ) < 0 ) {
#endif
		return false;
	}

	return true;
}

bool nu_recvfrom(int sock, void *buf, int ntoread, int *nread)
{
	struct sockaddr_in recv_sin;
	socklen_t recv_sin_len = sizeof(recv_sin);
	int nread_temp;

#ifdef _WIN32
	nread_temp = recvfrom(sock, (char *)buf, ntoread, 0,
						(struct sockaddr *)&recv_sin, &recv_sin_len);
#else
	nread_temp = recvfrom(sock, (char *)buf, ntoread, MSG_NOSIGNAL,
						(struct sockaddr *)&recv_sin, &recv_sin_len);
#endif
	if ( nread_temp < 0 ) {
		return false;
	}

	*nread = nread_temp;

	return true;
}

bool nu_readn(int sock, void *buf, int ntoread, int *nread)
{
	int nleft;
	int nread_temp;
	char *ptr;

	ptr = (char *)buf;
	nleft = ntoread;
	while ( nleft > 0 ) {
#ifdef _WIN32
		if ( (nread_temp = recv(sock, ptr, nleft, 0)) < 0 ) {
#else
		if ( (nread_temp = recv(sock, ptr, nleft, MSG_NOSIGNAL)) < 0 ) {
#endif
			return false;
		} else if (nread_temp == 0) {
			break;	/* EOF */
		}

		nleft -= nread_temp;
		ptr += nread_temp;
	}

	*nread = ntoread - nleft;

	return true;
}

bool nu_writen(int sock, const void *buf, int ntowrite, int *nwritten)
{
	int nleft;
	int nwritten_temp;
	const char *ptr;

	ptr = (const char *)buf;
	nleft = ntowrite;
	while (nleft > 0) {
#ifdef _WIN32
		if ( ( nwritten_temp = send(sock, ptr, nleft, 0) ) < 0 ) {
#else
		if ( ( nwritten_temp = send(sock, ptr, nleft, MSG_NOSIGNAL) ) < 0 ) {
#endif
			return false;
		}
		else if ( nwritten_temp == 0 ) {
			break;
		}
		nleft -= nwritten_temp;
		ptr += nwritten_temp;
	}

	*nwritten = ntowrite - nleft;

	return true;
}

bool nu_udp_read_int8(int sock, char *int8)
{
	int nread;

	if ( !nu_recvfrom(sock, (void *)int8, 1, &nread) ) {
		return false;
	}

	if ( nread != 1 ) {
		return false;
	}

	return true;
}

bool nu_udp_read_int16(int sock, short *int16)
{
	short int16_temp;
	int nread;

	if ( !nu_recvfrom(sock, (void *)&int16_temp, 2, &nread) ) {
		return false;
	}

	if ( nread != 2 ) {
		return false;
	}

	*int16 = ntohs(int16_temp);

	return true;
}

bool nu_udp_read_int32(int sock, int *int32)
{
	int int32_temp;
	int nread;

	if ( !nu_recvfrom(sock, (void *)&int32_temp, 4, &nread) ) {
		return false;
	}

	if ( nread != 4 ) {
		return false;
	}

	*int32 = ntohl(int32_temp);

	return true;
}

bool nu_udp_read_int64(int sock, long long *int64)
{
	long long int64_temp0;
	long long int64_temp1;
	int nread;
	unsigned char *ptr_temp0;
	unsigned char *ptr_temp1;

	if ( !nu_recvfrom(sock, (void *)&int64_temp0, 8, &nread) ) {
		return false;
	}

	if ( nread != 8 ) {
		return false;
	}

	int64_temp1 = int64_temp0;

	if ( 1 != ntohl(1) ) {
		int i;

		ptr_temp0 = (unsigned char *)&int64_temp0;
		ptr_temp1 = (unsigned char *)&int64_temp1;

		for ( i = 0; i < (int)sizeof(long long); i++ ) {
			ptr_temp1[i] = ptr_temp0[sizeof(long long)-1-i];
		}
	}

	*int64 = int64_temp1;

	return true;
}

bool nu_udp_write_int8(int sock, char int8, const char *dest_addr, unsigned short port)
{
	int nwritten;

	if ( !nu_sendto(sock, (const void *)&int8, 1, dest_addr, port, &nwritten) ) {
		return false;
	}

	if ( nwritten != 1 ) {
		return false;
	}

	return true;
}

bool nu_udp_write_int16(int sock, short int16, const char *dest_addr, unsigned short port)
{
	int int16_temp;
	int nwritten;

	int16_temp = htons(int16);
	if ( !nu_sendto(sock, (const void *)&int16_temp, 2, dest_addr, port, &nwritten) ) {
		return false;
	}

	if ( nwritten != 2 ) {
		return false;
	}

	return true;
}

bool nu_udp_write_int32(int sock, int int32, const char *dest_addr, unsigned short port)
{
	int int32_temp;
	int nwritten;

	int32_temp = htonl(int32);
	if ( !nu_sendto(sock, (const void *)&int32_temp, 4, dest_addr, port, &nwritten) ) {
		return false;
	}

	if ( nwritten != 4 ) {
		return false;
	}

	return true;
}

bool nu_udp_write_int64(int sock, long long int64, const char *dest_addr, unsigned short port)
{
	long long int64_temp;
	int nwritten;
	unsigned char *ptr;
	unsigned char *ptr_temp;

	int64_temp = int64;
	if ( 1 != htonl(1) ) {
		int i;

		ptr = (unsigned char *)&int64;
		ptr_temp = (unsigned char *)&int64_temp;

		for ( i = 0; i < (int)sizeof(long long); i++ ) {
			ptr_temp[i] = ptr[sizeof(long long)-1-i];
		}
	}

	if ( !nu_sendto(sock, (const void *)&int64_temp, 8, dest_addr, port, &nwritten) ) {
		return false;
	}

	if ( nwritten != 8 ) {
		return false;
	}

	return true;
}

bool nu_read_int8(int sock, char *int8)
{
	int nread;

	if ( !nu_readn(sock, (void *)int8, 1, &nread) ) {
		return false;
	}

	if ( nread != 1 ) {
		return false;
	}

	return true;
}

bool nu_read_int16(int sock, short *int16)
{
	short int16_temp;
	int nread;

	if ( !nu_readn(sock, (void *)&int16_temp, 2, &nread) ) {
		return false;
	}

	if ( nread != 2 ) {
		return false;
	}

	*int16 = ntohs(int16_temp);

	return true;
}

bool nu_read_int32(int sock, int *int32)
{
	int int32_temp;
	int nread;

	if ( !nu_readn(sock, (void *)&int32_temp, 4, &nread) ) {
		return false;
	}

	if ( nread != 4 ) {
		return false;
	}

	*int32 = ntohl(int32_temp);

	return true;
}

bool nu_read_int64(int sock, long long *int64)
{
	long long int64_temp0;
	long long int64_temp1;
	int nread;
	unsigned char *ptr_temp0;
	unsigned char *ptr_temp1;

	if ( !nu_readn(sock, (void *)&int64_temp0, 8, &nread) ) {
		return false;
	}

	if ( nread != 8 ) {
		return false;
	}

	int64_temp1 = int64_temp0;

	if ( 1 != ntohl(1) ) {
		int i;

		ptr_temp0 = (unsigned char *)&int64_temp0;
		ptr_temp1 = (unsigned char *)&int64_temp1;

		for ( i = 0; i < (int)sizeof(long long); i++ ) {
			ptr_temp1[i] = ptr_temp0[sizeof(long long)-1-i];
		}
	}

	*int64 = int64_temp1;

	return true;
}

bool nu_write_int8(int sock, char int8)
{
	int nwritten;

	if ( !nu_writen(sock, (const void *)&int8, 1, &nwritten) ) {
		return false;
	}

	if ( nwritten != 1 ) {
		return false;
	}

	return true;
}

bool nu_write_int16(int sock, short int16)
{
	int int16_temp;
	int nwritten;

	int16_temp = htons(int16);
	if ( !nu_writen(sock, (const void *)&int16_temp, 2, &nwritten) ) {
		return false;
	}

	if ( nwritten != 2 ) {
		return false;
	}

	return true;
}

bool nu_write_int32(int sock, int int32)
{
	int int32_temp;
	int nwritten;

	int32_temp = htonl(int32);
	if ( !nu_writen(sock, (const void *)&int32_temp, 4, &nwritten) ) {
		return false;
	}

	if ( nwritten != 4 ) {
		return false;
	}

	return true;
}

bool nu_write_int64(int sock, long long int64)
{
	long long int64_temp;
	int nwritten;
	unsigned char *ptr;
	unsigned char *ptr_temp;

	int64_temp = int64;
	if ( 1 != htonl(1) ) {
		int i;

		ptr = (unsigned char *)&int64;
		ptr_temp = (unsigned char *)&int64_temp;

		for ( i = 0; i < (int)sizeof(long long); i++ ) {
			ptr_temp[i] = ptr[sizeof(long long)-1-i];
		}
	}

	if ( !nu_writen(sock, (const void *)&int64_temp, 8, &nwritten) ) {
		return false;
	}

	if ( nwritten != 8 ) {
		return false;
	}

	return true;
}

bool nu_get_local_ip_address(int connected_socket, unsigned char *ip_address_out)
{
	struct sockaddr_in local_addr;
	socklen_t addrlen;

	addrlen = sizeof(struct sockaddr_in);
	if ( getsockname(connected_socket, (struct sockaddr *)&local_addr, &addrlen) < 0 ) {
		return false;
	}

	strcpy((char*)ip_address_out, inet_ntoa(local_addr.sin_addr));

	return true;
}

bool nu_get_local_port_number(int binded_socket, unsigned short *port_out)
{
	struct sockaddr_in local_addr;
	socklen_t addrlen;

	addrlen = sizeof(struct sockaddr_in);
	if ( getsockname(binded_socket, (struct sockaddr *)&local_addr, &addrlen) < 0 )
	{
		//fprintf(stderr,"nu_get_local_port_number : getsockname ERROR !! (%s)\n", strerror(errno));
		return false;
	}

	*port_out = ntohs(local_addr.sin_port);

	return true;
}

bool nu_get_peer_ip_address(int connected_socket, unsigned char *ip_address_out)
{
	struct sockaddr_in peer_addr;
	socklen_t addrlen;

	addrlen = sizeof(struct sockaddr_in);
	if ( getpeername(connected_socket, (struct sockaddr *)&peer_addr, &addrlen) < 0 ) {
		return false;
	}

	strcpy((char*)ip_address_out, inet_ntoa(peer_addr.sin_addr));

	return true;
}

bool nu_writen_async(int sock, const void *buf, int ntowrite, int *nwritten, int iHundredMillisecondTimeOut)
{
	int nleft;
	int nwritten_temp;
	const char *ptr;

	ptr = (const char *)buf;
	nleft = ntowrite;
	while (nleft > 0) {
#ifdef _WIN32
		struct timeval timeout = { iHundredMillisecondTimeOut/10, (iHundredMillisecondTimeOut% 10)*100000 };

		fd_set wset;

		FD_ZERO(&wset);
		FD_SET(sock, &wset);

		int iNEvent = select( sock+1, NULL, &wset, NULL, &timeout);

		if ( iNEvent < 0 )
		{
			return false;
		}
		else if ( iNEvent == 0 )
		{
			break;
		}
#else
		pollfd pollfd;
		pollfd.fd = sock;
		pollfd.events = POLLOUT | POLLWRNORM | POLLWRBAND;
		pollfd.revents = 0;
		int iNEvent = 0;
		do
		{
			iNEvent = poll(&pollfd, 1, iHundredMillisecondTimeOut*100);
		}while(iNEvent < 0 && errno == EINTR);

		if ( iNEvent < 0 )
		{
			return false;
		}
		else if ( iNEvent == 0 )
		{
			break;
		}
#endif
#ifdef _WIN32
		if ( ( nwritten_temp = send(sock, ptr, nleft, 0) ) < 0 ) {
#else
		if ( ( nwritten_temp = send(sock, ptr, nleft, MSG_NOSIGNAL) ) < 0 ) {
#endif
			return false;
		}
		else if ( nwritten_temp == 0 ) {
			break;
		}
		nleft -= nwritten_temp;
		ptr += nwritten_temp;
	}

	*nwritten = ntowrite - nleft;

	if( nleft == 0 ) {
		return true;
	} else {
		return false;
	}
}

bool nu_readn_async(int sock, void *buf, int ntoread, int *nread, int iHundredMillisecondTimeOut)
{
	int nleft;
	int nread_temp;
	char *ptr;

	ptr = (char *)buf;
	nleft = ntoread;
	while ( nleft > 0 ) {
#ifdef _WIN32
		struct timeval timeout = { iHundredMillisecondTimeOut/10, (iHundredMillisecondTimeOut % 10)*100000 };

		fd_set rset;

		FD_ZERO(&rset);
		FD_SET(sock, &rset);

		int iNEvent = select( sock+1, &rset, NULL, NULL, &timeout);

		if ( iNEvent < 0 )
		{
			return false;
		}
		else if ( iNEvent == 0 )
		{
			break;
		}
#else
		pollfd pollfd;
		pollfd.fd = sock;
		pollfd.events = POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI;
		pollfd.revents = 0;
		int iNEvent = 0;
		do
		{
			iNEvent = poll(&pollfd, 1, iHundredMillisecondTimeOut*100);
		}while(iNEvent < 0 && errno == EINTR);

		if ( iNEvent < 0 )
		{
			return false;
		}
		else if ( iNEvent == 0 )
		{
			break;
		}
#endif
#ifdef _WIN32
		if ( (nread_temp = recv(sock, ptr, nleft, 0)) < 0 ) {
#else
		if ( (nread_temp = recv(sock, ptr, nleft, MSG_NOSIGNAL)) < 0 ) {
#endif
			return false;
		} else if (nread_temp == 0) {
			break;	/* EOF */
		}

		nleft -= nread_temp;
		ptr += nread_temp;
	}

	*nread = ntoread - nleft;

	if( nleft == 0 ) {
		return true;
	} else {
		return false;
	}
}

bool nu_read_int8_async(int sock, char *int8, int iHundredMillisecondTimeOut)
{
	int nread;

	if ( !nu_readn_async(sock, (void *)int8, 1, &nread, iHundredMillisecondTimeOut) ) {
		return false;
	}

	if ( nread != 1 ) {
		return false;
	}

	return true;
}

bool nu_read_int16_async(int sock, short *int16, int iHundredMillisecondTimeOut)
{
	short int16_temp;
	int nread;

	if ( !nu_readn_async(sock, (void *)&int16_temp, 2, &nread, iHundredMillisecondTimeOut) ) {
		return false;
	}

	if ( nread != 2 ) {
		return false;
	}

	*int16 = ntohs(int16_temp);

	return true;
}

bool nu_read_int32_async(int sock, int *int32, int iHundredMillisecondTimeOut)
{
	int int32_temp;
	int nread;

	if ( !nu_readn_async(sock, (void *)&int32_temp, 4, &nread, iHundredMillisecondTimeOut) ) {
		return false;
	}

	if ( nread != 4 ) {
		return false;
	}

	*int32 = ntohl(int32_temp);

	return true;
}

bool nu_read_int64_async(int sock, long long *int64, int iHundredMillisecondTimeOut)
{
	long long int64_temp0;
	long long int64_temp1;
	int nread;
	unsigned char *ptr_temp0;
	unsigned char *ptr_temp1;

	if ( !nu_readn_async(sock, (void *)&int64_temp0, 8, &nread, iHundredMillisecondTimeOut) ) {
		return false;
	}

	if ( nread != 8 ) {
		return false;
	}

	int64_temp1 = int64_temp0;

	if ( 1 != ntohl(1) ) {
		int i;

		ptr_temp0 = (unsigned char *)&int64_temp0;
		ptr_temp1 = (unsigned char *)&int64_temp1;

		for ( i = 0; i < (int)sizeof(long long); i++ ) {
			ptr_temp1[i] = ptr_temp0[sizeof(long long)-1-i];
		}
	}

	*int64 = int64_temp1;

	return true;
}

bool nu_write_int8_async(int sock, char int8, int iHundredMillisecondTimeOut)
{
	int nwritten;

	if ( !nu_writen_async(sock, (const void *)&int8, 1, &nwritten, iHundredMillisecondTimeOut) ) {
		return false;
	}

	if ( nwritten != 1 ) {
		return false;
	}

	return true;
}

bool nu_write_int16_async(int sock, short int16, int iHundredMillisecondTimeOut)
{
	int int16_temp;
	int nwritten;

	int16_temp = htons(int16);
	if ( !nu_writen_async(sock, (const void *)&int16_temp, 2, &nwritten, iHundredMillisecondTimeOut) ) {
		return false;
	}

	if ( nwritten != 2 ) {
		return false;
	}

	return true;
}

bool nu_write_int32_async(int sock, int int32, int iHundredMillisecondTimeOut)
{
	int int32_temp;
	int nwritten;

	int32_temp = htonl(int32);
	if ( !nu_writen_async(sock, (const void *)&int32_temp, 4, &nwritten, iHundredMillisecondTimeOut) ) {
		return false;
	}

	if ( nwritten != 4 ) {
		return false;
	}

	return true;
}

bool nu_write_int64_async(int sock, long long int64, int iHundredMillisecondTimeOut)
{
	long long int64_temp;
	int nwritten;
	unsigned char *ptr;
	unsigned char *ptr_temp;

	int64_temp = int64;
	if ( 1 != htonl(1) ) {
		int i;

		ptr = (unsigned char *)&int64;
		ptr_temp = (unsigned char *)&int64_temp;

		for ( i = 0; i < (int)sizeof(long long); i++ ) {
			ptr_temp[i] = ptr[sizeof(long long)-1-i];
		}
	}

	if ( !nu_writen_async(sock, (const void *)&int64_temp, 8, &nwritten, iHundredMillisecondTimeOut) ) {
		return false;
	}

	if ( nwritten != 8 ) {
		return false;
	}

	return true;
}

bool nu_set_send_timeout(int sock, int iTimeoutSecond)
{
	int len;

#ifdef _WIN32
	int timeout;
	timeout = iTimeoutSecond * 1000;	// milli-seconds
#else
	struct timeval timeout;
	timeout.tv_sec = iTimeoutSecond;
	timeout.tv_usec = 0;
#endif

	len = sizeof(timeout);
	if ( setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, len) == -1 ) {
		return false;
	}

	return true;
}

bool nu_set_recv_timeout(int sock, int iTimeoutSecond)
{
	int len;

#ifdef _WIN32
	int timeout;
	timeout = iTimeoutSecond * 1000;	// milli-seconds
#else
	struct timeval timeout;
	timeout.tv_sec = iTimeoutSecond;
	timeout.tv_usec = 0;
#endif

	len = sizeof(timeout);
	if ( setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, len) == -1 ) {
		return false;
	}

	return true;
}

bool nu_set_keep_alive(int sock, bool bEnable)
{
    int optval;

    if ( bEnable ) {
        optval = 1;
    }
    else {
        optval = 0;
    }

    if (setsockopt(sock,
                    SOL_SOCKET,
                    SO_KEEPALIVE,
                    (const char*)&optval,
                    sizeof(int)) == -1) {
        return false;
    }

    return true;
}

bool nu_set_reuse_addr(int sock, bool bEnable)
{
    int optval;

    if ( bEnable ) {
        optval = 1;
    }
    else {
        optval = 0;
    }

    if ( setsockopt(sock,
                    SOL_SOCKET,
                    SO_REUSEADDR,
                    (const char*)&optval,
                    sizeof(int)) == -1 ) {
        return false;
    }

    return true;
}

bool nu_set_revbuf(int sock, int iBufferSize)
{
    if ( setsockopt(sock,
                    SOL_SOCKET,
                    SO_RCVBUF,
                    (const char*)&iBufferSize,
                    sizeof(int)) == -1 ) {
        return false;
    }

    return true;
}

bool nu_set_sndbuf(int sock, int iBufferSize)
{
    if ( setsockopt(sock,
                    SOL_SOCKET,
                    SO_SNDBUF,
                    (const char*)&iBufferSize,
                    sizeof(int)) == -1 ) {
        return false;
    }

    return true;
}

bool nu_get_rcvbuf(int sock, int *piBufferSize)
{
    socklen_t iSize = sizeof(int);

    if ( getsockopt(sock,
                    SOL_SOCKET,
                    SO_RCVBUF,
                    (char*)piBufferSize,
                    &iSize) == -1 ) {
        return false;
    }

    if ( iSize != sizeof(int) ) {
        return false;
    }

    return true;
}

bool nu_get_sndbuf(int sock, int *piBufferSize)
{
    socklen_t iSize = sizeof(int);

    if ( getsockopt(sock,
                    SOL_SOCKET,
                    SO_SNDBUF,
                    (char *)piBufferSize,
                    &iSize) == -1 ) {
        return false;
    }

    if ( iSize != sizeof(int) ) {
        return false;
    }

    return true;
}

bool nu_get_address(const struct sockaddr_in *sockaddr_in, unsigned char *ip_address_out, unsigned short *port_out)
{
	if ( ip_address_out == NULL )
		return false;

	strcpy((char *)ip_address_out, inet_ntoa(sockaddr_in->sin_addr));
	*port_out = ntohs(sockaddr_in->sin_port);

	return true;
}

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4100)
#endif

#ifdef _WIN32
bool SHRegReadString(HKEY hKey, LPCTSTR lpKey, LPCTSTR lpValue, LPCTSTR lpDefault, LPTSTR lpRet, DWORD nSize)
{

	HKEY key;
	DWORD dwDisp;
	DWORD Size;
	if(RegCreateKeyEx(hKey, lpKey,0,NULL,REG_OPTION_NON_VOLATILE, KEY_READ, NULL, &key, &dwDisp) != ERROR_SUCCESS)
		return false;
	Size = nSize;
	if(RegQueryValueEx(key, lpValue, 0, NULL,(LPBYTE)lpRet,&Size) != ERROR_SUCCESS)
	{
		_tcscpy(lpRet, lpDefault);
		RegCloseKey(key);
		return false;
	}
	RegCloseKey(key);
	return true;

}
#endif

/* 결과값인 addr_out은 human-format이 아님 */
bool nu_get_mac_address(const char *ifname, unsigned char *addr_out)
{
#ifdef _WIN32
	IP_ADAPTER_INFO AdapterInfo[16];

	DWORD dwBufLen = sizeof(AdapterInfo);
	DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);

	if(dwStatus != ERROR_SUCCESS)
	{
		return false;
	}
	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;

	while(pAdapterInfo)
	{
		_TCHAR szNICName[128] = { 0, };
		_TCHAR MuxString[225] = { 0 ,};
		_stprintf(MuxString, REGISTE_STRING, pAdapterInfo->AdapterName);
		SHRegReadString( HKEY_LOCAL_MACHINE, MuxString, _T("Name"), _T("Nothing"), szNICName, 100 );

		char nic_name[CI_MAX_DESCRIPTION_LENGTH+1];
		nic_name[CI_MAX_DESCRIPTION_LENGTH] = '\0';

#ifdef _UNICODE
		WideCharToMultiByte(CP_ACP, 0, szNICName, -1, nic_name, CI_MAX_DESCRIPTION_LENGTH, NULL, NULL);
#else
		strncpy(nic_name, szNICName, CI_MAX_DESCRIPTION_LENGTH);
#endif

		if( !strcmp(nic_name, ifname) ) //NIC name동일시 MAC Address 조사
		{
			memcpy(addr_out, pAdapterInfo->Address, 6);
			return true;
		}
		pAdapterInfo = pAdapterInfo->Next;
	}
	return false;
#else
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if ( sockfd < 0 )
	{
		return false;
	}

	struct ifreq ifr;
 	strcpy( ifr.ifr_name, ifname );
	ifr.ifr_addr.sa_family = AF_INET;

	if ( ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0 )
	{
		close(sockfd);
		return false;
	}
	close(sockfd);

	memcpy(addr_out, ifr.ifr_hwaddr.sa_data, 6);

	return true;
#endif
}

bool nu_get_my_ip_address_list(std::vector<std::string>& ips)
{
#ifdef _WIN32
	IP_ADAPTER_INFO AdapterInfo[16];

	DWORD dwBufLen = sizeof(AdapterInfo);
	DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);

	if(dwStatus != ERROR_SUCCESS)
	{
		return false;
	}
	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;

	while(pAdapterInfo)
	{
		std::string sIpAddress;
		sIpAddress = pAdapterInfo->IpAddressList.IpAddress.String;
		ips.push_back(sIpAddress);
		pAdapterInfo = pAdapterInfo->Next;
	}
	return true;
#else
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if ( sockfd < 0 )
	{
		return false;
	}

	struct ifconf ifc;
	struct ifreq ifr[CI_MAX_NIC_COUNT];
    memset(ifr, 0, sizeof(struct ifreq) * CI_MAX_NIC_COUNT);

    ifc.ifc_len = sizeof(struct ifreq) * CI_MAX_NIC_COUNT;
    ifc.ifc_buf = (char *)ifr;

	if ( ioctl(sockfd, SIOCGIFCONF, &ifc) < 0 )
	{
		close(sockfd);
		return false;
	}

	int iNicCount = ifc.ifc_len / sizeof(struct ifreq);

	for ( int i=0; i<iNicCount; i++ )
	{
		struct sockaddr_in *sin = (struct sockaddr_in *)&ifr[i].ifr_addr;
		ips.push_back(inet_ntoa(sin->sin_addr));
	}

	close(sockfd);

	return true;
#endif
}

bool nu_get_my_nic_list(std::vector<std::string>& nics)
{
#ifdef _WIN32
	IP_ADAPTER_INFO AdapterInfo[16];

	DWORD dwBufLen = sizeof(AdapterInfo);
	DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);

	if(dwStatus != ERROR_SUCCESS)
	{
		return false;
	}
	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;

	while(pAdapterInfo)
	{
		_TCHAR szNICName[128] = { 0, };
		_TCHAR MuxString[225] = { 0 ,};
		_stprintf(MuxString, REGISTE_STRING, pAdapterInfo->AdapterName);
		SHRegReadString( HKEY_LOCAL_MACHINE, MuxString, _T("Name"), _T("Nothing"), szNICName, 100 );

		char nic_name[CI_MAX_DESCRIPTION_LENGTH+1];
		nic_name[CI_MAX_DESCRIPTION_LENGTH] = '\0';

#ifdef _UNICODE
		WideCharToMultiByte(CP_ACP, 0, szNICName, -1, nic_name, CI_MAX_DESCRIPTION_LENGTH, NULL, NULL);
#else
		strncpy(nic_name, szNICName, CI_MAX_DESCRIPTION_LENGTH);
#endif

		nics.push_back(nic_name);
		pAdapterInfo = pAdapterInfo->Next;
	}
	return true;
#else
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if ( sockfd < 0 )
	{
		return false;
	}

	struct ifconf ifc;
	struct ifreq ifr[CI_MAX_NIC_COUNT];
    memset(ifr, 0, sizeof(struct ifreq) * CI_MAX_NIC_COUNT);

    ifc.ifc_len = sizeof(struct ifreq) * CI_MAX_NIC_COUNT;
    ifc.ifc_buf = (char *)ifr;

	if ( ioctl(sockfd, SIOCGIFCONF, &ifc) < 0 )
	{
		close(sockfd);
		return false;
	}

	int iNicCount = ifc.ifc_len / sizeof(struct ifreq);

	for ( int i=0; i<iNicCount; i++ )
		nics.push_back(ifr[i].ifr_name);

	close(sockfd);

	return true;
#endif
}



bool nu_get_nic_from_ip(char *szNICName, const char *szIPAddress)
{
#ifdef _WIN32
	IP_ADAPTER_INFO AdapterInfo[16];

	DWORD dwBufLen = sizeof(AdapterInfo);
	DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);

	if(dwStatus != ERROR_SUCCESS)
	{
		return false;
	}
	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
	_IP_ADDR_STRING* pIpAddressList = &pAdapterInfo->IpAddressList;

	while(pAdapterInfo)
	{
		while(pIpAddressList != NULL)
		{
			if( !strcmp(pIpAddressList->IpAddress.String, szIPAddress) ) //IP 동일시 NIC조사
			{
				_TCHAR szNIC[128] = { 0, };
				_TCHAR MuxString[225] = { 0 ,};
				_stprintf(MuxString, REGISTE_STRING, pAdapterInfo->AdapterName);
				SHRegReadString( HKEY_LOCAL_MACHINE, MuxString, _T("Name"), _T("Nothing"), szNIC, 100 );

#ifdef _UNICODE
				WideCharToMultiByte(CP_ACP, 0, szNIC, -1, szNICName, 100, NULL, NULL);
#else
				strcpy(szNICName, szNIC);
#endif
				return true;
			}
			pIpAddressList = pIpAddressList->Next;
		}
		pAdapterInfo = pAdapterInfo->Next;
	}
	return false;
#else

	int iNTriedToCreate;
	CiStrInit(szNICName);

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if ( sockfd < 0 )
	{
		return false;
	}

	struct ifconf ifc;
	struct ifreq ifr[CI_MAX_NIC_COUNT];
    memset(ifr, 0, sizeof(struct ifreq) * CI_MAX_NIC_COUNT);

    ifc.ifc_len = sizeof(struct ifreq) * CI_MAX_NIC_COUNT;
    ifc.ifc_buf = (char *)ifr;

	if ( ioctl(sockfd, SIOCGIFCONF, &ifc) < 0 )
	{
		close(sockfd);
		return false;
	}

	int i;
	int iNicCount = ifc.ifc_len / sizeof(struct ifreq);

	for ( i=0; i<iNicCount; i++ )
	{
		struct sockaddr_in *sin = (struct sockaddr_in *)&ifr[i].ifr_addr;

		if ( !strcmp(szIPAddress, inet_ntoa(sin->sin_addr)) )
		  	CiStrCpy(szNICName, ifr[i].ifr_name, CI_MAX_NAME_LENGTH, &iNTriedToCreate);
	}

	close(sockfd);

	return true;
#endif
}

bool nu_get_ip_from_nic(std::string& szIPAddress, const std::string& szNICName)
{
#ifdef _WIN32
	IP_ADAPTER_INFO AdapterInfo[16];

	DWORD dwBufLen = sizeof(AdapterInfo);
	DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);

	if(dwStatus != ERROR_SUCCESS)
	{
		return false;
	}
	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;

	while (pAdapterInfo)
	{
		_TCHAR getNICName[128] = { 0, };
		_TCHAR MuxString[225] = { 0 ,};
		_stprintf(MuxString, REGISTE_STRING, pAdapterInfo->AdapterName);
		SHRegReadString( HKEY_LOCAL_MACHINE, MuxString, _T("Name"), _T("Nothing"), getNICName, 100 );

		char nic_name[CI_MAX_DESCRIPTION_LENGTH+1];
		nic_name[CI_MAX_DESCRIPTION_LENGTH] = '\0';

#ifdef _UNICODE
		WideCharToMultiByte(CP_ACP, 0, getNICName, -1, nic_name, CI_MAX_DESCRIPTION_LENGTH, NULL, NULL);
#else
		strncpy(nic_name, getNICName, CI_MAX_DESCRIPTION_LENGTH);
#endif
		if (strcmp(szNICName.c_str(), nic_name))
		{
			szIPAddress = pAdapterInfo->IpAddressList.IpAddress.String;
			return true;
		}
		pAdapterInfo = pAdapterInfo->Next;
	}
	return false;
#else
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if ( sockfd < 0 )
	{
		return false;
	}

	struct ifconf ifc;
	struct ifreq ifr[CI_MAX_NIC_COUNT];
	memset(ifr, 0, sizeof(struct ifreq) * CI_MAX_NIC_COUNT);

	ifc.ifc_len = sizeof(struct ifreq) * CI_MAX_NIC_COUNT;
	ifc.ifc_buf = (char *)ifr;

	if ( ioctl(sockfd, SIOCGIFCONF, &ifc) < 0 )
	{
		close(sockfd);
		return false;
	}

	int iNicCount = ifc.ifc_len / sizeof(struct ifreq);

	for ( int i=0; i<iNicCount; i++ )
	{
		if (strcmp(ifr[i].ifr_name, szNICName.c_str()) == 0)
		{
			struct sockaddr_in *sin = (struct sockaddr_in *)&ifr[i].ifr_addr;
			szIPAddress = inet_ntoa(sin->sin_addr);
		}
	}

	close(sockfd);

	return true;
#endif
}

#ifdef _WIN32
#pragma warning(pop)
#endif

