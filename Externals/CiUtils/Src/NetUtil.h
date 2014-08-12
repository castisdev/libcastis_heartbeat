// netutil.h: interface for the netutil library.
//
//////////////////////////////////////////////////////////////////////

#ifndef __NETUTIL_H__
#define __NETUTIL_H__

#include <errno.h>

#ifndef NU_INVALID_SOCKET
#define NU_INVALID_SOCKET		(-1)
#endif

#ifndef NU_INVALID_PORT_NUMBER
#define NU_INVALID_PORT_NUMBER	(-1)
#endif

#ifdef _WIN32
bool nu_initialize_winsock2();
bool nu_finalize_winsock2();
#endif

int nu_get_last_error();
bool nu_create_listen_socket(int *sock_out, unsigned short port, int listen_queue, const char* listen_ipaddr = "0.0.0.0");
bool nu_create_udp_socket(int *sock_out);

bool nu_disconnect(int sock);

bool nu_accept(int listen_socket, int *sock_out,
				char *szClientIP, unsigned short *pusClientPort);
bool nu_bind(int sock, unsigned short port);

bool nu_get_local_port_number(int binded_socket, unsigned short *port_out);

bool nu_set_keep_alive(int sock, bool bEnable);
bool nu_set_reuse_addr(int sock, bool bEnable);

#endif
