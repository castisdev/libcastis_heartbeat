
// CiTypes.h
//		- Frequently used integer types are defined here.
//			Because 'boost' library already provides the definitions on size-tagged integer types,
//			we just use them.

#ifndef _CI_TYPES_H_
#define _CI_TYPES_H_

#include <boost/version.hpp>
#include <boost/config.hpp>


#ifndef BOOST_HAS_STDINT_H
# include <boost/limits.hpp> // implementation artifact; not part of interface
#endif

#include <limits.h>         // needed for limits macros

// INT32_IS_NOT_INT
//		- In some platform, 'boost' library defines 'int32_t' as 'long'
//			and 'uint32_t' as 'unsigned long'.
//			In that case, C++ compiler fails to resolve 'int' type argument into 'int32_t'.
//			So, you should add another overloaded function which has 'int' type parameter.
//			Do that implementation only if 'INT32_IS_NOT_INT' is defined.

// INT64_IS_NOT_LONGLONG
//		- 'int64_t' is defined as 'long long' only in Linux 32-bit platform.
//			Otherwise, it is type-defined as 'long' in Linux 64-bit or as '__int64' in Windows.
//			So, you should add another overloaded function which has 'long long' type parameter.
//			Do that implementation only if 'INT64_IS_NOT_LONGLONG' is defined.

#ifdef _WIN32
# ifndef _WIN64

//---------------------------
// Windows 32-bit platform

#if BOOST_VERSION < 104500
#  define INT32_IS_NOT_INT
#endif

# else

//---------------------------
// Windows 64-bit platform
#  define INT32_IS_NOT_INT

# endif
#elif defined(linux) || defined(__linux) || defined(__linux__)
# if (ULONG_MAX == 0xFFFFFFFF)

//---------------------------
// Linux 32-bit platform
/*	- Nothing to be defined */

# else

//---------------------------
// Linux 64-bit platform
#  define INT64_IS_NOT_LONGLONG

# endif
#endif


#include <boost/cstdint.hpp>		// from boost library
using boost::int8_t;
using boost::int16_t;
using boost::int32_t;
using boost::int64_t;
using boost::uint8_t;
using boost::uint16_t;
using boost::uint32_t;
using boost::uint64_t;

#endif // _CI_TYPES_H_
