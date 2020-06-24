#ifndef __COMMON_CIUTILS_H__
#define __COMMON_CIUTILS_H__

// Support for platform-independent integer types
#include "../Src/CiTypes.h"

#include "../Src/CiSafeString.h"
#include "../Src/CiMutex.h"
#include "../Src/CiSemaphore.h"
#include "../Src/CiUtils.h"
#include "../Src/MTime2.h"
#include "../Src/MTimeSpan2.h"
#include "../Src/NetUtil.h"
#include "../Src/NetworkThread.h"
#include "../Src/CiSocket.h"
#include "../Src/CiThread2.h"
#include "../Src/CiThreadComplex.h"
#include "../Src/CiThreadRealTimeComplex.h"
#include "../Src/LinkedListTemplate.h"
#include "../Src/RealTimeFlavor.h"

#ifdef _WIN32
#pragma comment ( lib, "pdh.lib" )
#pragma comment ( lib, "iphlpapi.lib" )
#endif


#endif
