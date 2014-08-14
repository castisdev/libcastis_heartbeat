#ifndef __INTERNAL_CIUTILS_H__
#define __INTERNAL_CIUTILS_H__

#ifdef _WIN32
#pragma warning (disable : 4786)
#endif

// Support for platform-independent integer types
#include "CiTypes.h"

#include "CiGlobals.h"
#include "CiSockError.h"

#include <stdlib.h>
#include <ctype.h>
#include <sys/timeb.h>

#include <boost/version.hpp>

#ifdef _WIN32
#pragma message("BOOST Version is " BOOST_LIB_VERSION)
#endif

#ifdef _WIN32
#define snprintf _snprintf
#endif

extern void CiUtils_PrintWithTime(FILE *stream, char *pszString);

#endif
