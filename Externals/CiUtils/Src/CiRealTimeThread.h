// CiRealTimeThread.h: interface for the CCiRealTimeThread class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CIREALTIMETHREAD_H__291DEB2E_B1AA_4C40_A2C9_D1FE48AC0BB6__INCLUDED_)
#define AFX_CIREALTIMETHREAD_H__291DEB2E_B1AA_4C40_A2C9_D1FE48AC0BB6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CiThread2.h"
#include "RealTimeFlavor.h"

class CCiRealTimeThread : public CCiThread2, public CRealTimeFlavor
{
public:
	CCiRealTimeThread();
	virtual ~CCiRealTimeThread();

	virtual void ThreadProc();
};

#endif // !defined(AFX_CIREALTIMETHREAD_H__291DEB2E_B1AA_4C40_A2C9_D1FE48AC0BB6__INCLUDED_)
