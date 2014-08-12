// CiThreadComplex.cpp: implementation of the CCiThreadComplex class.
//
//////////////////////////////////////////////////////////////////////

#include "internal_CiUtils.h"
#include "NetUtil.h"
#include "CiThreadComplex.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCiThreadComplex::CCiThreadComplex()
{

}

CCiThreadComplex::~CCiThreadComplex()
{

}

/* ci thread list management */
bool CCiThreadComplex::AddCiThread(CCiThread2 *pCiThread)
{
	return m_ciThreads.SafeAddItem(pCiThread);
}

bool CCiThreadComplex::DeleteCiThread(CCiThread2 *pCiThread)
{
	return m_ciThreads.SafeDeleteItem(pCiThread, true);
}

int CCiThreadComplex::GetNCiThreads()
{
	return m_ciThreads.SafeGetNItems();
}

/* 2004.05.23 NURI */
int CCiThreadComplex::GetRawNCiThreads()
{
	return m_ciThreads.GetNItems();
}

/* 2004.06.08 NURI */
int CCiThreadComplex::GetRawNRunningThreads()
{
	int iNRunningThreads = 0;

	CCiThread2 *pCiThread;
	unsigned long ulPosition, ulPositionNext;
	TRAVERSELIST(m_ciThreads, ulPosition, ulPositionNext) {
		pCiThread = m_ciThreads.GetCurrentItem(ulPosition);

		if ( pCiThread->GetState() != CI_THREAD2_INACTIVE )
			iNRunningThreads++;
	}

	return iNRunningThreads;
}

bool CCiThreadComplex::Run()
{
	m_ciThreads.Lock();

	CCiThread2 *pCiThread;
	unsigned long ulPosition, ulPositionNext;
	TRAVERSELIST(m_ciThreads, ulPosition, ulPositionNext) {
		pCiThread = m_ciThreads.GetCurrentItem(ulPosition);

		/* 2004.06.08 NURI
		 * INACTIVE 상태를 추가하고 그에 따라 수정 */
		if ( pCiThread->GetState() != CI_THREAD2_INACTIVE )
			pCiThread->ThreadProc();
	}

	m_ciThreads.Unlock();

	return true;
}

/* for safe termination of all the threads in this complex */
bool CCiThreadComplex::ExitInstance()
{
	m_ciThreads.Lock();

	CCiThread2 *pCiThread;
	unsigned long ulPosition, ulPositionNext;
	TRAVERSELIST(m_ciThreads, ulPosition, ulPositionNext) {
		pCiThread = m_ciThreads.GetCurrentItem(ulPosition);

		/* run individual thread until it ends */
		pCiThread->EndThread();
		while ( !pCiThread->IsThreadDone() ) {				/* by nuri 2003.09.08 */
			pCiThread->ThreadProc();
		}
	}

	m_ciThreads.Unlock();

	return true;
}

