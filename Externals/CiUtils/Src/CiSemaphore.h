#pragma once

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
