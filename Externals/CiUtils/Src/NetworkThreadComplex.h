// NetworkThreadComplex.h: interface for the CNetworkThreadComplex class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NETWORKTHREADCOMPLEX_H__1DF4DCFC_02E7_4813_8B50_F4C180860C66__INCLUDED_)
#define AFX_NETWORKTHREADCOMPLEX_H__1DF4DCFC_02E7_4813_8B50_F4C180860C66__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CiThread2.h"
#include "NetworkThread.h"

class CNetworkThreadComplex : public CCiThread2  
{
public:
	CNetworkThreadComplex();
	virtual ~CNetworkThreadComplex();

	CNetworkThreadList m_networkThreads;

public:
	/* network thread list stuffs */
	CiBool_t AddNetworkThread(CNetworkThread *pNetworkThread);
	CiBool_t DeleteNetworkThread(CNetworkThread *pNetworkThread);
	int GetNNetworkThreads();

	// overridables
	virtual CiBool_t InitInstance();
	virtual CiBool_t Run();
	/* for safe termination of all the threads in this complex */
	virtual CiBool_t ExitInstance();

#ifdef _WIN32
	virtual int ComposeSocketSet(fd_set *pRSet, fd_set *pWSet, CiBool_t bAutoReset=CI_TRUE);
#else
	virtual int ComposeSocketSet();
#endif
	virtual CiBool_t WaitNetworkEvent(fd_set *pRSet, fd_set *pWSet, int *piNEvent, int iTimeoutUSec=1);
	virtual CiBool_t ProcessTimeout();
    virtual CiBool_t ProcessConnection(fd_set *pRSet);
	virtual CiBool_t ProcessEvents(fd_set *pRSet, fd_set *pWSet);

};

#endif // !defined(AFX_NETWORKTHREADCOMPLEX_H__1DF4DCFC_02E7_4813_8B50_F4C180860C66__INCLUDED_)
