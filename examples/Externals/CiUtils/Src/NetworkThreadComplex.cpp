// NetworkThreadComplex.cpp: implementation of the CNetworkThreadComplex class.
//
//////////////////////////////////////////////////////////////////////

#include "internal_CiUtils.h"
#include "NetUtil.h"
#include "NetworkThreadComplex.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNetworkThreadComplex::CNetworkThreadComplex()
{

}

CNetworkThreadComplex::~CNetworkThreadComplex()
{

}

/* network thread list management */
CiBool_t CNetworkThreadComplex::AddNetworkThread(CNetworkThread *pNetworkThread)
{
	return m_networkThreads.AddItem(pNetworkThread);
}

CiBool_t CNetworkThreadComplex::DeleteNetworkThread(CNetworkThread *pNetworkThread)
{
	return m_networkThreads.DeleteItem(pNetworkThread);
}

int CNetworkThreadComplex::GetNNetworkThreads()
{
	return m_networkThreads.GetNItems();
}

CiBool_t CNetworkThreadComplex::InitInstance()
{
    // initialize thread
	if ( CCiThread2::InitInstance() != CI_TRUE ) {
		return CI_FALSE;
	}

	// initialize all network thread's listen socket 
	CiBool_t bResult = CI_TRUE;
	unsigned long ulPosition, ulNextPosition;
	TRAVERSELIST(m_networkThreads, ulPosition, ulNextPosition) {
		CNetworkThread *pNetworkThread = m_networkThreads.GetCurrentItem(ulPosition);
		if ( pNetworkThread == NULL ) {
			continue;
		}
		if ( pNetworkThread->InitInstance() != CI_TRUE ) {
			bResult = CI_FALSE;
			break;
		}
	}

	return bResult;
}

CiBool_t CNetworkThreadComplex::ExitInstance()
{
    // finalize thread
	if ( CCiThread2::ExitInstance() != CI_TRUE ) {
		return CI_FALSE;
	}

	// initialize all network thread's listen socket 
	CiBool_t bResult = CI_TRUE;
	unsigned long ulPosition, ulNextPosition;
	TRAVERSELIST(m_networkThreads, ulPosition, ulNextPosition) {
		CNetworkThread *pNetworkThread = m_networkThreads.GetCurrentItem(ulPosition);
		if ( pNetworkThread == NULL ) {
			continue;
		}
		if ( pNetworkThread->ExitInstance() != CI_TRUE ) {
			bResult = CI_FALSE;
			break;
		}
	}

	return CI_TRUE;
}

int CNetworkThreadComplex::ComposeSocketSet(fd_set *pRSet, fd_set *pWSet, CiBool_t bAutoReset)
{
	if ( bAutoReset )
	{
		FD_ZERO(pRSet);
		FD_ZERO(pWSet);
	}

	int iMaximumSocket = -1;

	// all network thread's listen socket and the client sockets 
	unsigned long ulPosition, ulNextPosition;
	TRAVERSELIST(m_networkThreads, ulPosition, ulNextPosition) {
		CNetworkThread *pNetworkThread = m_networkThreads.GetCurrentItem(ulPosition);
		if ( pNetworkThread == NULL ) 
		{
			continue;
		}
		/* add this thread's sockets */
////		int iMaximumSocketTemp1 = pNetworkThread->ComposeReadSocketSet(pRSet, CI_FALSE);
////		int iMaximumSocketTemp2 = pNetworkThread->ComposeWriteSocketSet(pWSet, CI_FALSE);

////		int iMaximumSocketTemp = iMaximumSocketTemp1;

////		if ( iMaximumSocketTemp < iMaximumSocketTemp2 )
////			iMaximumSocketTemp = iMaximumSocketTemp2;

////		if ( iMaximumSocket < iMaximumSocketTemp )
////		{
////			iMaximumSocket = iMaximumSocketTemp;
////		}
	}

	return iMaximumSocket;
}

CiBool_t CNetworkThreadComplex::WaitNetworkEvent(fd_set *pRSet, fd_set *pWSet, int *piNEvent, int iTimeoutSec)
{
	struct timeval timeout = { iTimeoutSec, 0 };

	FD_ZERO(pRSet);
	int iMaximumSocket = ComposeSocketSet(pRSet, pWSet);

	int iNEvent = select(iMaximumSocket+1, pRSet, pWSet, NULL, &timeout);
	if ( iNEvent < 0 ) {
#ifdef _WIN32
		int Errno;
		if ( ( Errno = WSAGetLastError() ) == WSAEINTR ) {
#else
        if ( errno == EINTR ) {
#endif
            return CI_FALSE;
        }
    }

	if ( piNEvent != NULL ) {
		*piNEvent = iNEvent;	
	}
		
	return CI_TRUE;
}

CiBool_t CNetworkThreadComplex::ProcessTimeout()
{
	// check all network thread
	unsigned long ulPosition, ulNextPosition;
	TRAVERSELIST(m_networkThreads, ulPosition, ulNextPosition) {
		CNetworkThread *pNetworkThread = m_networkThreads.GetCurrentItem(ulPosition);
		if ( pNetworkThread == NULL ) {
			continue;
		}
		pNetworkThread->ProcessTimeout();
	}

	return CI_TRUE;
}

CiBool_t CNetworkThreadComplex::ProcessConnection(fd_set *pRSet)
{
	// check all network thread's listen sockets 
	unsigned long ulPosition, ulNextPosition;
	TRAVERSELIST(m_networkThreads, ulPosition, ulNextPosition) {
		CNetworkThread *pNetworkThread = m_networkThreads.GetCurrentItem(ulPosition);
		if ( pNetworkThread == NULL ) {
			continue;
		}
////		pNetworkThread->ProcessConnection(pRSet, NULL);
	}

	return CI_TRUE;
}

CiBool_t CNetworkThreadComplex::ProcessEvents(fd_set *pRSet, fd_set *pWSet)
{
	// check all network thread's sockets 
	unsigned long ulPosition, ulNextPosition;
	TRAVERSELIST(m_networkThreads, ulPosition, ulNextPosition) {
		CNetworkThread *pNetworkThread = m_networkThreads.GetCurrentItem(ulPosition);
		if ( pNetworkThread == NULL ) 
		{
			continue;
		}
////		pNetworkThread->ProcessReadEvent(pRSet);
////		pNetworkThread->ProcessWriteEvent(pWSet);
	}

	return CI_TRUE;	
}

CiBool_t CNetworkThreadComplex::Run()
{
	while ( !m_bExit ) 
	{
		fd_set rset;
		fd_set wset;
		int iNEvent = 0;
		if ( WaitNetworkEvent(&rset, &wset, &iNEvent) == CI_TRUE ) 
		{
			if ( iNEvent == 0 ) 
			{
				ProcessTimeout();
			}
			else if ( iNEvent > 0 ) 
			{
				/* look in the listen socket first */
				ProcessConnection(&rset);
				ProcessEvents(&rset, &wset);
			}
		}
	}

	return CI_TRUE;
}
