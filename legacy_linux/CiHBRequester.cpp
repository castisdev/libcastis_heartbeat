// CiHBRequester.cpp: implementation of the CCiHBRequester class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CiHBRequester.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCiHBRequester::CCiHBRequester(const char *pszServiceName,
							   const char *pszMyIPAddress,
							   CiThread2Handle_t complexThreadHandle,
							   bool bLogPrint)
: CNetworkThread(NU_INVALID_PORT_NUMBER)
{
	m_ciThreadHandle = complexThreadHandle;

	m_seqNum = 0;

	m_pSocket = NULL;

	int iNTriedToCreate;
	CiStrCpy(m_szServiceName, pszServiceName, CI_MAX_NAME_LENGTH, &iNTriedToCreate);
	CiStrCpy(m_szMyIPAddress, pszMyIPAddress, CI_MAX_IP_ADDRESS_LENGTH, &iNTriedToCreate);

	ResetRequestCount();

	m_bLogPrint = bLogPrint;
}

CCiHBRequester::~CCiHBRequester()
{
	/* m_pSocket은 destructor의 m_ReadSockets[] 정리 루틴에서 정리된다 */
}

bool CCiHBRequester::InitInstance()
{
	if ( CNetworkThread::InitInstance() == false )
		return false;

	/* create UDP socket */
	int sockfd;
	if ( nu_create_udp_socket(&sockfd) == false )
	{
		char szString[256];
#ifdef _WIN32
		snprintf(szString, 255, " (%s) CCiHBRequester::InitInstance :: create udp socket fail (%d)", m_szServiceName, GetLastError());
#else
		snprintf(szString, 255, " (%s) CCiHBRequester::InitInstance :: create udp socket fail (%s)", m_szServiceName, strerror(errno));
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
		snprintf(szString, 255, " (%s) CCiHBRequester::InitInstance :: allocation for CCiSocket fail", m_szServiceName);
		CiUtils_PrintWithTime(stderr, szString);

#ifdef _WIN32
		if ( closesocket(sockfd) != 0 )
#else
		if ( close(sockfd) < 0 )
#endif
		{
			char szString[256];
#ifdef _WIN32
			snprintf(szString, 255, " (%s) CCiHBRequester::InitInstance :: close(sockfd %d) fail (%d)", m_szServiceName, sockfd, GetLastError());
#else
			snprintf(szString, 255, " (%s) CCiHBRequester::InitInstance :: close(sockfd %d) fail (%s)", m_szServiceName, sockfd, strerror(errno));
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

bool CCiHBRequester::ExitInstance()
{
	return CNetworkThread::ExitInstance();
}

bool CCiHBRequester::OnReadSocketError(CCiSocket* /*pSocket*/)
{
	/* connected TCP socket이 아니므로 socket을 정리하지 않는다. */
	return true;
}

ReceiveIntMessageResult_t CCiHBRequester::ReceiveIntMessage(CCiSocket *pReadSocket, int *piReceivedMessage)
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

	int nRead = recvfrom(pReadSocket->m_iSocket, m_szRecvBuf, CIHB_MAX_DATA_SIZE, 0, &m_saRecv, &saRecvLen);
	if ( nRead < 0 )
	{
		char szString[256];
#ifdef _WIN32
		snprintf(szString, 255, " (%s) CCiHBRequester::ReceiveIntMessage :: recvfrom fail (%d)", m_szServiceName, GetLastError());
#else
		snprintf(szString, 255, " (%s) CCiHBRequester::ReceiveIntMessage :: recvfrom fail (%s)", m_szServiceName, strerror(errno));
#endif
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
		snprintf(szString, 255, " (%s) CCiHBRequester::ReceiveIntMessage :: nu_get_address fail", m_szServiceName);
		CiUtils_PrintWithTime(stderr, szString);

		CiStrInit(szPeerIPAddress);
		usPeerPortNumber = 0;
	}

#ifdef _DEBUG
	char szString[256];
	snprintf(szString, 255,
			" (%s) CCiHBRequester::ReceiveIntMessage :: %d bytes recvfrom (%s:%u)",
			m_szServiceName, nRead, szPeerIPAddress, usPeerPortNumber);
	CiUtils_PrintWithTime(stdout, szString);
#endif

	return RECEIVE_INT_MESSAGE_TRUE;
}

bool CCiHBRequester::OnMessage(CCiSocket *pReadSocket, int iMessage)
{
	if ( pReadSocket == NULL )
		return false;

	switch ( iMessage )
	{
		case CIHB_HEARTBEAT_RESPONSE:
			OnHeartbeatResponse();
			break;

		default:
			char szString[256];
			snprintf(szString, 255, " (%s) CCiHBRequester::OnMessage :: invalid message (%d)", m_szServiceName, iMessage);
			CiUtils_PrintWithTime(stderr, szString);
			break;
	}

	return true;
}

bool CCiHBRequester::OnHeartbeatResponse()
{
	return ParseHeartbeatResponse();
}

bool CCiHBRequester::ParseHeartbeatResponse()
{
	/* response format :
	 * seq_num + representative_ip_length + representative_ip + state + local_ip_length + local_ip */

	int iSeqNum, iIPLength, iTemp;
	CiHBState_t state;
	char szRepresentativeIP[CI_MAX_IP_ADDRESS_LENGTH+1];
	char szLocalIP[CI_MAX_IP_ADDRESS_LENGTH+1];
	char *p;

	p = m_szRecvBuf + 4;

	/* now, p points to seq_num */
	memcpy((void*)&iTemp, p, 4);
	iSeqNum = ntohl(iTemp);
	p = p + 4;

	/* now, p points to representative_ip_length */
	memcpy((void*)&iTemp, p, 4);
	iIPLength = ntohl(iTemp);
	p = p + 4;

	/* now, p points to representative_ip */
	memcpy((void*)szRepresentativeIP, p, iIPLength);
	p = p + iIPLength;
	szRepresentativeIP[iIPLength] = '\0';

	/* now, p points to state */
	memcpy((void*)&iTemp, p, 4);
	state = (CiHBState_t)ntohl(iTemp);
	p = p + 4;

	/* now, p points to local_ip_length */
	memcpy((void*)&iTemp, p, 4);
	iIPLength = ntohl(iTemp);
	p = p + 4;

	/* now, p points to local_ip */
	memcpy((void*)szLocalIP, p, iIPLength);
	p = p + iIPLength;
	szLocalIP[iIPLength] = '\0';

	/* check if this turn's HB response */
	if ( iSeqNum != m_seqNum - 1 )
	{
		char szString[256];
		snprintf(szString, 255,
				" (%s) CCiHBRequester::ParseHeartbeatResponse :: not this turn's HB Response (seqnum %d != my seqnum %d)",
				m_szServiceName, iSeqNum, m_seqNum-1);
		CiUtils_PrintWithTime(stderr, szString);
		return true;
	}

	/* get peer address from m_saRecv */
	char szPeerIPAddress[CI_MAX_IP_ADDRESS_LENGTH+1];
	unsigned short usPeerPortNumber;
	if ( nu_get_address((struct sockaddr_in *)&m_saRecv, (unsigned char *)szPeerIPAddress, &usPeerPortNumber) == false )
	{
		char szString[256];
		snprintf(szString, 255,
				" (%s) CCiHBRequester::ParseHeartbeatResponse :: nu_get_address fail",
				m_szServiceName);
		CiUtils_PrintWithTime(stderr, szString);

		CiStrInit(szPeerIPAddress);
		usPeerPortNumber = 0;
	}

	/* find HBAddress object and set HBAddress->m_state to RESPONSED */
	unsigned long ulPosition, ulPositionNext;
	TRAVERSELIST(m_HBAddressList, ulPosition, ulPositionNext)
	{
		CCiHBAddress *pHBAddress = m_HBAddressList.GetCurrentItem(ulPosition);
		if ( pHBAddress == NULL )
			continue;

		if ( !strcmp(pHBAddress->m_szHeartbeatIP, szRepresentativeIP)
				&& pHBAddress->m_usHeartbeatPort == usPeerPortNumber )
		{
			if ( m_bLogPrint == true )
			{
				char szString[256];
				snprintf(szString, 255, " (%s) CCiHBRequester::ParseHeartbeatResponse :: (%s:%u) responsed (state %d)",
						m_szServiceName, pHBAddress->m_szHeartbeatIP, pHBAddress->m_usHeartbeatPort, state);
				CiUtils_PrintWithTime(stdout, szString);
			}

			if ( state == CIHB_STATE_ALIVE )
			{
				pHBAddress->m_state = CIHB_REQUEST_STATE_RESPONSED_ALIVE;
			}
			else if ( state == CIHB_STATE_SW_ERROR && !strcmp(szLocalIP, m_szMyIPAddress) )
			{
				pHBAddress->m_state = CIHB_REQUEST_STATE_RESPONSED_MY_SW_ERROR;
				if ( m_bLogPrint == true )
				{
					char szString[256];
					snprintf(szString, 255, " (%s) CCiHBRequester::ParseHeartbeatResponse :: (%s:%u) responsed (state SW_ERROR(%d))",
							m_szServiceName, pHBAddress->m_szHeartbeatIP, pHBAddress->m_usHeartbeatPort, state);
					CiUtils_PrintWithTime(stdout, szString);
				}
			}
			else if ( state == CIHB_STATE_HW_ERROR && !strcmp(szLocalIP, m_szMyIPAddress) )
			{
				pHBAddress->m_state = CIHB_REQUEST_STATE_RESPONSED_MY_HW_ERROR;
				if ( m_bLogPrint == true )
				{
					char szString[256];
					snprintf(szString, 255, " (%s) CCiHBRequester::ParseHeartbeatResponse :: (%s:%u) responsed (state HW_ERROR(%d))",
							m_szServiceName, pHBAddress->m_szHeartbeatIP, pHBAddress->m_usHeartbeatPort, state);
					CiUtils_PrintWithTime(stdout, szString);
				}
			}
			else
				pHBAddress->m_state = CIHB_REQUEST_STATE_RESPONSED_ALIVE;

			break;
		}
	}

	return true;
}

bool CCiHBRequester::SendRequest()
{
	/* format : MsgType + seqNum */

	char msgStr[8];
	char *p = msgStr;
	int iTemp;

	iTemp = htonl(CIHB_HEARTBEAT_REQUEST);
	memcpy(p, (void*)&iTemp, 4);
	p = p + 4;

	int iSeqNum = m_seqNum++;
	if ( m_seqNum == 0x7FFFFFFF ) {
		m_seqNum = 0;
	}
	iTemp = htonl(iSeqNum);
	memcpy(p, (void*)&iTemp, 4);
	p = p + 4;

	/* send to all Processes */
	int nSend;
	unsigned long ulPosition, ulPositionNext;
	TRAVERSELIST(m_HBAddressList, ulPosition, ulPositionNext)
	{
		CCiHBAddress *pAddress = (CCiHBAddress *)m_HBAddressList.GetCurrentItem(ulPosition);
		if ( pAddress == NULL )
			continue;

		if ( nu_sendto( m_pSocket->m_iSocket,
						msgStr,
						8,
						pAddress->m_szHeartbeatIP,
						pAddress->m_usHeartbeatPort,
						&nSend ) == false )
		{
			char szString[256];
#ifdef _WIN32
			snprintf(szString, 255, " (%s) CCiHBRequester::SendRequest :: sendto (%s:%d) fail (%d)",
					m_szServiceName, pAddress->m_szHeartbeatIP, pAddress->m_usHeartbeatPort, GetLastError());
#else
			snprintf(szString, 255, " (%s) CCiHBRequester::SendRequest :: sendto (%s:%d) fail (%s)",
					m_szServiceName, pAddress->m_szHeartbeatIP, pAddress->m_usHeartbeatPort, strerror(errno));
#endif
			CiUtils_PrintWithTime(stderr, szString);

			return false;
		}

		if ( nSend != 8 )
		{
			char szString[256];
			snprintf(szString, 255, " (%s) CCiHBRequester::SendRequest :: sendto (%s:%d) fail (nSend != %d)",
					m_szServiceName, pAddress->m_szHeartbeatIP, pAddress->m_usHeartbeatPort, 8);
			CiUtils_PrintWithTime(stderr, szString);

			return false;
		}

		pAddress->m_state = CIHB_REQUEST_STATE_REQUESTED;

#ifdef _DEBUG
		char szString[256];
		snprintf(szString, 255, " (%s) CCiHBRequester::SendRequest :: sendto (%s:%u) : seqnum %d",
				m_szServiceName, pAddress->m_szHeartbeatIP, pAddress->m_usHeartbeatPort, iSeqNum);
		CiUtils_PrintWithTime(stdout, szString);
#endif
	}

	CMTime2 mtCurTime;
	m_mtRequestTime = mtCurTime;

	return true;
}

bool CCiHBRequester::AddHBAddress(const char *pszIPAddress, unsigned short usPortNumber)
{
	CCiHBAddress *pHBAddress = new CCiHBAddress(pszIPAddress, usPortNumber);
	if ( pHBAddress == NULL )
	{
		char szString[256];
		snprintf(szString, 255, " (%s) CCiHBRequester::AddHBAddress :: Alloc fail", m_szServiceName);
		CiUtils_PrintWithTime(stderr, szString);
		return false;
	}

	if ( m_HBAddressList.AddItem(pHBAddress) == false )
	{
		char szString[256];
		snprintf(szString, 255, " (%s) CCiHBRequester::AddHBAddress :: AddItem (IP %s, Port %u) fail",
				m_szServiceName, pHBAddress->m_szHeartbeatIP, pHBAddress->m_usHeartbeatPort);
		CiUtils_PrintWithTime(stderr, szString);

		delete pHBAddress;
		return false;
	}

	return true;
}

bool CCiHBRequester::IsAllResponseArrived()
{
	unsigned long ulPosition, ulPositionNext;
	TRAVERSELIST(m_HBAddressList, ulPosition, ulPositionNext)
	{
		CCiHBAddress *pHBAddress = m_HBAddressList.GetCurrentItem(ulPosition);
		if ( pHBAddress == NULL )
			continue;

		if ( pHBAddress->m_state == CIHB_REQUEST_STATE_INIT
			|| pHBAddress->m_state == CIHB_REQUEST_STATE_REQUESTED )
		{
			return false;
		}
	}

	return true;
}

bool CCiHBRequester::IsResponseMyHWError()
{
	unsigned long ulPosition, ulPositionNext;
	TRAVERSELIST(m_HBAddressList, ulPosition, ulPositionNext)
	{
		CCiHBAddress *pHBAddress = m_HBAddressList.GetCurrentItem(ulPosition);
		if ( pHBAddress == NULL )
			continue;

		if ( pHBAddress->m_state == CIHB_REQUEST_STATE_RESPONSED_MY_HW_ERROR )
		{
			return true;
		}
	}

	return false;
}

bool CCiHBRequester::IsResponseMySWError()
{
	unsigned long ulPosition, ulPositionNext;
	TRAVERSELIST(m_HBAddressList, ulPosition, ulPositionNext)
	{
		CCiHBAddress *pHBAddress = m_HBAddressList.GetCurrentItem(ulPosition);
		if ( pHBAddress == NULL )
			continue;

		if ( pHBAddress->m_state == CIHB_REQUEST_STATE_RESPONSED_MY_SW_ERROR )
		{
			return true;
		}
	}

	return false;
}

void CCiHBRequester::ResetAllResponseArrived()
{
	unsigned long ulPosition, ulPositionNext;
	TRAVERSELIST(m_HBAddressList, ulPosition, ulPositionNext)
	{
		CCiHBAddress *pHBAddress = m_HBAddressList.GetCurrentItem(ulPosition);
		if ( pHBAddress == NULL )
			continue;

		pHBAddress->ResetState();
	}
}

void CCiHBRequester::ResetRequestTime()
{
	CMTime2 mtCurTime;
	m_mtRequestTime = mtCurTime;
}

bool CCiHBRequester::IsRequestTimeOut(int iTimeOutMSec)
{
	CMTime2 mtCurTime;
	CMTimeSpan2 mtTimeDiff = mtCurTime - m_mtRequestTime;

	CiLongLong_t llDiffMSecs;
	mtTimeDiff.GetTimeSpan(llDiffMSecs);

	if ( llDiffMSecs >= iTimeOutMSec )
		return true;
	else
		return false;
}

void CCiHBRequester::ResetRequestCount()
{
	m_iRequestCount = 0;
}

int CCiHBRequester::GetRequestCount()
{
	return m_iRequestCount;
}

void CCiHBRequester::IncreaseRequestCount()
{
	m_iRequestCount++;
}
