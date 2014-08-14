#pragma once

#ifdef _WIN32
#define FD_SETSIZE 1024

/* windows.h includes winsock2.h */
#ifndef _AFXDLL
#include <windows.h>
#else
#include <afxsock.h>
/* in AFXDLL application, windows.h should not be included by apps. */
/* it is already included by afxwin.h */
/* in this case, windows.h in afxwin.h doesn't include winsock(2).h */
/* so, we have to include afxsock.h to use socket functions */
#endif

#else	/* These are for Linux code, actually gcc, g++ code */

#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/poll.h>

#endif


#ifdef _WIN32
#include <wtypes.h>
#else
#include <semaphore.h>
#endif

#include <set>
#include <boost/scoped_array.hpp>

namespace cihb
{
	class CCiSemaphore
	{
	private:
#ifdef _WIN32
		HANDLE	m_hSemaphore;
#else
		sem_t m_sem;
#endif

	public:
		bool Initialize(long lInitialValue, long lMaximumValue=1);
		bool Wait();
		bool Post();
		bool Finalize();
	};

	//////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
	typedef unsigned long CiThread2Handle_t;
#else
	typedef pthread_t CiThread2Handle_t;
#endif

	typedef enum {
		CI_THREAD2_READY,
		CI_THREAD2_DONE_INITINSTANCE,
		CI_THREAD2_DONE_RUN,
		CI_THREAD2_DONE_EXITINSTANCE,
		CI_THREAD2_DONE_EXITINSTANCE_ERROR,
		CI_THREAD2_DONE,
		CI_THREAD2_DONE_ERROR,
		CI_THREAD2_ERROR,
		CI_THREAD2_INACTIVE			/* 2004.06.08 NURI */
	} CiThread2State_t;

	/* CCiThread2 */
	/* There are three possible ways to make CCiThread2 work */
	/* 1. new CCiThread2 -> CreateThread() */
	/* 2. new CCiThread2 -> RunThreadHere() */
	/* 3. new CCiThread2 -> PrepareComplexing() -> AddCiThread() to a Complex */

	class CCiThread2
	{
	public:
		CCiThread2();
		virtual ~CCiThread2();

	protected:
		CiThread2Handle_t m_ciThreadHandle;
		bool m_bExit;
		CCiSemaphore m_semExit;

		CiThread2State_t m_state;

	public:
		/* create & end thread */
		bool CreateThread();
		//	void EndThread();		/* this is a gracefull end by setting the exit flag */

		/* attach this to another thread's context */
		bool RunThreadHere();

		/* set/get functions */
		CiThread2State_t GetState() const
		{
			return m_state;
		}

		void SetState(CiThread2State_t state)
		{
			m_state = state;
		}

		bool IsState(CiThread2State_t state) const
		{
			return m_state == state;
		}

		/* exit synchronization */
		void NotifyExit();
		void WaitUntilExit();

		// by sasgas
		// thread_done or thread_done_error
		bool IsThreadDone() const
		{
			return (GetState() == CI_THREAD2_DONE || GetState() == CI_THREAD2_DONE_ERROR);
		}

		// Overidables
		virtual void EndThread()		/* this is a gracefull end by setting the exit flag */
		{
			m_bExit = true;
		}

		virtual bool PrepareRunning();

		virtual bool InitInstance();
		virtual bool ExitInstance();
		virtual bool Run();
		virtual void ThreadProc();
	};

	//////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4512)
#endif
	class CNetworkThread : public CCiThread2
	{
	public:
		CNetworkThread(int iTimeoutMillisec = 0);
		virtual ~CNetworkThread();

	protected:
		// The read sockets of this network thread
		std::set<int> m_ReadSockets;

#ifdef _WIN32
		fd_set m_fdSetAllRead;
		fd_set m_fdSetRead;
		int m_iMaximumSocket;
#else
		/* poll version */
		boost::scoped_array<pollfd> m_PollFds;
		int	m_iPollFdCount;
#endif
		// Events Count for immediate break to save the time
		int m_iEventsCount;

		// select time
		int	m_iTimeoutMillisec;

	private:
		const int m_iMaxFDSize;

	public:
		int GetMaxFDSize() const { return m_iMaxFDSize; }

#ifdef _WIN32
		/* for select */
		/* fd set manipulation */
		bool FDSetAdd(int sockfd);
		bool FDSetDelete(int sockfd);
#else
		/* for poll */
		/* poll fd manipulation */
		bool PollFDAdd(int sockfd);
		bool PollFDDelete(int sockfd);
		bool PollFDEnable(pollfd *pPollFD);
		bool PollFDDisable(pollfd *pPollFD);
#endif

		/* socket set management */
		bool AddReadSocket(int sockfd);
		bool DeleteReadSocket(int sockfd);

		/* disable a socket in case of internal errors */
		bool DisableSocket(int sockfd);
		virtual bool OnReadSocketError(int sockfd);

		// implementations
		virtual bool ExitInstance();
		virtual bool Run();

		// overridables
		virtual bool WaitNetworkEvent(int *piNEvent);
		virtual bool ProcessReadEvent();

		virtual bool ReceiveIntMessage(int sockfd, int *piReceivedMessage);
		virtual bool OnMessage(int sockfd, int iMessage);
	};
#ifdef _WIN32
#pragma warning(pop)
#endif
}
