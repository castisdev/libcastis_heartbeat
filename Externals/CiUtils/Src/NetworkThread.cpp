// NetworkThread.cpp: implementation of the CNetworkThread class.
//
//////////////////////////////////////////////////////////////////////

#include "internal_CiUtils.h"
#include "NetUtil.h"
#include "NetworkThread.h"
#include "CiSocket.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

#ifdef _WIN32
#pragma warning(disable : 4244 4389)
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNetworkThread::CNetworkThread()
: m_ReadSockets(NULL)
, m_iConnectedSocketSendBufferSize(CI_SOCKET_SEND_BUFFER_SIZE)
, m_iConnectedSocketRecvBufferSize(CI_SOCKET_RECV_BUFFER_SIZE)
, m_iListenQueueSize(NETWORK_THREAD_LISTEN_QUEUE_DEFAULTSIZE)
, m_iMaxFDSize(NETWORK_THREAD_FD_SETSIZE)
{
	m_iReadSocketCount = 0;
	m_iWriteSocketCount = 0;

#ifdef _WIN32
	FD_ZERO(&m_fdSetAllRead);
	FD_ZERO(&m_fdSetAllWrite);

	FD_ZERO(&m_fdSetRead);
	FD_ZERO(&m_fdSetWrite);

	m_iMaximumSocket = NU_INVALID_SOCKET;
#endif

	m_ReadSockets = new CCiSocket*[m_iMaxFDSize];
	m_WriteSockets = new CCiSocket*[m_iMaxFDSize];

	for ( int i = 0; i < m_iMaxFDSize; i++ )
	{
		m_ReadSockets[i] = NULL;
		m_WriteSockets[i] = NULL;
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

	// select time
	m_iTimeoutMillisec = 0;	// no wait
}

CNetworkThread::~CNetworkThread()
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

		delete m_WriteSockets[i];
		m_WriteSockets[i] = NULL;
	}
	delete[] m_ReadSockets;
	delete[] m_WriteSockets;

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

#ifdef _WIN32
/* for select */
/* fd set manipulation */
bool CNetworkThread::FDSetAdd(CCiSocket* pSocket, bool bReadCase)
{
	if ( pSocket == NULL ) {
		return false;
	}

	int iSocketFD = pSocket->m_iSocket;

	if ( iSocketFD == NU_INVALID_SOCKET ) {
		return false;
	}

	if ( bReadCase == true ) {
		FD_SET(iSocketFD, &m_fdSetAllRead);
	}
	else {
		FD_SET(iSocketFD, &m_fdSetAllWrite);
	}

	if ( m_iMaximumSocket < iSocketFD ) {
		m_iMaximumSocket = iSocketFD;
	}

	return true;
}

bool CNetworkThread::FDSetDelete(CCiSocket* pSocket, bool bReadCase)
{
	if ( pSocket == NULL ) {
		return false;
	}

	int iSocketFD = pSocket->m_iSocket;

	if ( iSocketFD == NU_INVALID_SOCKET ) {
		return false;
	}

	if ( bReadCase == true ) {
		FD_CLR((unsigned int)iSocketFD, &m_fdSetAllRead);
	}
	else {
		FD_CLR((unsigned int)iSocketFD, &m_fdSetAllWrite);
	}

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
bool CNetworkThread::PollFDAdd(CCiSocket *pSocket, bool bReadCase)
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

	if ( bReadCase == true )
	{
		m_PollFds[ m_iPollFdCount ].events = POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI;
	}
	else
	{
		m_PollFds[ m_iPollFdCount ].events = POLLOUT | POLLWRNORM | POLLWRBAND;
	}

	m_iPollFdCount++;

	return true;
}

bool CNetworkThread::PollFDDelete(CCiSocket *pSocket)
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

bool CNetworkThread::PollFDEnable(pollfd *pPollFD, bool bReadCase)
{
	if ( pPollFD == NULL ) {
		return false;
	}

	if ( pPollFD->fd == NU_INVALID_SOCKET ) {
		return false;
	}

	if ( bReadCase == true ) {
		pPollFD->events |= (POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI);
	}
	else {
		pPollFD->events |= (POLLOUT | POLLWRNORM | POLLWRBAND);
	}

	return true;
}

bool CNetworkThread::PollFDDisable(pollfd *pPollFD, bool bReadCase)
{
	if ( pPollFD == NULL ) {
		return false;
	}

	if ( pPollFD->fd == NU_INVALID_SOCKET ) {
		return false;
	}

	if ( bReadCase == true ) {
		pPollFD->events &= ~(POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI);
	}
	else {
		pPollFD->events &= ~(POLLOUT | POLLWRNORM | POLLWRBAND);
	}

	return true;
}

pollfd *CNetworkThread::FindPollFD(CCiSocket *pSocket)
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

/* select/poll timeout */
void CNetworkThread::SetTimeoutMillisec(int iTimeoutMillisec)
{
	m_iTimeoutMillisec = iTimeoutMillisec;
}

/* socket set management */
CCiSocket* CNetworkThread::FindSocket(int iSocketFD, bool bReadCase)
{
	int i = 0;
	int iMaxFDSize = GetMaxFDSize();

	if ( bReadCase == true )
	{
		for ( ; i < iMaxFDSize; i++ )
		{
			if ( m_ReadSockets[i] != NULL
				&& m_ReadSockets[i]->m_iSocket == iSocketFD )
			{
				return m_ReadSockets[i];
			}
		}
	}
	else
	{
		for ( ; i < iMaxFDSize; i++ )
		{
			if ( m_WriteSockets[i] != NULL
				&& m_WriteSockets[i]->m_iSocket == iSocketFD )
			{
				return m_WriteSockets[i];
			}
		}
	}

	return NULL;
}

bool CNetworkThread::AddReadSocket(CCiSocket* pReadSocket)
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
			m_iReadSocketCount++;
			return true;
		}
	}

	return false;
}

bool CNetworkThread::DeleteReadSocket(CCiSocket* pReadSocket, bool bPreserve)
{
	if ( pReadSocket == NULL ) {
		return false;
	}

	int iMaxFDSize = GetMaxFDSize();
	for ( int i = 0; i < iMaxFDSize; i++ )
	{
		if ( m_ReadSockets[i] == pReadSocket )
		{
			if ( bPreserve == false ) {
				delete pReadSocket;
			}
			m_ReadSockets[i] = NULL;
			m_iReadSocketCount--;
			return true;
		}
	}

	return false;
}

bool CNetworkThread::AddWriteSocket(CCiSocket* pWriteSocket)
{
	if ( pWriteSocket == NULL ) {
		return false;
	}

	int iMaxFDSize = GetMaxFDSize();
    for ( int i = 0; i < iMaxFDSize; i++ )
	{
		if ( m_WriteSockets[i] == NULL )
		{
			m_WriteSockets[i] = pWriteSocket;
			m_iWriteSocketCount++;
			return true;
		}
	}

	return false;
}

bool CNetworkThread::DeleteWriteSocket(CCiSocket* pWriteSocket, bool bPreserve)
{
	if ( pWriteSocket == NULL ) {
		return false;
	}

	int iMaxFDSize = GetMaxFDSize();
	for ( int i = 0; i < iMaxFDSize; i++ )
	{
		if ( m_WriteSockets[i] == pWriteSocket )
		{
			if ( bPreserve == false ) {
				delete pWriteSocket;
			}
			m_WriteSockets[i] = NULL;
			m_iWriteSocketCount--;
			return true;
		}
	}

	return false;
}

/* disable a socket in case of internal errors */
bool CNetworkThread::DisableSocket(CCiSocket* pSocket)
{
	/* Disabling a socket means that it will be delete from fd set */
	/* and disconnected buf is not be deleted from the socket set */

	if ( pSocket == NULL ) {
		return false;
	}

#ifdef _WIN32
	FDSetDelete(pSocket, true);	/* from read socket array */
	FDSetDelete(pSocket, false);	/* from write socket array */
#else
	PollFDDelete(pSocket);
#endif

	pSocket->Disconnect();

	return true;
}

bool CNetworkThread::OnReadSocketError(CCiSocket* pSocket)
{
	if ( pSocket == NULL ) {
		return false;
	}

	DisableSocket(pSocket);
	DeleteWriteSocket(pSocket, true);
	DeleteReadSocket(pSocket, true);
	delete pSocket;
	return true;
}

/* implementations */
bool CNetworkThread::InitInstance()
{
	return CCiThread2::InitInstance();
}

bool CNetworkThread::ExitInstance()
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

		m_WriteSockets[i] = NULL;
	}

	m_iReadSocketCount = 0;
	m_iWriteSocketCount = 0;

#ifdef _WIN32
	FD_ZERO(&m_fdSetAllRead);
	FD_ZERO(&m_fdSetAllWrite);

	FD_ZERO(&m_fdSetRead);
	FD_ZERO(&m_fdSetWrite);

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

bool CNetworkThread::Run()
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

		if ( !m_bExit && m_iEventsCount > 0 )
		{
			ProcessWriteEvent();
		}

		if ( !m_bExit )
		{
			ProcessTimeout();
		}
	}

	return true;
}

// overridables
bool CNetworkThread::WaitNetworkEvent(int *piNEvent)
{
#ifdef _WIN32
	struct timeval timeout = { m_iTimeoutMillisec/1000, (m_iTimeoutMillisec%1000)*1000 };

	// clear the fd_set
	FD_ZERO(&m_fdSetRead);
	FD_ZERO(&m_fdSetWrite);

	m_fdSetRead = m_fdSetAllRead;
	m_fdSetWrite = m_fdSetAllWrite;

	int iNEvent = select(m_iMaximumSocket+1, &m_fdSetRead, &m_fdSetWrite, NULL, &timeout);
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

bool CNetworkThread::ProcessReadEvent()
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
#ifdef _WIN32
				else if ( m_ReadSockets[i] != NULL && m_ReadSockets[i] == pSocket && pSocket->GetRemainSendDataSize() > 0 )	// by gun 2003.9.1
#else
				else if ( m_PollSockets[i] != NULL && m_PollSockets[i] == pSocket && pSocket->GetRemainSendDataSize() > 0 )	// by gun 2003.9.1
#endif
				{
					if ( FindSocket(pSocket->m_iSocket, false) == NULL )
					{
#ifdef _WIN32
						FDSetAdd(pSocket, false);
#else
						PollFDEnable(&(m_PollFds[i]), false);
#endif
						AddWriteSocket(pSocket);
					}
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

bool CNetworkThread::ProcessWriteEvent()
{
#ifdef _WIN32
	int iMaxFDSize = GetMaxFDSize();
	for ( int i = 0; i < iMaxFDSize; i++ )
	{
		if ( m_ReadSockets[i] != NULL
			&& FD_ISSET(m_ReadSockets[i]->m_iSocket, &m_fdSetWrite) )
		{
			CCiSocket* pSocket = m_ReadSockets[i];

			if ( DoWrite(pSocket) == false ) {
				if ( m_ReadSockets[i] != NULL && m_ReadSockets[i] == pSocket )
					OnReadSocketError(pSocket);
			} else if ( m_ReadSockets[i] != NULL && m_ReadSockets[i] == pSocket && pSocket->GetRemainSendDataSize() == 0 ) {
				/* if there is no pending data in the socket then clear read fd from the write set */
				FDSetDelete(pSocket, false);
				DeleteWriteSocket(pSocket, true);
			}

			if ( --m_iEventsCount <= 0 ) {
				break;
			}
		}
	}
#else
	for ( int i = 0; i < m_iPollFdCount; i++ )
	{
		if ( m_PollFds[i].fd != NU_INVALID_SOCKET
			&& ( m_PollFds[i].revents & ( POLLOUT | POLLWRNORM | POLLWRBAND | POLLPRI )) )
		{
			m_PollFds[i].revents = 0;	// CLEAR;

			CCiSocket* pSocket = m_PollSockets[i];

			if ( DoWrite(pSocket) == false )
			{
				if ( m_PollSockets[i] != NULL && m_PollSockets[i] == pSocket )
					OnReadSocketError(pSocket);
			}
			else if ( m_PollSockets[i] != NULL && m_PollSockets[i] == pSocket && pSocket->GetRemainSendDataSize() == 0 ) {
				/* if there is no pending data in the socket then clear write event */
				PollFDDisable(&m_PollFds[i], false);
				DeleteWriteSocket(pSocket, true);
			}

			if ( --m_iEventsCount <= 0 ) {
				break;
			}
		}
	}
#endif

	return true;
}

bool CNetworkThread::ProcessTimeout()
{
	return true;
}

ReceiveIntMessageResult_t CNetworkThread::ReceiveIntMessage(CCiSocket* pReadSocket, int *piReceivedMessage)
{
	if ( pReadSocket == NULL || piReceivedMessage == NULL )
	{
		return RECEIVE_INT_MESSAGE_FALSE;
	}

	///////////////////////////////////////////////////////////
	//
	// Message -> 4 Byte, BodySize-> 4 Byte, Body -> x Byte
	//
	// 이것을 만족하는 Protocol에 적용 가능. 이외에는 오바라이딩 필요..
	//

	int iRecvDataSize = pReadSocket->GetCurrentRecvDataSize();
	bool isReceiveHeader = false;

	// 고정된 크기의 Header Size ( 8 Byte ) 만큼을 다 읽지 못한 경우
	if ( iRecvDataSize < 8 )
	{
		if ( pReadSocket->ReadFromNetworkToBuffer( 8 - iRecvDataSize ) != true )
		{
			return RECEIVE_INT_MESSAGE_FALSE;
		}

		if ( pReadSocket->GetCurrentRecvDataSize() == iRecvDataSize )	// 이 경우는 FD_CLOSE 이벤트가 발생한 경우로 판단.
		{
			return RECEIVE_INT_MESSAGE_FALSE;
		}

		if ( pReadSocket->GetCurrentRecvDataSize() < 8 )
			return RECEIVE_INT_MESSAGE_INTERIM;

		iRecvDataSize = 8;
		isReceiveHeader = true;
	}

	int iNBodySize = 0;

	if ( pReadSocket->LookAheadInt32( &iNBodySize, 4 ) != true )
		return RECEIVE_INT_MESSAGE_INTERIM;

	if ( iNBodySize > 0 )
	{
		iRecvDataSize = pReadSocket->GetCurrentRecvDataSize();
		if ( pReadSocket->ReadFromNetworkToBuffer( iNBodySize - ( iRecvDataSize - 8 )) != true )
		{
			return RECEIVE_INT_MESSAGE_FALSE;
		}

		if ( !isReceiveHeader && iRecvDataSize == pReadSocket->GetCurrentRecvDataSize() )
		{
			return RECEIVE_INT_MESSAGE_FALSE;
		}
	}

	if ( pReadSocket->GetCurrentRecvDataSize() < ( 8+iNBodySize ) )
		return RECEIVE_INT_MESSAGE_INTERIM;

	if ( pReadSocket->ReadInt32( piReceivedMessage ) != true )
		return RECEIVE_INT_MESSAGE_INTERIM;

	return RECEIVE_INT_MESSAGE_TRUE;

}

bool CNetworkThread::OnMessage(CCiSocket* /*pReadSocket*/, int /*iMessage*/)
{
	return true;
}

bool CNetworkThread::DoWrite(CCiSocket *pWriteSocket)
{
	if ( pWriteSocket == NULL ) {
		return false;
	}

	/* Flush is needed */
	return pWriteSocket->WriteFromBufferToNetwork();
}
