#ifndef __COMMON_CIUTILS_H__
#define __COMMON_CIUTILS_H__

#include "../Src/CiSemaphore.h"
#include "../Src/NetUtil.h"
#include "../Src/CiSocket.h"
#include "../Src/CiThread2.h"

#ifdef _WIN32
#ifdef _DEBUG
#if _MSC_VER >= 1500
#ifdef _UNICODE
#pragma comment ( lib, "CiUtilsD_Unicode.lib" )
#else
#pragma comment ( lib, "CiUtilsD.lib" )
#endif
#elif _MSC_VER >= 1310
#pragma comment ( lib, "CiUtilsD.vc71.lib" )
#endif
#else
#if _MSC_VER >= 1500
#ifdef _UNICODE
#pragma comment ( lib, "CiUtils_Unicode.lib" )
#else
#pragma comment ( lib, "CiUtils.lib" )
#endif
#elif _MSC_VER >= 1310
#pragma comment ( lib, "CiUtils.vc71.lib" )
#endif
#endif
#pragma comment ( lib, "pdh.lib" )
#pragma comment ( lib, "iphlpapi.lib" )
#endif


#endif
