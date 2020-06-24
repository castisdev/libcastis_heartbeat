// CiSemaphore.cpp: implementation of the CCiSemaphore class.
//
//////////////////////////////////////////////////////////////////////

#include "internal_CiUtils.h"
#include "CiSemaphore.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCiSemaphore::CCiSemaphore()
{
}

CCiSemaphore::~CCiSemaphore()
{

}

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
