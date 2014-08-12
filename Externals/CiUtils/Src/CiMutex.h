#pragma once

class CCiMutex
{
public:
	CCiMutex(void);
	~CCiMutex(void);

public:
	bool Lock();
	bool UnLock();

private:
#ifdef _WIN32
	CRITICAL_SECTION _mutex;
#else
	pthread_mutex_t _mutex;
#endif
};
