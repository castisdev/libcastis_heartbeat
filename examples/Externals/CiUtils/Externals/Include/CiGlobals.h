#ifndef __CIGLOBALS_H__
#define __CIGLOBALS_H__

/* CiGlobals.h's role */
/* 1. resolve type mismatching between OS. */
/* 2. constant definition */
/* 3. header file mismatching between OS. */

#ifdef _WIN32

#define FD_SETSIZE 1024

/* windows.h includes winsock2.h */
#ifndef _AFXDLL
#include <windows.h>
#else
#include <afxsock.h>
/* in AFXDLL application, windows.h should not be included by apps. */
/* it is already included by afxwin.h */
/* in this case, windows.h in afxwin.h doesn't include winsock(2).h */
/* so, we have to include afxsock.h to use socket functions */
#endif

#ifdef _WIN32
#include <io.h>
#endif

#else	/* These are for Linux code, actually gcc, g++ code */

#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/time.h>
#include <semaphore.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

/* the following files are needed to use socket in Linux */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* the following files are needed to use aync i/o */
#ifndef __CYGWIN__
#include <aio.h>
#endif

#endif


/////////////////////////////////////////////////////////////////////////////////////////


#include <fcntl.h>
#include <sys/stat.h>

// max. string length definitions
// be sure to declare char. arraies having one more byte for the safe null termination.
// Use castis' safe string copy and concatenation function.
#define CI_MAX_NAME_LENGTH			(64)
#define CI_MAX_DESCRIPTION_LENGTH	(256)
#define CI_MAX_FILE_PATH_LENGTH		(256)
#define CI_MAX_FILE_NAME_LENGTH		(CI_MAX_FILE_PATH_LENGTH)
#define CI_MAX_IP_ADDRESS_LENGTH	(32)

#define	CI_IP_ADDR_LENGTH			(16)
#define	CI_USER_ID_LENGTH			(24+1)  // +1 for null character
#define	CI_TIME_LENGTH				(14+1)  // strlen("YYYYMMDDHHmmSS")
#define CI_UUID_LENGTH				(32+4) // 32 hex digits, +4 dashes

#define KB							1024

#ifdef _WIN32
#define DIRECTORY_DELIMITER			'\\'
#else
#define DIRECTORY_DELIMITER			'/'
#endif

/* CastIs FileSystem Directory Delimiter */
#define CI_DIRECTORY_DELIMITER		"~="

// 8 byte integer definition
#if defined(_WIN32) && (_MSC_VER <= 1200)

typedef __int64 CiLongLong_t;
typedef unsigned __int64 CiULongLong_t;

#else

typedef long long CiLongLong_t;
typedef unsigned long long CiULongLong_t;

#endif

// NURI 20030616 file stat structure.
#ifdef _WIN32
typedef struct _stati64 CiStat;
#else
typedef struct stat CiStat;
#endif


#if !defined(ASSERT) || !defined(VERIFY)
#define ASSERT(f)          ((void)0)
#define VERIFY(f)          ((void)(f))
#endif


//#define ENABLE_DEBUGPRINTF

/* DebugPrintf */
#ifdef _WIN32

#include <stdio.h>

#ifndef _INLINE_DEBUGPRINTF
#define _INLINE_DEBUGPRINTF

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4100)
#endif

inline void DebugPrintf(const char *szFmt, ...)
{
#ifdef ENABLE_DEBUGPRINTF
	va_list argList;
	va_start(argList, szFmt);
	vfprintf(stderr, szFmt, argList);
	va_end(argList);
#else
	/* do nothing */
#endif
}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif	/* _CI_INLINE_ */

#else	/* _WIN32 */

#include <stdio.h>
#ifdef ENABLE_DEBUGPRINTF
#define DebugPrintf(fmt, args...) fprintf(stderr, fmt, ##args)
#else
#define DebugPrintf(fmt, args...)

#define TRACE(fmt, args...) DebugPrintf(fmt, ##args)
#endif

#endif	/* else */

// file io stuffs

// file open flag
#ifdef _WIN32

#define CI_O_APPEND	_O_APPEND
#define CI_O_BINARY	_O_BINARY
#define CI_O_TEXT   _O_TEXT
#define CI_O_CREAT	_O_CREAT
#define CI_O_EXCL	_O_EXCL
#define CI_O_RDONLY	_O_RDONLY
#define CI_O_RDWR	_O_RDWR
#define CI_O_WRONLY	_O_WRONLY
#define CI_O_TRUNC	_O_TRUNC
#define CI_O_LARGEFILE 0

#define CI_S_IREAD	_S_IREAD
#define CI_S_IWRITE	_S_IWRITE

#else

#define CI_O_APPEND	O_APPEND
#define CI_O_BINARY	0
#define CI_O_TEXT	0
#define CI_O_CREAT	O_CREAT
#define CI_O_EXCL	O_EXCL
#define CI_O_RDONLY	O_RDONLY
#define CI_O_RDWR	O_RDWR
#define CI_O_WRONLY	O_WRONLY
#define CI_O_TRUNC	O_TRUNC
#define CI_O_LARGEFILE O_LARGEFILE

#define CI_S_IREAD	S_IREAD
#define CI_S_IWRITE	S_IWRITE

#endif


// for Visual c++ 6.0
// iostream 에 __int64 에 대한 serialize operator 를 정의했다.
#if defined(_WIN32) && (_MSC_VER <= 1200)
#include <iostream>
inline std::ostream& operator<< (std::ostream& lhs, const CiLongLong_t& rhs)
{
	char buffer[32];

	_i64toa(rhs, buffer, 10);
	lhs << buffer;

	return lhs;
}

inline std::istream& operator>> (std::istream& lhs, CiLongLong_t& rhs)
{
	char buffer[32];

	lhs >> buffer;
	rhs = _atoi64(buffer);

	return lhs;
}
#endif    // #ifdef _WIN32


#ifdef _WIN32

#pragma warning (push)
#pragma warning (disable : 4996)

// same as stdio
inline int CiOpen( const char* szFileName , int oflag , int pmode = CI_S_IREAD | CI_S_IWRITE )
{ return _open( szFileName , oflag , pmode ); };

inline int CiClose(int fd)
{ return _close(fd); };

inline int CiRead(int fd, void* buffer , unsigned int count)
{ return _read( fd , buffer , count ); };

inline int CiWrite(int fd , const void* buffer , unsigned int count)
{ return _write( fd , buffer , count ); };

inline CiLongLong_t CiLLSeek(int fd, CiLongLong_t llOffset, int whence)
{ return _lseeki64(fd, llOffset, whence); };

// NURI 20030616 file stat functions

inline CiLongLong_t CiFStat64(int fd, CiStat &st)
{ return _fstati64(fd, &st); };

inline CiLongLong_t CiStat64(const char *szFileName, CiStat &st)
{ return _stati64(szFileName, &st); };

inline int CiFlush(int fd)
{ return _commit(fd); };

// 0(ok) -1(fail)
inline int CiTruncate(int fd, CiLongLong_t llSize)
{
	CiLongLong_t pos = CiLLSeek(fd , llSize, SEEK_SET);
	if ( pos < 0 ) {
		return -1;
	}
	if ( SetEndOfFile((HANDLE)_get_osfhandle(fd)) ) {
		return 0;
	}

	return -1;
};

#pragma warning (pop)

#else

// same as stdio
inline int CiOpen( const char* szFileName , int oflag , int pmode = CI_S_IREAD | CI_S_IWRITE )
{ return open( szFileName , oflag , pmode ); };

inline int CiClose(int fd)
{ return close(fd); };

inline int CiRead(int fd, void* buffer , unsigned int count)
{ return read( fd , buffer , count ); };

inline int CiWrite(int fd , const void* buffer , unsigned int count)
{ return write( fd , buffer , count ); };

inline CiLongLong_t CiLLSeek(int fd, CiLongLong_t llOffset, int whence)
{ return lseek(fd, llOffset, whence); };

// NURI 20030616 file stat functions.
// need to compile flag : -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE

inline CiLongLong_t CiFStat64(int fd, CiStat &st)
{ return fstat(fd, &st); };

inline CiLongLong_t CiStat64(const char *szFileName, CiStat &st)
{ return stat(szFileName, &st); };

inline int CiFlush(int fd)
{ return fsync(fd); };

// trunc and file pos is at trunc point
inline int CiTruncate(int fd, CiLongLong_t llSize)
{
	int ir = ftruncate(fd, llSize);
	if ( ir != 0 )
		return -1;
	if ( (llSize=lseek( fd , llSize , SEEK_SET )) < 0 )
		return -1;
	return ir;
};


#if defined _POSIX_ASYNCHRONOUS_IO
// everain 20051013 async I/O
// Link 'librt' unless supported at kernel level.

//0(ok), -1(fail)
inline int CiAioRead(aiocb* aiocbp)
{
	return aio_read( aiocbp );
}
//0(ok), -1(fail)
inline int CiAioWrite(aiocb* aiocbp)
{
	return aio_write( aiocbp );
}

//0(ok), EINPROGRESS(not finished), ECANCELED(canceled), else(fail)
inline int CiAioError( aiocb *aiocbp )
{
	return aio_error( aiocbp );
}

//returns last return value of aiocb
inline int CiAioReturn( aiocb *aiocbp )
{
	return aio_return( aiocbp );
}

//0(ok), -1(fail)
inline int CiAioFlush(int op, aiocb* aiocbp)
{
	return aio_fsync( op, aiocbp );
}

#endif //_POSIX_ASYNCHRONOUS_IO


#endif


namespace castis {
	inline void msleep(unsigned long msec)
	{
#ifdef _WIN32
		Sleep(msec);
#else
		usleep(msec * 1000);
#endif
	}

	inline void microsleep(unsigned long usec)
	{
#ifdef _WIN32
		Sleep(usec/1000);
#else
		usleep(usec);
#endif
	}
}

#ifndef _WIN32
#define strtok(x,y) disablestrtok(x,y)
#define CiStrtok(x,y,z) strtok_r(x,y,z)
#else

#if (_MSC_VER >= 1400)
#define strtok(x,y) disablestrtok(x,y)
#define CiStrtok(x,y,z) strtok_s(x,y,z)
#else

// Microsoft Visual C++ .NET 2003 이하
#define CiStrtok(x,y,z)  strtok(x,y)
#endif
#endif

#endif	// __CIGLOBALS_H__
