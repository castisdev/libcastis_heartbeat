// RealTimeFlavor.cpp: implementation of the CRealTimeFlavor class.
//
//////////////////////////////////////////////////////////////////////

#include "internal_CiUtils.h"
#include "RealTimeFlavor.h"
#include "UTimeSpan.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRealTimeFlavor::CRealTimeFlavor()
{
	m_iPeriodMicroSec = 0;		/* run as soon as possible */

	/* for safety, automatically setbase */
	SetBaseTime();
}

CRealTimeFlavor::~CRealTimeFlavor()
{

}

void CRealTimeFlavor::SetBaseTime()
{
	CUTime utCurrentTime;
	m_utBaseTime = utCurrentTime;

	m_llExecutionCounter = 0;
}

void CRealTimeFlavor::SetPeriodMicroSec( int iPeriodMicroSec )
{
	m_iPeriodMicroSec = iPeriodMicroSec;
	SetBaseTime();
}

CUTime& CRealTimeFlavor::GetBaseTime()
{
	return m_utBaseTime;
}

int CRealTimeFlavor::GetPeriodMicroSec()
{
	return m_iPeriodMicroSec;
}

/* a plus value means that the pace is faster than expected */
/* a minus value means some delay */
long long CRealTimeFlavor::GetPaceInMicroSec()
{
	CUTime utTimeToGo =	m_utBaseTime + (long long)(static_cast<long long>(m_iPeriodMicroSec)*m_llExecutionCounter);

	CUTime utCurrentTime;
	CUTimeSpan utsDifference = utTimeToGo - utCurrentTime;

	long long llUSeconds = 0;
	utsDifference.GetTimeSpan(llUSeconds);

	return llUSeconds;
}

CUTime CRealTimeFlavor::GetPeriodStartTime()
{
	return m_utBaseTime + (long long)(static_cast<long long>(m_iPeriodMicroSec)*m_llExecutionCounter);
}

CUTime CRealTimeFlavor::GetPeriodEndTime()
{
	return m_utBaseTime+
			(long long)(static_cast<long long>(m_iPeriodMicroSec)*m_llExecutionCounter)+
			static_cast<long long>(m_iPeriodMicroSec);
}
