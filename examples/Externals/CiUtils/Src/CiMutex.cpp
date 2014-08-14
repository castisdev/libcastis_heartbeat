#include "internal_CiUtils.h"
#include "CiMutex.h"

CCiMutex::CCiMutex(void)
{
#ifdef _WIN32
	InitializeCriticalSection(&_mutex);
#else
	pthread_mutex_init(&_mutex, NULL);
#endif
}

CCiMutex::~CCiMutex(void)
{
#ifdef _WIN32
	DeleteCriticalSection(&_mutex);
#else
	pthread_mutex_destroy(&_mutex);
#endif
}

bool CCiMutex::Lock()
{
#ifdef _WIN32
	EnterCriticalSection(&_mutex);
	return true;
#else
	return ( pthread_mutex_lock(&_mutex) == 0 );
#endif
}

bool CCiMutex::UnLock()
{
#ifdef _WIN32
	LeaveCriticalSection(&_mutex);
	return true;
#else
	return ( pthread_mutex_unlock(&_mutex) == 0 );
#endif
}
