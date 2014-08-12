// CiHBResponser.cpp: implementation of the CCiHBResponser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CiHBResponser.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCiHBResponser::CCiHBResponser(const char *szRepresentativeIP,
							   unsigned short iPortNumber,
							   const char *szLocalIP,
							   CiThread2Handle_t complexThreadHandle)
: CNetworkThread(NU_INVALID_PORT_NUMBER)
{
	int iNTriedToCreate;

	m_ciThreadHandle = complexThreadHandle;

	m_pSocket = NULL;
	CiStrCpy(m_szRepresentativeIP, szRepresentativeIP, CI_MAX_IP_ADDRESS_LENGTH+1, &iNTriedToCreate);
	m_iPortNumber = iPortNumber;
	CiStrCpy(m_szLocalIP, szLocalIP, CI_MAX_IP_ADDRESS_LENGTH+1, &iNTriedToCreate);

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
		char szString[256];
#ifdef _WIN32
		snprintf(szString, 255, "CCiHBResponser::InitInstance :: create udp socket fail (%d)", GetLastError());
#else
		snprintf(szString, 255, "CCiHBResponser::InitInstance :: create udp socket fail (%s)", strerror(errno));
#endif
		CiUtils_PrintWithTime(stderr, szString);

		return false;
	}


	/* reuse_addr option */
	if ( nu_set_reuse_addr(sockfd, true) == false )
	{
		char szString[256];
		snprintf(szString, 255, "CCiHBResponser::InitInstance :: set_reuse_addr fail (%s)", strerror(errno));
		CiUtils_PrintWithTime(stderr, szString);

		return false;
	}

	/* bind */
	if ( nu_bind(sockfd, m_iPortNumber) == false )
	{
		char szString[256];
#ifdef _WIN32
		snprintf(szString, 255, "CCiHBResponser::InitInstance :: bind(port %d) fail (%d)", m_iPortNumber, GetLastError());
#else
		snprintf(szString, 255, "CCiHBResponser::InitInstance :: bind(port %d) fail (%s)", m_iPortNumber, strerror(errno));
#endif
		CiUtils_PrintWithTime(stderr, szString);

		return false;
	}

	/* create CCiSocket */
	/* CiSocket의 send buffer size와 recv buffer size는 각각 1KB. 실제 사용하지는 않는다. */
	CCiSocket *pReadSocket = new CCiSocket(sockfd, CI_SOCKET_UDP, 1024, 1024);
	if ( pReadSocket == NULL )
	{
		char szString[256];
		snprintf(szString, 255, " (ThHandle %lu) CCiHBResponser::InitInstance :: allocation for CCiSocket fail", GetHandle());
		CiUtils_PrintWithTime(stderr, szString);

#ifdef _WIN32
		if ( closesocket(sockfd) != 0 )
#else
		if ( close(sockfd) < 0 )
#endif
		{
			char szString[256];
#ifdef _WIN32
			snprintf(szString, 255, "CCiHBResponser::InitInstance :: close(sockfd %d) fail (%d)", sockfd, GetLastError());
#else
			snprintf(szString, 255, "CCiHBResponser::InitInstance :: close(sockfd %d) fail (%s)", sockfd, strerror(errno));
#endif
			CiUtils_PrintWithTime(stderr, szString);
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

	m_pSocket = pReadSocket;

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

	CMTime2 mtCurTime;
	m_mtRecvTime = mtCurTime;

#ifdef _WIN32
	int saRecvLen = sizeof(m_saRecv);
#else
	socklen_t saRecvLen = sizeof(m_saRecv);
#endif

	int nRead = recvfrom(pReadSocket->m_iSocket, m_szRecvBuf, 8, 0, &m_saRecv, &saRecvLen);
	if ( nRead < 0 )
	{
		char szString[256];
#ifdef _WIN32
		snprintf(szString, 255, "CCiHBResponser::ReceiveIntMessage :: recvfrom fail (%d)", GetLastError());
#else
		snprintf(szString, 255, "CCiHBResponser::ReceiveIntMessage :: recvfrom fail (%s)", strerror(errno));
#endif
		CiUtils_PrintWithTime(stderr, szString);

		return RECEIVE_INT_MESSAGE_FALSE;
	}

	if ( nRead != 8 )
	{
		char szString[256];
		snprintf(szString, 255, "CCiHBResponser::ReceiveIntMessage :: recvfrom fail (nRead != %d)", 8);
		CiUtils_PrintWithTime(stderr, szString);

		return RECEIVE_INT_MESSAGE_FALSE;
	}

	/* parse message type */
	char* p = m_szRecvBuf;
	int iTemp;
	memcpy((void*)&iTemp, p, 4);
	*piReceivedMessage = ntohl(iTemp);
	p = p + 4;

	/* get peer address from m_saRecv */
	char szPeerIPAddress[CI_MAX_IP_ADDRESS_LENGTH+1];
	unsigned short usPeerPortNumber;
	if ( nu_get_address((struct sockaddr_in *)&m_saRecv, (unsigned char *)szPeerIPAddress, &usPeerPortNumber) == false )
	{
		char szString[256];
		snprintf(szString, 255, "CCiHBResponser::ReceiveIntMessage :: nu_get_address fail");
		CiUtils_PrintWithTime(stderr, szString);

		CiStrInit(szPeerIPAddress);
		usPeerPortNumber = 0;
	}

#ifdef _DEBUG
	char szString[256];
	snprintf(szString, 255,
			"CCiHBResponser::ReceiveIntMessage :: recvfrom (%s:%u)",
			szPeerIPAddress, usPeerPortNumber);
	CiUtils_PrintWithTime(stdout, szString);
#endif

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

			char szString[256];
			snprintf(szString, 255, "CCiHBResponser::OnMessage :: invalid message (%d)", iMessage);
			CiUtils_PrintWithTime(stderr, szString);

			break;
	}

	return true;
}

bool CCiHBResponser::OnHeartbeatRequest(CCiSocket *pReadSocket)
{
	char *p = NULL;

	/* request format : seq_num */
	/* parse request */
	int iSeqNum;
	p = m_szRecvBuf + 4;		/* m_szRecvBuf : MessageType + SeqNum */

	/* now, p points to seq_num */
	int iTemp;
	memcpy((void*)&iTemp, p, 4);
	iSeqNum = ntohl(iTemp);
	p = p + 4;

	return SendHeartbeatResponse(pReadSocket, iSeqNum);
}

bool CCiHBResponser::SendHeartbeatResponse(CCiSocket *pReadSocket, int iSeqNum)
{
	int iMsgStrSize;
	char *pMsgStr = NULL;
	int iIPLength = 0;
	char *p = NULL;
	int iTemp = 0;

	/* response format :
	 * msg_type + seq_num + representative ip length + representative ip + state + local ip length + local ip */

	iMsgStrSize = 4 + 4 + 4 + CiStrLen(m_szRepresentativeIP) + 4 + 4 + CiStrLen(m_szLocalIP);
	pMsgStr = new char[iMsgStrSize];
	p = pMsgStr;

	iTemp = htonl(CIHB_HEARTBEAT_RESPONSE);
	memcpy(p, (void*)&iTemp, 4);
	p = p + 4;

	iTemp = htonl(iSeqNum);
	memcpy(p, (void*)&iTemp, 4);
	p = p + 4;

	iIPLength = CiStrLen(m_szRepresentativeIP);
	iTemp = htonl(iIPLength);
	memcpy(p, (void*)&iTemp, 4);
	p = p + 4;

	memcpy(p, (void*)m_szRepresentativeIP, CiStrLen(m_szRepresentativeIP));
	p = p + CiStrLen(m_szRepresentativeIP);

	iTemp = htonl(m_processState);
	memcpy(p, (void*)&iTemp, 4);
	p = p + 4;

	iIPLength = CiStrLen(m_szLocalIP);
	iTemp = htonl(iIPLength);
	memcpy(p, (void*)&iTemp, 4);
	p = p + 4;

	memcpy(p, (void*)m_szLocalIP, CiStrLen(m_szLocalIP));
	p = p + CiStrLen(m_szLocalIP);

	/* send response */

	int	nSend = sendto(pReadSocket->m_iSocket, pMsgStr, iMsgStrSize, 0, &m_saRecv, sizeof(m_saRecv));
	if ( nSend < 0 )
	{
		char szString[256];
#ifdef _WIN32
		snprintf(szString, 255, "CCiHBResponser::SendHeartbeatResponse :: sendto fail (%d)", GetLastError());
#else
		snprintf(szString, 255, "CCiHBResponser::SendHeartbeatResponse :: sendto fail (%s)", strerror(errno));
#endif
		CiUtils_PrintWithTime(stderr, szString);

		return false;
	}

	if ( nSend != iMsgStrSize )
	{
		char szString[256];
		snprintf(szString, 255,
				"CCiHBResponser::SendHeartbeatResponse :: sendto fail (nSend != %d)", iMsgStrSize);
		CiUtils_PrintWithTime(stderr, szString);

		return false;
	}

	/* get peer address from m_saRecv */
	char szPeerIPAddress[CI_MAX_IP_ADDRESS_LENGTH+1];
	unsigned short usPeerPortNumber;
	if ( nu_get_address((struct sockaddr_in *)&m_saRecv, (unsigned char *)szPeerIPAddress, &usPeerPortNumber) == false )
	{
		char szString[256];
		snprintf(szString, 255,
				"CCiHBResponser::SendHeartbeatResponse :: nu_get_address fail");
		CiUtils_PrintWithTime(stderr, szString);

		CiStrInit(szPeerIPAddress);
		usPeerPortNumber = 0;
	}

#ifdef _DEBUG
	char szString[256];
	snprintf(szString, 255,
			"CCiHBResponser::SendHeartbeatResponse :: %d bytes sendto (%s:%u)",
			iMsgStrSize, szPeerIPAddress, usPeerPortNumber);
	CiUtils_PrintWithTime(stdout, szString);
#endif

	if ( pMsgStr != NULL )
		delete [] pMsgStr;

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
