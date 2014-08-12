#include "internal_CiUtils.h"

#include "CiThread2.h"

#ifdef _WIN32
#include <process.h>
#endif

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

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

