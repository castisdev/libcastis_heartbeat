#include "CiHBResponser.h"

#include <algorithm>

//////////////////////////////////////////////////////////////////////////
// from netutil
//

#ifndef NU_INVALID_SOCKET
#define NU_INVALID_SOCKET		(-1)
#endif

namespace cihb {
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
}

//////////////////////////////////////////////////////////////////////////

#define NETWORK_THREAD_FD_SETSIZE					(1024)

CCiHBResponser::CCiHBResponser(const std::string& szRepresentativeIP,
							   unsigned short iPortNumber,
							   const std::string& szLocalIP,
							   int iTimeoutMillisec/*=0*/)
: cihb::CNetworkThread(iTimeoutMillisec)
{
	m_szRepresentativeIP = szRepresentativeIP;
	m_iPortNumber = iPortNumber;
	m_szLocalIP = szLocalIP;

	m_processState = CIHB_STATE_ALIVE;
}

CCiHBResponser::~CCiHBResponser()
{
}

bool CCiHBResponser::InitInstance()
{
	if ( cihb::CNetworkThread::InitInstance() == false )
		return false;

	/* create UDP socket */
	int sockfd;
	if ( cihb::nu_create_udp_socket(&sockfd) == false )
	{
		return false;
	}

	/* reuse_addr option */
	if ( cihb::nu_set_reuse_addr(sockfd, true) == false )
	{
		return false;
	}

	/* bind */
	if ( cihb::nu_bind(sockfd, m_iPortNumber) == false )
	{
		return false;
	}

	/* add to ReadSocketArray */
	if ( AddReadSocket(sockfd) == false )
	{
		cihb::nu_disconnect(sockfd);
		return false;
	}

#ifdef _WIN32
	if ( FDSetAdd( sockfd ) == false )
#else
	if ( PollFDAdd( sockfd ) == false )
#endif
	{
		return false;
	}

	return true;
}

bool CCiHBResponser::OnReadSocketError(int /*sockfd*/)
{
  /* connected TCP socket 이 아니므로
	 * socket을 정리하지 않는다. */
	return true;
}

bool CCiHBResponser::OnHeartbeatRequest(int sockfd)
{
	/* request format : seq_num */
	/* parse request */
	char *p = m_szRecvBuf + 4;		/* m_szRecvBuf : MessageType + SeqNum */

	/* now, p points to seq_num */
	int iTemp;
	memcpy((void*)&iTemp, p, 4);
	int iSeqNum = ntohl(iTemp);
	p = p + 4;

	return SendHeartbeatResponse(sockfd, iSeqNum);
}

bool CCiHBResponser::SendHeartbeatResponse(int sockfd, int iSeqNum)
{
	/* response format :
	 * msg_type + seq_num + representative ip length + representative ip + state + local ip length + local ip */

	int iMsgStrSize = 4 + 4 + 4 + m_szRepresentativeIP.length() + 4 + 4 + m_szLocalIP.length();
	boost::scoped_array<char> pMsgStr(new char[iMsgStrSize]);
	char *p = pMsgStr.get();

	int iTemp = htonl(CIHB_HEARTBEAT_RESPONSE);
	memcpy(p, (void*)&iTemp, 4);
	p = p + 4;

	iTemp = htonl(iSeqNum);
	memcpy(p, (void*)&iTemp, 4);
	p = p + 4;

	int iIPLength = m_szRepresentativeIP.length();
	iTemp = htonl(iIPLength);
	memcpy(p, (void*)&iTemp, 4);
	p = p + 4;

	memcpy(p, m_szRepresentativeIP.c_str(), m_szRepresentativeIP.length());
	p = p + m_szRepresentativeIP.length();

	iTemp = htonl(m_processState);
	memcpy(p, (void*)&iTemp, 4);
	p = p + 4;

	iIPLength = m_szLocalIP.length();
	iTemp = htonl(iIPLength);
	memcpy(p, (void*)&iTemp, 4);
	p = p + 4;

	memcpy(p, m_szLocalIP.c_str(), m_szLocalIP.length());
	p = p + m_szLocalIP.length();

	/* send response */

	int	nSend = sendto(sockfd, pMsgStr.get(), iMsgStrSize, 0, &m_saRecv, sizeof(m_saRecv));
	if ( nSend < 0 )
	{
		return false;
	}

	if ( nSend != iMsgStrSize )
	{
		return false;
	}

	return true;
}

CiHBState_t CCiHBResponser::GetProcessState()
{
	return m_processState;
}

void CCiHBResponser::SetProcessState(CiHBState_t state)
{
	m_processState = state;
	return;
}

void CCiHBResponser::SetProcessHWError()
{
	SetProcessState(CIHB_STATE_HW_ERROR);
}

void CCiHBResponser::SetProcessSWError()
{
	SetProcessState(CIHB_STATE_SW_ERROR);
}

void CCiHBResponser::SetProcessAlive()
{
	SetProcessState(CIHB_STATE_ALIVE);
}

bool CCiHBResponser::ReceiveIntMessage(int sockfd, int *piReceivedMessage)
{
	if ( sockfd == NU_INVALID_SOCKET || piReceivedMessage == NULL )
	{
		return false;
	}

#ifdef _WIN32
	int saRecvLen = sizeof(m_saRecv);
#else
	socklen_t saRecvLen = sizeof(m_saRecv);
#endif

	int nRead = recvfrom(sockfd, m_szRecvBuf, 8, 0, &m_saRecv, &saRecvLen);
	if ( nRead < 0 )
	{
		return false;
	}

	if ( nRead != 8 )
	{
		return false;
	}

	/* parse message type */
	char* p = m_szRecvBuf;
	int iTemp;
	memcpy((void*)&iTemp, p, 4);
	*piReceivedMessage = ntohl(iTemp);
	p = p + 4;

	return true;
}

bool CCiHBResponser::OnMessage(int sockfd, int iMessage)
{
	if ( sockfd == NU_INVALID_SOCKET )
		return false;

	switch ( iMessage )
	{
	case CIHB_HEARTBEAT_REQUEST:
		OnHeartbeatRequest(sockfd);
		break;

	default:
		return false;
	}

	return true;
}
