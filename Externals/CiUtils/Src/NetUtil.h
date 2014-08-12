// netutil.h: interface for the netutil library.
//
//////////////////////////////////////////////////////////////////////

#ifndef __NETUTIL_H__
#define __NETUTIL_H__

#include <string>
#include <vector>
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
bool nu_create_tcp_socket(int *sock_out);
bool nu_create_udp_socket(int *sock_out);

bool nu_connect(int sock, const char *dest_addr, unsigned short port);
bool nu_NonBlockConnect( int iSocketFD, const char* pszServerIPAddress, int iPortNumber, unsigned int uiTimeOUTInSeconds );

bool nu_disconnect(int sock);

bool nu_accept(int listen_socket, int *sock_out,
				char *szClientIP, unsigned short *pusClientPort);
bool nu_bind(int sock, unsigned short port);

bool nu_sendto(int sock, const void *buf, int ntowrite, const char *dest_addr, unsigned short port, int *nwritten);
bool nu_recvfrom(int sock, void *buf, int ntoread, int *nread);

bool nu_writen(int sock, const void *buf, int ntowrite, int *nwritten);
bool nu_readn(int sock, void *buf, int ntoread, int *nread);

bool nu_read_int8(int sock, char *int8);
bool nu_read_int16(int sock, short *int16);
bool nu_read_int32(int sock, int *int32);
bool nu_read_int64(int sock, long long *int64);

bool nu_write_int8(int sock, char int8);
bool nu_write_int16(int sock, short int16);
bool nu_write_int32(int sock, int int32);
bool nu_write_int64(int sock, long long int64);

bool nu_udp_read_int8(int sock, char *int8);
bool nu_udp_read_int16(int sock, short *int16);
bool nu_udp_read_int32(int sock, int *int32);
bool nu_udp_read_int64(int sock, long long *int64);

bool nu_udp_write_int8(int sock, char int8, const char *dest_addr, unsigned short port);
bool nu_udp_write_int16(int sock, short int16, const char *dest_addr, unsigned short port);
bool nu_udp_write_int32(int sock, int int32, const char *dest_addr, unsigned short port);
bool nu_udp_write_int64(int sock, long long int64, const char *dest_addr, unsigned short port);

bool nu_get_local_ip_address(int connected_socket, unsigned char *ip_address_out);
bool nu_get_local_port_number(int binded_socket, unsigned short *port_out);

// 2003.4.29 Hawke
bool nu_get_peer_ip_address(int connected_socket, unsigned char *ip_address_out);

// by sinma 2001.11.21
// You can use these functions when you need time out for asyncsocket..
bool nu_writen_async(int sock, const void *buf, int ntowrite, int *nwritten, int iHundredMillisecondTimeOut);
bool nu_readn_async(int sock, void *buf, int ntoread, int *nread, int iHundredMillisecondTimeOut);

bool nu_read_int8_async(int sock, char *int8, int iHundredMillisecondTimeOut);
bool nu_read_int16_async(int sock, short *int16, int iHundredMillisecondTimeOut);
bool nu_read_int32_async(int sock, int *int32, int iHundredMillisecondTimeOut);
bool nu_read_int64_async(int sock, long long *int64, int iHundredMillisecondTimeOut);

bool nu_write_int8_async(int sock, char int8, int iHundredMillisecondTimeOut);
bool nu_write_int16_async(int sock, short int16, int iHundredMillisecondTimeOut);
bool nu_write_int32_async(int sock, int int32, int iHundredMillisecondTimeOut);
bool nu_write_int64_async(int sock, long long int64, int iHundredMillisecondTimeOut);

bool nu_set_send_timeout(int sock, int iTimeoutSecond);
bool nu_set_recv_timeout(int sock, int iTimeoutSecond);
bool nu_set_keep_alive(int sock, bool bEnable);
bool nu_set_nonblock(int sock, bool bEnable);
bool nu_set_reuse_addr(int sock, bool bEnable);
bool nu_set_revbuf(int sock, int iBufferSize);
bool nu_set_sndbuf(int sock, int iBufferSize);
bool nu_get_rcvbuf(int sock, int *piBufferSize);
bool nu_get_sndbuf(int sock, int *piBufferSize);

// by nuri 2006.09.01
bool nu_get_address(const struct sockaddr_in *sockaddr_in, unsigned char *ip_address_out, unsigned short *port_out);
bool nu_get_my_ip_address_list(std::vector<std::string>& ips);
bool nu_get_my_nic_list(std::vector<std::string>& nics);
bool nu_get_mac_address(const char *ifname, unsigned char *addr_out);
bool nu_get_nic_from_ip(char *szNICName, const char *szIPAddress);
bool nu_get_ip_from_nic(std::string& szIPAddress, const std::string& szNICName);

#endif
