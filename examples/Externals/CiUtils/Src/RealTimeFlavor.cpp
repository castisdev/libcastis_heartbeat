// RealTimeFlavor.cpp: implementation of the CRealTimeFlavor class.
//
//////////////////////////////////////////////////////////////////////

#include "internal_CiUtils.h"
#include "RealTimeFlavor.h"
#include "MTimeSpan2.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRealTimeFlavor::CRealTimeFlavor()
{
	m_iPeriodMillieSec = 0;		/* run as soon as possible */

	/* for safety, automatically setbase */
	SetBaseTime();
}

CRealTimeFlavor::~CRealTimeFlavor()
{

}

void CRealTimeFlavor::SetBaseTime()
{
	CMTime2 mdCurrentTime;
	m_mdBaseTime = mdCurrentTime;

	m_llExecutionCounter = 0;
}

void CRealTimeFlavor::SetPeriodMillieSec( int iPeriodMillieSec )
{
	m_iPeriodMillieSec = iPeriodMillieSec;
	SetBaseTime();
}

CMTime2& CRealTimeFlavor::GetBaseTime()
{
	return m_mdBaseTime;
}

int CRealTimeFlavor::GetPeriodMillieSec()
{
	return m_iPeriodMillieSec;
}

/* a plus value means that the pace is faster than expected */
/* a minus value means some delay */
long long CRealTimeFlavor::GetPaceInMillieSec()
{
	CMTime2 mdTimeToGo =
		m_mdBaseTime +
		(long long)(static_cast<long long>(m_iPeriodMillieSec)*m_llExecutionCounter);

	CMTime2 mdCurrentTime;
	CMTimeSpan2 mdsDifference = mdTimeToGo - mdCurrentTime;

	long long llMSeconds = 0;
	mdsDifference.GetTimeSpan(llMSeconds);

	return llMSeconds;
}

CMTime2 CRealTimeFlavor::GetPeriodStartTime()
{
	return m_mdBaseTime+(long long)(static_cast<long long>(m_iPeriodMillieSec)*m_llExecutionCounter);
}

CMTime2 CRealTimeFlavor::GetPeriodEndTime()
{
	return m_mdBaseTime+
			(long long)(static_cast<long long>(m_iPeriodMillieSec)*m_llExecutionCounter)+
			static_cast<long long>(m_iPeriodMillieSec);
}
