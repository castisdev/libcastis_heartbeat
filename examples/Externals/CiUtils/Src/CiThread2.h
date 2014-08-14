// CiThread2.h: interface for the CCiThread2 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CITHREAD2_H__806FEFC4_87A0_4252_9149_BC8AD8A451F5__INCLUDED_)
#define AFX_CITHREAD2_H__806FEFC4_87A0_4252_9149_BC8AD8A451F5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CiSemaphore.h"

#ifdef _WIN32

#define CI_THREAD2_INVALID_THREAD_HANDLE (CiThread2Handle_t)(NULL)
typedef unsigned long CiThread2Handle_t;

#else

#define CI_THREAD2_INVALID_THREAD_HANDLE (CiThread2Handle_t)(NULL)
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
	CCiSemaphore m_semLock;

	CiThread2State_t m_state;

public:
	/* locking facilities */
	bool Lock();
	bool Unlock();

	/* create & end thread */
	bool CreateThread();
//	void EndThread();		/* this is a gracefull end by setting the exit flag */

	/* attach this to another thread's context */
	bool RunThreadHere();

	/* prepare complexing. you should call this before add it to a complex */
	bool PrepareComplexing();

	/* end complexing. you should call this right after delete it from a complex */
	bool EndComplexing();

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

	CiThread2Handle_t GetHandle() const
	{
		return m_ciThreadHandle;
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

#include "LinkedListTemplate.h"
typedef CLinkedListTemplate<CCiThread2> CCiThreadList;

#endif // !defined(AFX_CITHREAD2_H__806FEFC4_87A0_4252_9149_BC8AD8A451F5__INCLUDED_)
