#include "stdafx.h"
#include "CiHBResponser.h"

#include <algorithm>
#include <boost/scoped_array.hpp>
#include "CiSockError.h"

CCiHBResponser::CCiHBResponser(const std::string& szRepresentativeIP,
							   unsigned short iPortNumber,
							   const std::string& szLocalIP,
							   int iTimeoutMillisec/*=0*/)
: m_iTimeoutMillisec(iTimeoutMillisec)
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

#ifndef _WIN32
	m_PollFds = new pollfd[m_iMaxFDSize];
	for ( int i = 0; i < m_iMaxFDSize; i++ )
	{
		m_PollFds[i].fd = NU_INVALID_SOCKET;
		m_PollFds[i].events = 0;
		m_PollFds[i].revents = 0;
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
	std::for_each(m_ReadSockets.begin(), m_ReadSockets.end(), &nu_disconnect);

#ifndef _WIN32
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

	/* add to ReadSocketArray */
	if ( AddReadSocket(sockfd) == false )
	{
		nu_disconnect(sockfd);
		return false;
	}

#ifdef _WIN32
	if ( FDSetAdd( sockfd ) == false )
#else
	if ( PollFDAdd( sockfd ) == false )
#endif
	{
		/* 실패하면 pReadSocket은 destructor의 m_ReadSockets[] 정리 루틴에서 정리된다 */
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

ReceiveIntMessageResult_t CCiHBResponser::ReceiveIntMessage(int sockfd, int *piReceivedMessage)
{
	if ( sockfd == NU_INVALID_SOCKET || piReceivedMessage == NULL )
	{
		return RECEIVE_INT_MESSAGE_FALSE;
	}

#ifdef _WIN32
	int saRecvLen = sizeof(m_saRecv);
#else
	socklen_t saRecvLen = sizeof(m_saRecv);
#endif

	int nRead = recvfrom(sockfd, m_szRecvBuf, 8, 0, &m_saRecv, &saRecvLen);
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// from network thread
//

#ifdef _WIN32
/* for select */
/* fd set manipulation */
bool CCiHBResponser::FDSetAdd(int sockfd)
{
	if ( sockfd == NU_INVALID_SOCKET ) {
		return false;
	}

	FD_SET((unsigned int)sockfd, &m_fdSetAllRead);

	if ( m_iMaximumSocket < sockfd ) {
		m_iMaximumSocket = sockfd;
	}

	return true;
}

bool CCiHBResponser::FDSetDelete(int sockfd)
{
	if ( sockfd == NU_INVALID_SOCKET ) {
		return false;
	}

	FD_CLR((unsigned int)sockfd, &m_fdSetAllRead);

	if ( m_iMaximumSocket <= sockfd ) {
		/* I gave up finding the exact m_iMaximumSocket because it needs */
		/* a linear search. */
		/* I set m_iMaximumSocket to an approximate value instead */
		m_iMaximumSocket = sockfd - 1;
	}

	return true;
}

#else
/* for poll */
/* poll fd manipulation */
bool CCiHBResponser::PollFDAdd(int sockfd)
{
	if ( sockfd == NU_INVALID_SOCKET ) {
		return false;
	}

	// for poll
	if ( m_iPollFdCount >= GetMaxFDSize() ) {
		return false;
	}

	m_PollFds[ m_iPollFdCount ].fd = sockfd;
	m_PollFds[ m_iPollFdCount ].revents = 0;

	m_PollFds[ m_iPollFdCount ].events = POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI;

	m_iPollFdCount++;

	return true;
}

bool CCiHBResponser::PollFDDelete(int sockfd)
{
	if ( sockfd == NU_INVALID_SOCKET ) {
		return false;
	}

	// for poll
	for ( int i = 0; i < m_iPollFdCount; i++ )
	{
		if ( m_PollFds[i].fd == sockfd )
		{
			if ( i !=  ( m_iPollFdCount - 1 ) )
			{
				m_PollFds[ i ].fd = m_PollFds[ m_iPollFdCount - 1 ].fd;
				m_PollFds[ i ].events = m_PollFds[ m_iPollFdCount - 1 ].events;
				m_PollFds[ i ].revents = m_PollFds[ m_iPollFdCount - 1 ].revents;
			}

			m_PollFds[ m_iPollFdCount - 1 ].fd = NU_INVALID_SOCKET;
			m_PollFds[ m_iPollFdCount - 1 ].events = 0;
			m_PollFds[ m_iPollFdCount - 1 ].revents = 0;

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

#endif

bool CCiHBResponser::AddReadSocket(int sockfd)
{
	if ( sockfd == NU_INVALID_SOCKET ) {
		return false;
	}

	m_ReadSockets.insert(sockfd);
	return false;
}

bool CCiHBResponser::DeleteReadSocket(int sockfd)
{
	if ( sockfd == NU_INVALID_SOCKET ) {
		return false;
	}

	m_ReadSockets.erase(sockfd);
	return false;
}

bool CCiHBResponser::ExitInstance()
{
	std::for_each(m_ReadSockets.begin(), m_ReadSockets.end(), &nu_disconnect);
	m_ReadSockets.clear();

#ifdef _WIN32
	FD_ZERO(&m_fdSetAllRead);

	FD_ZERO(&m_fdSetRead);

	m_iMaximumSocket = NU_INVALID_SOCKET;
#endif


#ifndef _WIN32
	for ( int i = 0; i < GetMaxFDSize() ; i++ )
	{
		m_PollFds[i].fd = NU_INVALID_SOCKET;
		m_PollFds[i].events = 0;
		m_PollFds[i].revents = 0;
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
	for (std::set<int>::const_iterator i = m_ReadSockets.begin(); i != m_ReadSockets.end(); ++i)
	{
		if ( *i != NU_INVALID_SOCKET && FD_ISSET(*i, &m_fdSetRead) )
		{
			int iMessage = 0;
			ReceiveIntMessageResult_t rimResult = ReceiveIntMessage(*i, &iMessage);
			if ( rimResult == RECEIVE_INT_MESSAGE_TRUE )
			{
				if ( OnMessage(*i, iMessage) == false )
				{
					OnReadSocketError(*i);
				}
			}
			else if ( rimResult == RECEIVE_INT_MESSAGE_FALSE )
			{
				OnReadSocketError(*i);
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
#else
	for ( int i = 0; i < m_iPollFdCount; i++ )
	{
		if ( m_PollFds[i].fd != NU_INVALID_SOCKET
			&& ( m_PollFds[i].revents & ( POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI )))
		{
			m_PollFds[i].revents = 0;	// CLEAR;

			int iMessage = 0;
			ReceiveIntMessageResult_t rimResult = ReceiveIntMessage(m_PollFds[i].fd, &iMessage);
			if ( rimResult == RECEIVE_INT_MESSAGE_TRUE )
			{
				if ( OnMessage(m_PollFds[i].fd, iMessage) == false )
				{
					OnReadSocketError(m_PollFds[i].fd);
				}
			}
			else if ( rimResult == RECEIVE_INT_MESSAGE_FALSE )
			{
				OnReadSocketError(m_PollFds[i].fd);
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
#endif

	return true;
}
