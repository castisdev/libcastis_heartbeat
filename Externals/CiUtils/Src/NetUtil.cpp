#include "internal_CiUtils.h"	/* CiUtils.h includes CiGlobals.h hence windows.h */

#include "NetUtil.h"

/* KSH for FIONBIO */
#ifndef _WIN32
#include <sys/ioctl.h>
#include <net/if.h>
#endif

#ifndef _WIN32
#include <sys/poll.h>
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

bool nu_get_local_port_number(int binded_socket, unsigned short *port_out)
{
	struct sockaddr_in local_addr;
	socklen_t addrlen;

	addrlen = sizeof(struct sockaddr_in);
	if ( getsockname(binded_socket, (struct sockaddr *)&local_addr, &addrlen) < 0 )
	{
		return false;
	}

	*port_out = ntohs(local_addr.sin_port);

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
