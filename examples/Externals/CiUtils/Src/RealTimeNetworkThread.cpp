// RealTimeNetworkThread.cpp: implementation of the CRealTimeNetworkThread class.
//
//////////////////////////////////////////////////////////////////////

#include "internal_CiUtils.h"
#include "RealTimeNetworkThread.h"
#include "CiSocket.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRealTimeNetworkThread::CRealTimeNetworkThread(int iListenPortNumber)
: CNetworkThread(iListenPortNumber)
{
	m_iPeriodMillieSec = 0;		/* run as fast as possible */
	m_llExecutionCounter = 0;
}

CRealTimeNetworkThread::~CRealTimeNetworkThread()
{

}

bool CRealTimeNetworkThread::Run()
{
	/*                  */
	/* real time flavor */
	/*                  */

	/* in case of the type limit */
	if ( m_llExecutionCounter == 0x7FFFFFFFFFFFFFFFLL ) {
		SetBaseTime();
	}

	/* assume ProcessTimeout() will be called */
	m_llExecutionCounter++;

	/* calculate the pace */
	long long llMSeconds = GetPaceInMillieSec();
	/* A plus value means that the pace is faster than expected */
	if ( llMSeconds > 0 ) {
		/* set select/poll timeout */
		SetTimeoutMillisec((int)llMSeconds);
	}
	else {
		/* ProcessTimeout should be called as soon as possible */
		SetTimeoutMillisec(0);
	}

	/* In case of select version */
	/* fd_set's are internally managed with member variables, m_fdSetAllRead, m_fdSetAllWrite, */
	/* m_fdSetRead, m_fdSetWrite */
	if ( WaitNetworkEvent(&m_iEventsCount) == true ) {
		if ( m_iEventsCount == 0 ) {	/* timeout case */
			ProcessTimeout();
		}
		else if ( m_iEventsCount > 0 ) {
			/* look in the listen socket first */
#ifdef _WIN32
			if ( m_pListenSocket != NULL && FD_ISSET(m_pListenSocket->m_iSocket, &m_fdSetRead) ) {
#else
			if ( m_pListenSocket != NULL && ( m_PollFds[0].revents & POLLRDNORM )) {
#endif
				ProcessConnection(NULL);
			}

			if ( m_iEventsCount > 0 ) {
				ProcessReadEvent();
			}

			if ( m_iEventsCount > 0 ) {
				ProcessWriteEvent();
			}

			long long llMSeconds = GetPaceInMillieSec();
			if ( llMSeconds <= 0 ) {	/* This is also another timeout case */
				ProcessTimeout();
			}
			else {
				/* timeout didn't occur in this case. */
				/* ProcessTimeout() was not be called */
				m_llExecutionCounter--;
			}
		}
	}

	return true;
}
