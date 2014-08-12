#include "stdafx.h"
#include "CiHBResponser.h"

#include <boost/scoped_array.hpp>
#include "CiSockError.h"

CCiHBResponser::CCiHBResponser(const std::string& szRepresentativeIP,
							   unsigned short iPortNumber,
							   const std::string& szLocalIP,
							   int iTimeoutMillisec/*=0*/)
: m_ReadSockets(NULL)
, m_iTimeoutMillisec(iTimeoutMillisec)
, m_iMaxFDSize(NETWORK_THREAD_FD_SETSIZE)
{
	//////////////////////////////////////////////////////////////////////////
	// from network thread
	//
#ifdef _WIN32
	FD_ZERO(&m_fdSetAllRead);
	FD_ZERO(&m_fdSetRead);
	m_iMaximumSocket = NU_INVALID_SOCKET;
#endif

	m_ReadSockets = new CCiSocket*[m_iMaxFDSize];

	for ( int i = 0; i < m_iMaxFDSize; i++ )
	{
		m_ReadSockets[i] = NULL;
	}

#ifndef _WIN32
	m_PollFds = new pollfd[m_iMaxFDSize];
	m_PollSockets = new CCiSocket*[m_iMaxFDSize];
	for ( int i = 0; i < m_iMaxFDSize; i++ )
	{
		m_PollFds[i].fd = NU_INVALID_SOCKET;
		m_PollFds[i].events = 0;
		m_PollFds[i].revents = 0;

		m_PollSockets[i] = NULL;
	}

	m_iPollFdCount = 0;
#endif

	// Events Count for immediate break to save the time
	m_iEventsCount = 0;

	//
	//////////////////////////////////////////////////////////////////////////

	m_szRepresentativeIP = szRepresentativeIP;
	m_iPortNumber = iPortNumber;
	m_szLocalIP = szLocalIP;

	m_processState = CIHB_STATE_ALIVE;
}

CCiHBResponser::~CCiHBResponser()
{
	/* m_pSocket은 destructor의 m_ReadSockets[] 정리 루틴에서 정리된다 */

	//////////////////////////////////////////////////////////////////////////
	// from network thread
	//
	int iMaxFDSize = GetMaxFDSize();
	for ( int i = 0; i < iMaxFDSize ; i++ )
	{
		if ( m_ReadSockets[i] != NULL )
		{
			m_ReadSockets[i]->Disconnect();
			delete m_ReadSockets[i];
			m_ReadSockets[i] = NULL;
		}
	}
	delete[] m_ReadSockets;

#ifndef _WIN32
	for (int i=0; i<iMaxFDSize; i++)
	{
		if( m_PollSockets[i] != NULL)
		{
			delete m_PollSockets[i];
			m_PollSockets[i] = NULL;
		}
	}

	delete[] m_PollSockets;	
	delete[] m_PollFds;
#endif
}

bool CCiHBResponser::InitInstance()
{
	if ( CCiThread2::InitInstance() == false )
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

bool CCiHBResponser::OnReadSocketError(CCiSocket* /*pSocket*/)
{
	/* connected TCP socket 이 아니므로
	 * socket을 정리하지 않는다. */
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// from network thread
//

#ifdef _WIN32
/* for select */
/* fd set manipulation */
bool CCiHBResponser::FDSetAdd(CCiSocket* pSocket)
{
	if ( pSocket == NULL ) {
		return false;
	}

	int iSocketFD = pSocket->m_iSocket;

	if ( iSocketFD == NU_INVALID_SOCKET ) {
		return false;
	}

	FD_SET(iSocketFD, &m_fdSetAllRead);

	if ( m_iMaximumSocket < iSocketFD ) {
		m_iMaximumSocket = iSocketFD;
	}

	return true;
}

bool CCiHBResponser::FDSetDelete(CCiSocket* pSocket)
{
	if ( pSocket == NULL ) {
		return false;
	}

	int iSocketFD = pSocket->m_iSocket;

	if ( iSocketFD == NU_INVALID_SOCKET ) {
		return false;
	}

	FD_CLR((unsigned int)iSocketFD, &m_fdSetAllRead);

	if ( m_iMaximumSocket <= iSocketFD ) {
		/* I gave up finding the exact m_iMaximumSocket because it needs */
		/* a linear search. */
		/* I set m_iMaximumSocket to an approximate value instead */
		m_iMaximumSocket = iSocketFD - 1;
	}

	return true;
}

#else
/* for poll */
/* poll fd manipulation */
bool CCiHBResponser::PollFDAdd(CCiSocket *pSocket)
{
	if ( pSocket == NULL ) {
		return false;
	}

	int iSocketFD = pSocket->m_iSocket;

	if ( iSocketFD == NU_INVALID_SOCKET ) {
		return false;
	}

	// for poll
	if ( m_iPollFdCount >= GetMaxFDSize() ) {
		return false;
	}

	m_PollSockets[ m_iPollFdCount ] = pSocket;

	m_PollFds[ m_iPollFdCount ].fd = iSocketFD;
	m_PollFds[ m_iPollFdCount ].revents = 0;

	m_PollFds[ m_iPollFdCount ].events = POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI;

	m_iPollFdCount++;

	return true;
}

bool CCiHBResponser::PollFDDelete(CCiSocket *pSocket)
{
	if ( pSocket == NULL ) {
		return false;
	}

	int iSocketFD = pSocket->m_iSocket;

	if ( iSocketFD == NU_INVALID_SOCKET ) {
		return false;
	}

	// for poll
	for ( int i = 0; i < m_iPollFdCount; i++ )
	{
		if ( m_PollFds[i].fd == iSocketFD )
		{
			if ( i !=  ( m_iPollFdCount - 1 ) )
			{
				m_PollFds[ i ].fd = m_PollFds[ m_iPollFdCount - 1 ].fd;
				m_PollFds[ i ].events = m_PollFds[ m_iPollFdCount - 1 ].events;
				m_PollFds[ i ].revents = m_PollFds[ m_iPollFdCount - 1 ].revents;

				/* 2003.08.22. added by nuri */
				m_PollSockets[ i ] = m_PollSockets[ m_iPollFdCount - 1 ];
			}

			m_PollFds[ m_iPollFdCount - 1 ].fd = NU_INVALID_SOCKET;
			m_PollFds[ m_iPollFdCount - 1 ].events = 0;
			m_PollFds[ m_iPollFdCount - 1 ].revents = 0;

			m_PollSockets[ m_iPollFdCount - 1 ] = NULL;

			m_iPollFdCount --;

			return true;
		}
	}

	return false;
}

bool CCiHBResponser::PollFDEnable(pollfd *pPollFD)
{
	if ( pPollFD == NULL ) {
		return false;
	}

	if ( pPollFD->fd == NU_INVALID_SOCKET ) {
		return false;
	}

	pPollFD->events |= (POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI);

	return true;
}

bool CCiHBResponser::PollFDDisable(pollfd *pPollFD)
{
	if ( pPollFD == NULL ) {
		return false;
	}

	if ( pPollFD->fd == NU_INVALID_SOCKET ) {
		return false;
	}

	pPollFD->events &= ~(POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI);

	return true;
}

pollfd *CCiHBResponser::FindPollFD(CCiSocket *pSocket)
{
	if ( pSocket == NULL || pSocket->m_iSocket == NU_INVALID_SOCKET ) {
		return NULL;
	}

	for ( int i = 0; i < m_iPollFdCount; i++ ) {
		if ( m_PollFds[i].fd == pSocket->m_iSocket ) {
			return &m_PollFds[i];
		}
	}

	return NULL;
}

#endif

bool CCiHBResponser::AddReadSocket(CCiSocket* pReadSocket)
{
	if ( pReadSocket == NULL ) {
		return false;
	}

	int iMaxFDSize = GetMaxFDSize();
	for ( int i = 0; i < iMaxFDSize; i++ )
	{
		if ( m_ReadSockets[i] == NULL )
		{
			m_ReadSockets[i] = pReadSocket;
			return true;
		}
	}

	return false;
}

bool CCiHBResponser::DeleteReadSocket(CCiSocket* pReadSocket)
{
	if ( pReadSocket == NULL ) {
		return false;
	}

	int iMaxFDSize = GetMaxFDSize();
	for ( int i = 0; i < iMaxFDSize; i++ )
	{
		if ( m_ReadSockets[i] == pReadSocket )
		{
			delete pReadSocket;
			m_ReadSockets[i] = NULL;
			return true;
		}
	}

	return false;
}

/* disable a socket in case of internal errors */
bool CCiHBResponser::DisableSocket(CCiSocket* pSocket)
{
	/* Disabling a socket means that it will be delete from fd set */
	/* and disconnected buf is not be deleted from the socket set */

	if ( pSocket == NULL ) {
		return false;
	}

#ifdef _WIN32
	FDSetDelete(pSocket);	/* from read socket array */
#else
	PollFDDelete(pSocket);
#endif

	pSocket->Disconnect();

	return true;
}

bool CCiHBResponser::ExitInstance()
{
	int iMaxFDSize = GetMaxFDSize();
	for ( int i = 0; i < iMaxFDSize ; i++ )
	{
		if ( m_ReadSockets[i] != NULL )
		{
			m_ReadSockets[i]->Disconnect();
			delete m_ReadSockets[i];
			m_ReadSockets[i] = NULL;
		}
	}

#ifdef _WIN32
	FD_ZERO(&m_fdSetAllRead);

	FD_ZERO(&m_fdSetRead);

	m_iMaximumSocket = NU_INVALID_SOCKET;
#endif


#ifndef _WIN32
	for ( int i = 0; i < iMaxFDSize ; i++ )
	{
		m_PollFds[i].fd = NU_INVALID_SOCKET;
		m_PollFds[i].events = 0;
		m_PollFds[i].revents = 0;

		m_PollSockets[i] = NULL;
	}

	m_iPollFdCount = 0;
#endif

	// Events Count for immediate break to save the time
	m_iEventsCount = 0;

	return CCiThread2::ExitInstance();
}

bool CCiHBResponser::Run()
{
	/* In case of select version */
	/* fd_set's are internally managed with member variables, m_fdSetAllRead, m_fdSetAllWrite, */
	/* m_fdSetRead, m_fdSetWrite */
	if ( !m_bExit && WaitNetworkEvent(&m_iEventsCount) == true )
	{
		if ( !m_bExit && m_iEventsCount > 0 )
		{
			ProcessReadEvent();
		}
	}

	return true;
}

// overridables
bool CCiHBResponser::WaitNetworkEvent(int *piNEvent)
{
#ifdef _WIN32
	struct timeval timeout = { m_iTimeoutMillisec/1000, (m_iTimeoutMillisec%1000)*1000 };

	// clear the fd_set
	FD_ZERO(&m_fdSetRead);

	m_fdSetRead = m_fdSetAllRead;

	int iNEvent = select(m_iMaximumSocket+1, &m_fdSetRead, NULL, NULL, &timeout);
#else
	int iNEvent = poll(m_PollFds, m_iPollFdCount, m_iTimeoutMillisec);
#endif

	if ( iNEvent < 0 )
	{
		if ( errno == CI_EINTR )
		{
			if ( piNEvent != NULL )
			{
				*piNEvent = 0;
			}

			return false;
		}
		else
			castis::msleep(m_iTimeoutMillisec);
	}

	if ( piNEvent != NULL )
	{
		*piNEvent = iNEvent;
	}

	return true;
}

bool CCiHBResponser::ProcessReadEvent()
{
#ifdef _WIN32
	int iMaxFDSize = GetMaxFDSize();
	for ( int i = 0; i < iMaxFDSize; i++ )
	{
		if ( m_ReadSockets[i] != NULL
			&& m_ReadSockets[i]->m_iSocket != NU_INVALID_SOCKET
			&& FD_ISSET(m_ReadSockets[i]->m_iSocket, &m_fdSetRead) )
		{
			CCiSocket* pSocket = m_ReadSockets[i];
#else
	for ( int i = 0; i < m_iPollFdCount; i++ )
	{
		if ( m_PollFds[i].fd != NU_INVALID_SOCKET
			&& ( m_PollFds[i].revents & ( POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI )))
		{
			m_PollFds[i].revents = 0;	// CLEAR;

			CCiSocket* pSocket = m_PollSockets[i];
#endif

			int iMessage;
			ReceiveIntMessageResult_t rimResult = ReceiveIntMessage(pSocket, &iMessage);
			if ( rimResult == RECEIVE_INT_MESSAGE_TRUE )
			{
				if ( OnMessage(pSocket, iMessage) == false )
				{
#ifdef _WIN32
					if ( m_ReadSockets[i] != NULL && m_ReadSockets[i] == pSocket )
#else
					if ( m_PollSockets[i] != NULL && m_PollSockets[i] == pSocket )
#endif
						OnReadSocketError(pSocket);
				}
			}
			else if ( rimResult == RECEIVE_INT_MESSAGE_FALSE )
			{
#ifdef _WIN32
				if ( m_ReadSockets[i] != NULL && m_ReadSockets[i] == pSocket )
#else
				if ( m_PollSockets[i] != NULL && m_PollSockets[i] == pSocket )
#endif
					OnReadSocketError(pSocket);
			}
			else {	/* rimResult == RECEIVE_INT_MESSAGE_INTERIM */
				/* interim message */
				/* do nothing just go ahead */
			}

			if ( --m_iEventsCount <= 0 )
			{
				break;
			}
		}
	}

	return true;
}
