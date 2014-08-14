// CiRealTimeThread.cpp: implementation of the CCiRealTimeThread class.
//
//////////////////////////////////////////////////////////////////////

#include "internal_CiUtils.h"
#include "CiRealTimeThread.h"
#include "MTimeSpan2.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCiRealTimeThread::CCiRealTimeThread()
{
}

CCiRealTimeThread::~CCiRealTimeThread()
{

}

void CCiRealTimeThread::ThreadProc()
{
	/*                  */
	/* real time flavor */
	/*                  */

	/* in case of the type limit */
	if ( m_llExecutionCounter == 0x7FFFFFFFFFFFFFFFLL ) {
		SetBaseTime();
	}

	m_llExecutionCounter++;

	long long llMSeconds = GetPaceInMillieSec();
	/* A plus value means that the pace is faster than expected */
	if ( llMSeconds > 0 )
		castis::msleep(static_cast<unsigned long>(llMSeconds));

	CCiThread2::ThreadProc();
}
