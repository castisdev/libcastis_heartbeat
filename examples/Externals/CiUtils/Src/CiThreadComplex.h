// CiThreadComplex.h: interface for the CCiThreadComplex class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CITHREADCOMPLEX2_H__B81BC011_942F_4913_99CC_4680220360EC__INCLUDED_)
#define AFX_CITHREADCOMPLEX2_H__B81BC011_942F_4913_99CC_4680220360EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CiThread2.h"

class CCiThreadComplex : public CCiThread2
{
public:
	CCiThreadComplex();
	virtual ~CCiThreadComplex();

protected:
	CCiThreadList m_ciThreads;

public:
	/* ci thread list stuffs */
	bool AddCiThread(CCiThread2 *pCiThread);
	bool DeleteCiThread(CCiThread2 *pCiThread);
	int GetNCiThreads();
	int GetRawNCiThreads();		/* 2004.05.23 NURI */
	int GetRawNRunningThreads();	/* 2004.06.08 NURI */

	// overridables
	virtual bool Run();
	/* for safe termination of all the threads in this complex */
	virtual bool ExitInstance();
};

#endif // !defined(AFX_CITHREADCOMPLEX_H__B81BC011_942F_4913_99CC_4680220360EC__INCLUDED_)
