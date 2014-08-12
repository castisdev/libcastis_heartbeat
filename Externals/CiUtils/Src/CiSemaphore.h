// CiSemaphore.h: interface for the CCiSemaphore class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CISEMAPHORE_H__ABC2DD00_3F83_4033_9324_32151C30D16F__INCLUDED_)
#define AFX_CISEMAPHORE_H__ABC2DD00_3F83_4033_9324_32151C30D16F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef _WIN32
#include <wtypes.h>
#else
#include <semaphore.h>
#endif

class CCiSemaphore
{
public:
	CCiSemaphore();
	virtual ~CCiSemaphore();

private:
#ifdef _WIN32
	HANDLE	m_hSemaphore;
#else
	sem_t m_sem;
#endif

public:
	bool Initialize(long lInitialValue, long lMaximumValue=1);
	bool Wait();
	bool Post();
	bool Finalize();
};

#endif // !defined(AFX_CISEMAPHORE_H__ABC2DD00_3F83_4033_9324_32151C30D16F__INCLUDED_)
