#include "stdafx.h"
#include "CiHBResponser.h"

#include <boost/scoped_array.hpp>

CCiHBResponser::CCiHBResponser(const std::string& szRepresentativeIP,
							   unsigned short iPortNumber,
							   const std::string& szLocalIP)
{
	m_szRepresentativeIP = szRepresentativeIP;
	m_iPortNumber = iPortNumber;
	m_szLocalIP = szLocalIP;

	m_processState = CIHB_STATE_ALIVE;
}

CCiHBResponser::~CCiHBResponser()
{
	/* m_pSocket은 destructor의 m_ReadSockets[] 정리 루틴에서 정리된다 */
}

bool CCiHBResponser::InitInstance()
{
	if ( CNetworkThread::InitInstance() == false )
		return false;

	/* create UDP socket */
	int sockfd;
	if ( nu_create_udp_socket(&sockfd) == false )
	{
		return false;
	}

	/* reuse_addr option */
	if ( nu_set_reuse_addr(sockfd, true) == false )
	{
		return false;
	}

	/* bind */
	if ( nu_bind(sockfd, m_iPortNumber) == false )
	{
		return false;
	}

	/* create CCiSocket */
	/* CiSocket의 send buffer size와 recv buffer size는 각각 1KB. 실제 사용하지는 않는다. */
	CCiSocket *pReadSocket = new CCiSocket(sockfd, CI_SOCKET_UDP, 1024, 1024);
	if ( pReadSocket == NULL )
	{
#ifdef _WIN32
		if ( closesocket(sockfd) != 0 )
#else
		if ( close(sockfd) < 0 )
#endif
		{
			// do nothing
		}

		return false;
	}

	/* add to ReadSocketArray */
	if ( AddReadSocket(pReadSocket) == false )
	{
		pReadSocket->Disconnect();
		delete pReadSocket;
		pReadSocket = NULL;

		return false;
	}

#ifdef _WIN32
	if ( FDSetAdd( pReadSocket ) == false )
#else
	if ( PollFDAdd( pReadSocket ) == false )
#endif
	{
		/* 실패하면 pReadSocket은 destructor의 m_ReadSockets[] 정리 루틴에서 정리된다 */
		return false;
	}

	return true;
}

bool CCiHBResponser::ExitInstance()
{
	return CNetworkThread::ExitInstance();
}

bool CCiHBResponser::OnReadSocketError(CCiSocket* /*pSocket*/)
{
	/* connected TCP socket 이 아니므로
	 * socket을 정리하지 않는다. */
	return true;
}

ReceiveIntMessageResult_t CCiHBResponser::ReceiveIntMessage(CCiSocket *pReadSocket, int *piReceivedMessage)
{
	if ( pReadSocket == NULL || piReceivedMessage == NULL )
	{
		return RECEIVE_INT_MESSAGE_FALSE;
	}

#ifdef _WIN32
	int saRecvLen = sizeof(m_saRecv);
#else
	socklen_t saRecvLen = sizeof(m_saRecv);
#endif

	int nRead = recvfrom(pReadSocket->m_iSocket, m_szRecvBuf, 8, 0, &m_saRecv, &saRecvLen);
	if ( nRead < 0 )
	{
		return RECEIVE_INT_MESSAGE_FALSE;
	}

	if ( nRead != 8 )
	{
		return RECEIVE_INT_MESSAGE_FALSE;
	}

	/* parse message type */
	char* p = m_szRecvBuf;
	int iTemp;
	memcpy((void*)&iTemp, p, 4);
	*piReceivedMessage = ntohl(iTemp);
	p = p + 4;

	return RECEIVE_INT_MESSAGE_TRUE;
}

bool CCiHBResponser::OnMessage(CCiSocket *pReadSocket, int iMessage)
{
	if ( pReadSocket == NULL )
		return false;

	switch ( iMessage )
	{
		case CIHB_HEARTBEAT_REQUEST:
			OnHeartbeatRequest(pReadSocket);
			break;

		default:
			return false;
	}

	return true;
}

bool CCiHBResponser::OnHeartbeatRequest(CCiSocket *pReadSocket)
{
	/* request format : seq_num */
	/* parse request */
	char *p = m_szRecvBuf + 4;		/* m_szRecvBuf : MessageType + SeqNum */

	/* now, p points to seq_num */
	int iTemp;
	memcpy((void*)&iTemp, p, 4);
	int iSeqNum = ntohl(iTemp);
	p = p + 4;

	return SendHeartbeatResponse(pReadSocket, iSeqNum);
}

bool CCiHBResponser::SendHeartbeatResponse(CCiSocket *pReadSocket, int iSeqNum)
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

	int	nSend = sendto(pReadSocket->m_iSocket, pMsgStr.get(), iMsgStrSize, 0, &m_saRecv, sizeof(m_saRecv));
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
