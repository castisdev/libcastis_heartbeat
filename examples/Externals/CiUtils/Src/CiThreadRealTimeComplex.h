// CiThreadRealTimeComplex.h: interface for the CCiThreadRealTimeComplex class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CITHREADREALTIMECOMPLEX_H__9B0FBA18_6791_49F0_87BC_BE7F133C21E7__INCLUDED_)
#define AFX_CITHREADREALTIMECOMPLEX_H__9B0FBA18_6791_49F0_87BC_BE7F133C21E7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CiThreadComplex.h"
#include "RealTimeFlavor.h"

class CCiThreadRealTimeComplex : public CCiThreadComplex, public CRealTimeFlavor
{
public:
	CCiThreadRealTimeComplex();
	virtual ~CCiThreadRealTimeComplex();

	// overridables
	virtual bool Run();
};

#endif // !defined(AFX_CITHREADREALTIMECOMPLEX_H__9B0FBA18_6791_49F0_87BC_BE7F133C21E7__INCLUDED_)
