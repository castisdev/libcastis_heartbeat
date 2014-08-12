// RealTimeFlavor.h: interface for the CRealTimeFlavor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REALTIMEFLAVOR_H__F3A91379_9095_4E17_AF97_3593183DDCD9__INCLUDED_)
#define AFX_REALTIMEFLAVOR_H__F3A91379_9095_4E17_AF97_3593183DDCD9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UTime.h"

class CRealTimeFlavor
{
public:
	CRealTimeFlavor();
	virtual ~CRealTimeFlavor();

public:
	void SetBaseTime();
	/* SetPeriodMicroSec calls SetBaseTime internally */
	void SetPeriodMicroSec( int iPeriodMicroSec );

	CUTime& GetBaseTime();
	int GetPeriodMicroSec();
	/* a plus value means that the pace is faster than expected */
	/* a minus value means some delay */
	long long GetPaceInMicroSec();

	/* return the start and end time of the period */
	CUTime GetPeriodStartTime();
	CUTime GetPeriodEndTime();

	CUTime	m_utBaseTime;
	int	m_iPeriodMicroSec;

	long long m_llExecutionCounter;
};

#endif // !defined(AFX_REALTIMEFLAVOR_H__F3A91379_9095_4E17_AF97_3593183DDCD9__INCLUDED_)
