#include "legacy_support.h"

#ifdef _WIN32
#include <process.h>
#else
#include <errno.h>
#include <pthread.h>
#endif

#include <algorithm>

#ifdef _WIN32

#define CI_EINTR                   WSAEINTR
#define CI_EBADF                   WSAEBADF
#define CI_EACCES                  WSAEACCES
#define CI_EFAULT                  WSAEFAULT
#define CI_EINVAL                  WSAEINVAL
#define CI_EMFILE                  WSAEMFILE

#define CI_EWOULDBLOCK             WSAEWOULDBLOCK
#define CI_EINPROGRESS             WSAEINPROGRESS
#define CI_EALREADY                WSAEALREADY
#define CI_ENOTSOCK                WSAENOTSOCK
#define CI_EDESTADDRREQ            WSAEDESTADDRREQ
#define CI_EMSGSIZE                WSAEMSGSIZE
#define CI_EPROTOTYPE              WSAEPROTOTYPE
#define CI_ENOPROTOOPT             WSAENOPROTOOPT
#define CI_EPROTONOSUPPORT         WSAEPROTONOSUPPORT
#define CI_ESOCKTNOSUPPORT         WSAESOCKTNOSUPPORT
#define CI_EOPNOTSUPP              WSAEOPNOTSUPP
#define CI_EPFNOSUPPORT            WSAEPFNOSUPPORT
#define CI_EAFNOSUPPORT            WSAEAFNOSUPPORT
#define CI_EADDRINUSE              WSAEADDRINUSE
#define CI_EADDRNOTAVAIL           WSAEADDRNOTAVAIL
#define CI_ENETDOWN                WSAENETDOWN
#define CI_ENETUNREACH             WSAENETUNREACH
#define CI_ENETRESET               WSAENETRESET
#define CI_ECONNABORTED            WSAECONNABORTED
#define CI_ECONNRESET              WSAECONNRESET
#define CI_ENOBUFS                 WSAENOBUFS
#define CI_EISCONN                 WSAEISCONN
#define CI_ENOTCONN                WSAENOTCONN
#define CI_ESHUTDOWN               WSAESHUTDOWN
#define CI_ETOOMANYREFS            WSAETOOMANYREFS
#define CI_ETIMEDOUT               WSAETIMEDOUT
#define CI_ECONNREFUSED            WSAECONNREFUSED
#define CI_ELOOP                   WSAELOOP
#define CI_ENAMETOOLONG            WSAENAMETOOLONG
#define CI_EHOSTDOWN               WSAEHOSTDOWN
#define CI_EHOSTUNREACH            WSAEHOSTUNREACH
#define CI_ENOTEMPTY               WSAENOTEMPTY
#define CI_EPROCLIM                WSAEPROCLIM
#define CI_EUSERS                  WSAEUSERS
#define CI_EDQUOT                  WSAEDQUOT
#define CI_ESTALE                  WSAESTALE
#define CI_EREMOTE                 WSAEREMOTE

#else

#define CI_EINTR                   EINTR
#define CI_EBADF                   EBADF
#define CI_EACCES                  EACCES
#define CI_EFAULT                  EFAULT
#define CI_EINVAL                  EINVAL
#define CI_EMFILE                  EMFILE

#define CI_EWOULDBLOCK             EWOULDBLOCK
#define CI_EINPROGRESS             EINPROGRESS
#define CI_EALREADY                EALREADY
#define CI_ENOTSOCK                ENOTSOCK
#define CI_EDESTADDRREQ            EDESTADDRREQ
#define CI_EMSGSIZE                EMSGSIZE
#define CI_EPROTOTYPE              EPROTOTYPE
#define CI_ENOPROTOOPT             ENOPROTOOPT
#define CI_EPROTONOSUPPORT         EPROTONOSUPPORT
#define CI_ESOCKTNOSUPPORT         ESOCKTNOSUPPORT
#define CI_EOPNOTSUPP              EOPNOTSUPP
#define CI_EPFNOSUPPORT            EPFNOSUPPORT
#define CI_EAFNOSUPPORT            EAFNOSUPPORT
#define CI_EADDRINUSE              EADDRINUSE
#define CI_EADDRNOTAVAIL           EADDRNOTAVAIL
#define CI_ENETDOWN                ENETDOWN
#define CI_ENETUNREACH             ENETUNREACH
#define CI_ENETRESET               ENETRESET
#define CI_ECONNABORTED            ECONNABORTED
#define CI_ECONNRESET              ECONNRESET
#define CI_ENOBUFS                 ENOBUFS
#define CI_EISCONN                 EISCONN
#define CI_ENOTCONN                ENOTCONN
#define CI_ESHUTDOWN               ESHUTDOWN
#define CI_ETOOMANYREFS            ETOOMANYREFS
#define CI_ETIMEDOUT               ETIMEDOUT
#define CI_ECONNREFUSED            ECONNREFUSED
#define CI_ELOOP                   ELOOP
#define CI_ENAMETOOLONG            ENAMETOOLONG
#define CI_EHOSTDOWN               EHOSTDOWN
#define CI_EHOSTUNREACH            EHOSTUNREACH
#define CI_ENOTEMPTY               ENOTEMPTY
#define CI_EPROCLIM                EPROCLIM
#define CI_EUSERS                  EUSERS
#define CI_EDQUOT                  EDQUOT
#define CI_ESTALE                  ESTALE
#define CI_EREMOTE                 EREMOTE

#endif

#ifndef NU_INVALID_SOCKET
#define NU_INVALID_SOCKET		(-1)
#endif

namespace
{
	bool nu_disconnect(int sock)
	{
#ifdef _WIN32
		return closesocket(sock) == 0;
#else
		return close(sock) == 0;
#endif
	}

	namespace castis {
		inline void msleep(unsigned long msec)
		{
#ifdef _WIN32
			Sleep(msec);
#else
			usleep(msec * 1000);
#endif
		}
	}
}

//////////////////////////////////////////////////////////////////////////

namespace cihb
{
	bool CCiSemaphore::Initialize(long lInitialValue, long lMaximumValue)
	{
		bool bResult = true;
#ifdef _WIN32
		m_hSemaphore = CreateSemaphore(NULL, lInitialValue, lMaximumValue, NULL);
		if ( m_hSemaphore == NULL ) {
			bResult = false;
		}
#else
		int iReturn = sem_init(&m_sem, 0, lInitialValue);
		if ( iReturn != 0 ) {
			bResult = false;
		}
#endif
		return bResult;
	}

	bool CCiSemaphore::Wait()
	{
		bool bResult = true;

#ifdef _WIN32
		DWORD dwResult = WaitForSingleObject(m_hSemaphore, INFINITE);
		if ( dwResult == WAIT_FAILED || dwResult == WAIT_ABANDONED ) {
			bResult = false;
		}
#else
		int iReturn = 0;
		while ( (iReturn = sem_wait(&m_sem)) != 0 && errno == EINTR );
		if ( iReturn != 0 ) {
			bResult = false;
		}
#endif

		return bResult;
	}

	bool CCiSemaphore::Post()
	{
		bool bResult = true;

#ifdef _WIN32
		BOOL bReturn = ReleaseSemaphore(m_hSemaphore, 1, NULL);
		if ( bReturn == FALSE ) {
			bResult = false;
		}
#else
		int iReturn = sem_post(&m_sem);
		if ( iReturn != 0 ) {
			bResult = false;
		}
#endif
		return bResult;
	}

	bool CCiSemaphore::Finalize()
	{
		bool bResult = true;
#ifdef _WIN32
		BOOL bReturn = CloseHandle(m_hSemaphore);
		if ( bReturn == FALSE ) {
			bResult = false;
		}
#else
		int iReturn = sem_destroy(&m_sem);
		if ( iReturn != 0 ) {
			bResult = false;
		}
#endif
		return bResult;
	}

	//////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#define CI_THREAD2_INVALID_THREAD_HANDLE (CiThread2Handle_t)(NULL)
#else
#define CI_THREAD2_INVALID_THREAD_HANDLE (CiThread2Handle_t)(NULL)
#endif

#ifdef _WIN32
	void (_cdecl _CiThread2Entry)(void* pParam)
#else
	void *(_CiThread2Entry)(void *pParam)
#endif
	{
		CCiThread2* pCiThread = (CCiThread2 *)pParam;

		/* by nuri 2003.09.15 */
#ifndef _WIN32
		pthread_detach(pthread_self());
#endif

		while ( !pCiThread->IsThreadDone() ) {
			pCiThread->ThreadProc();
		}

		pCiThread->NotifyExit();

#ifdef _WIN32
		return;
#else
		return NULL;
#endif
	}

	CCiThread2::CCiThread2()
		: m_ciThreadHandle(CI_THREAD2_INVALID_THREAD_HANDLE)
		, m_bExit(true)
		, m_state(CI_THREAD2_READY)
	{
		m_semExit.Initialize(1);
	}

	CCiThread2::~CCiThread2()
	{
		m_semExit.Finalize();
	}

	bool CCiThread2::CreateThread()
	{
		if ( m_ciThreadHandle != CI_THREAD2_INVALID_THREAD_HANDLE ) {
			return false;
		}

		if ( !PrepareRunning() ) {
			m_bExit = true;
			return false;
		}

		// create the thread
#ifdef _WIN32
		m_ciThreadHandle = (CiThread2Handle_t)_beginthread(_CiThread2Entry, 0, this);
		if (m_ciThreadHandle == (CiThread2Handle_t)NULL) {
			m_bExit = true;
			m_ciThreadHandle = CI_THREAD2_INVALID_THREAD_HANDLE;
			m_semExit.Post();
			return false;
		}
#else

		int iReturn = pthread_create(&m_ciThreadHandle, NULL, _CiThread2Entry, this);
		if ( iReturn != 0 ) {
			m_bExit = true;
			m_ciThreadHandle = CI_THREAD2_INVALID_THREAD_HANDLE;
			m_semExit.Post();
			return false;
		}
#endif

		return true;
	}

	bool CCiThread2::RunThreadHere()
	{
		if ( m_ciThreadHandle != CI_THREAD2_INVALID_THREAD_HANDLE ) {
			return false;
		}

		if ( !PrepareRunning() ) {
			m_bExit = true;
			return false;
		}

		// run the thread directly
		while ( !IsThreadDone() ) {					/* bu nuri 2003.09.08 */
			ThreadProc();
		}

		NotifyExit();

		return true;
	}

	void CCiThread2::NotifyExit()
	{
		m_semExit.Post();
	}

	void CCiThread2::WaitUntilExit()
	{
		if (m_ciThreadHandle != CI_THREAD2_INVALID_THREAD_HANDLE)
		{
			m_semExit.Wait();

			m_ciThreadHandle = CI_THREAD2_INVALID_THREAD_HANDLE;
			m_semExit.Post();
		}
	}


	/* overridables */

	bool CCiThread2::PrepareRunning()
	{
		m_bExit = false;
		m_state = CI_THREAD2_READY;
		m_semExit.Wait();

		return true;
	}

	bool CCiThread2::InitInstance()
	{
		return true;
	}

	bool CCiThread2::ExitInstance()
	{
		return true;
	}

	bool CCiThread2::Run()
	{
		return true;
	}

	void CCiThread2::ThreadProc()
	{
		switch (m_state) {
		case CI_THREAD2_READY:
			if ( InitInstance() != true ) {
				m_state = CI_THREAD2_ERROR;
				return;
			}

			m_state = CI_THREAD2_DONE_INITINSTANCE;
			/* run through to CI_THREAD2_DONE_INITINSTANCE */

		case CI_THREAD2_DONE_INITINSTANCE:
			if ( !m_bExit ) {
				if ( Run() != true ) {
					m_state = CI_THREAD2_ERROR;
				}
				return;
			}

			m_state = CI_THREAD2_DONE_RUN;
			/* run through to CI_THREAD2_DONE_RUN */

		case CI_THREAD2_DONE_RUN:
		case CI_THREAD2_ERROR:
		default:
			ExitInstance();
			if (m_state == CI_THREAD2_DONE_RUN)
				m_state = CI_THREAD2_DONE_EXITINSTANCE;
			else
				m_state = CI_THREAD2_DONE_EXITINSTANCE_ERROR;
			/* run through to CI_THREAD2_DONE_EXITINSTANCE */

		case CI_THREAD2_DONE_EXITINSTANCE:
		case CI_THREAD2_DONE_EXITINSTANCE_ERROR:
			if (m_state == CI_THREAD2_DONE_EXITINSTANCE)
				m_state = CI_THREAD2_DONE;
			else
				m_state = CI_THREAD2_DONE_ERROR;
			/* notify exit in the virtual function ThreadProc */
			/* can cause a premature deletion of the thread object */
			/* You should notify exit after the return of ThreadProc */
			/* m_semExit.Post(); */
			return;

			/* never enter here */
		case CI_THREAD2_DONE:
		case CI_THREAD2_DONE_ERROR:
			return;
		}
	}

	//////////////////////////////////////////////////////////////////////////

#define NETWORK_THREAD_FD_SETSIZE					(1024)

	CNetworkThread::CNetworkThread(int iTimeoutMillisec/*=0*/)
	: m_iTimeoutMillisec(iTimeoutMillisec)
	, m_iMaxFDSize(NETWORK_THREAD_FD_SETSIZE)
	{
#ifdef _WIN32
		FD_ZERO(&m_fdSetAllRead);
		FD_ZERO(&m_fdSetRead);
		m_iMaximumSocket = NU_INVALID_SOCKET;
#endif

#ifndef _WIN32
		m_PollFds.reset(new pollfd[m_iMaxFDSize]);
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
	}

	CNetworkThread::~CNetworkThread()
	{
		std::for_each(m_ReadSockets.begin(), m_ReadSockets.end(), &nu_disconnect);
	}

	bool CNetworkThread::ReceiveIntMessage(int /*sockfd*/, int* /*piReceivedMessage*/)
	{
		return false;
	}

	bool CNetworkThread::OnMessage(int /*sockfd*/, int /*iMessage*/)
	{
		return false;
	}

#ifdef _WIN32
	/* for select */
	/* fd set manipulation */
	bool CNetworkThread::FDSetAdd(int sockfd)
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

	bool CNetworkThread::FDSetDelete(int sockfd)
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
	bool CNetworkThread::PollFDAdd(int sockfd)
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

	bool CNetworkThread::PollFDDelete(int sockfd)
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

	bool CNetworkThread::PollFDEnable(pollfd *pPollFD)
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

	bool CNetworkThread::PollFDDisable(pollfd *pPollFD)
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

	bool CNetworkThread::AddReadSocket(int sockfd)
	{
		if ( sockfd == NU_INVALID_SOCKET ) {
			return false;
		}

		m_ReadSockets.insert(sockfd);
		return true;
	}

	bool CNetworkThread::DeleteReadSocket(int sockfd)
	{
		if ( sockfd == NU_INVALID_SOCKET ) {
			return false;
		}

		m_ReadSockets.erase(sockfd);
		return true;
	}

	bool CNetworkThread::ExitInstance()
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

		m_fdSetRead = m_fdSetAllRead;

		int iNEvent = select(m_iMaximumSocket+1, &m_fdSetRead, NULL, NULL, &timeout);
#else
		int iNEvent = poll(m_PollFds.get(), m_iPollFdCount, m_iTimeoutMillisec);
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
		for (std::set<int>::const_iterator i = m_ReadSockets.begin(); i != m_ReadSockets.end(); ++i)
		{
			if ( *i != NU_INVALID_SOCKET && FD_ISSET(*i, &m_fdSetRead) )
			{
				int iMessage = 0;
				bool rimResult = ReceiveIntMessage(*i, &iMessage);
				if ( rimResult == true )
				{
					if ( OnMessage(*i, iMessage) == false )
					{
						OnReadSocketError(*i);
					}
				}
				else
				{
					OnReadSocketError(*i);
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
				bool rimResult = ReceiveIntMessage(m_PollFds[i].fd, &iMessage);
				if ( rimResult == true )
				{
					if ( OnMessage(m_PollFds[i].fd, iMessage) == false )
					{
						OnReadSocketError(m_PollFds[i].fd);
					}
				}
				else
				{
					OnReadSocketError(m_PollFds[i].fd);
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

	/* disable a socket in case of internal errors */
	bool CNetworkThread::DisableSocket(int sockfd)
	{
		/* Disabling a socket means that it will be delete from fd set */
		/* and disconnected buf is not be deleted from the socket set */

		if ( sockfd == NU_INVALID_SOCKET ) {
			return false;
		}

#ifdef _WIN32
		FDSetDelete(sockfd);	/* from read socket array */
#else
		PollFDDelete(sockfd);
#endif
		nu_disconnect(sockfd);

		return true;
	}

	bool CNetworkThread::OnReadSocketError(int sockfd)
	{
		if ( sockfd == NU_INVALID_SOCKET ) {
			return false;
		}

		DisableSocket(sockfd);
		DeleteReadSocket(sockfd);
		return true;
	}

} // namespace cihb
