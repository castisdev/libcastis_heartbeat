// RealTimeFlavor.h: interface for the CRealTimeFlavor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REALTIMEFLAVOR_H__F3A91379_9095_4E17_AF97_3593183DDCD9__INCLUDED_)
#define AFX_REALTIMEFLAVOR_H__F3A91379_9095_4E17_AF97_3593183DDCD9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MTime2.h"

class CRealTimeFlavor
{
public:
	CRealTimeFlavor();
	virtual ~CRealTimeFlavor();

public:
	void SetBaseTime();
	/* SetPeriodMillieSec calls SetBaseTime internally */
	void SetPeriodMillieSec( int iPeriodMillieSec );

	CMTime2& GetBaseTime();
	int GetPeriodMillieSec();
	/* a plus value means that the pace is faster than expected */
	/* a minus value means some delay */
	long long GetPaceInMillieSec();

	/* return the start and end time of the period */
	CMTime2 GetPeriodStartTime();
	CMTime2 GetPeriodEndTime();

	CMTime2	m_mdBaseTime;
	int	m_iPeriodMillieSec;

	long long m_llExecutionCounter;
};

#endif // !defined(AFX_REALTIMEFLAVOR_H__F3A91379_9095_4E17_AF97_3593183DDCD9__INCLUDED_)
