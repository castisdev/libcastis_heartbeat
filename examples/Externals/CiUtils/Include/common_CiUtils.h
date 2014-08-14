#ifndef __COMMON_CIUTILS_H__
#define __COMMON_CIUTILS_H__

// Support for platform-independent integer types
#include "../Src/CiTypes.h"

#include "../Src/CiConfig.h"
#include "../Src/CiSafeString.h"
#include "../Src/CiMutex.h"
#include "../Src/CiSemaphore.h"
#include "../Src/CiUtils.h"
#include "../Src/crc32.h"
#include "../Src/MTime2.h"
#include "../Src/MTimeSpan2.h"
#include "../Src/UTime.h"
#include "../Src/UTimeSpan.h"
#include "../Src/NetUtil.h"
#include "../Src/NetworkThread.h"
#include "../Src/CiQueue.h"
#include "../Src/CiSocket.h"
#include "../Src/CiMulticastSocket.h"
#include "../Src/CiThread2.h"
#include "../Src/CiThreadComplex.h"
#include "../Src/CiRealTimeThread.h"
#include "../Src/CiThreadRealTimeComplex.h"
#include "../Src/LinkedListTemplate.h"
#include "../Src/RealTimeFlavor.h"
#include "../Src/RealTimeNetworkThread.h"
#include "../Src/StringLinkedList.h"
#include "../Src/AuthNetworkThread.h"

#include "../Src/MessageDigest.h"
#include "../Src/MessageDigestMD5.h"

#include "../Src/CiLogger.h"

// 2005-12-20
#include "../Src/NetIOStream.h"
#include "../Src/NetIOStreamInterface.h"

#ifdef _WIN32
#pragma comment ( lib, "pdh.lib" )
#pragma comment ( lib, "iphlpapi.lib" )
#endif


#endif
