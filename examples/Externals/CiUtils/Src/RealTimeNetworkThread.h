// RealTimeNetworkThread.h: interface for the CRealTimeNetworkThread class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REALTIMENETWORKTHREAD_H__14DA15D5_4687_4070_B0BF_BD7D0EE32365__INCLUDED_)
#define AFX_REALTIMENETWORKTHREAD_H__14DA15D5_4687_4070_B0BF_BD7D0EE32365__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NetworkThread.h"
#include "RealTimeFlavor.h"

class CRealTimeNetworkThread : public CNetworkThread, public CRealTimeFlavor
{
public:
	CRealTimeNetworkThread(int iListenPortNumber);
	virtual ~CRealTimeNetworkThread();

	virtual bool Run();
};

#endif // !defined(AFX_REALTIMENETWORKTHREAD_H__14DA15D5_4687_4070_B0BF_BD7D0EE32365__INCLUDED_)
