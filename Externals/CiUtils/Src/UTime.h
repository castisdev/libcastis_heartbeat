#pragma once

#ifdef _WIN32
#include < time.h >
#else
#include <sys/time.h>
#include <unistd.h>
#endif

class CUTimeSpan;

class CUTime
{
public:
	CUTime(void);
	virtual ~CUTime(void);

public:
	long long m_usec;
	enum {
		MTIME_CASE_FRAMECOUNT,
		MTIME_CASE_MILLIESECOND
	};

	// Operations
public:
	CUTime operator+(long long microseconds);
	CUTime operator+(CUTimeSpan timeSpan);

	CUTime operator-(long long microseconds);
	CUTime operator-(CUTimeSpan timeSpan);
	CUTimeSpan operator-(CUTime mtTime);

	bool operator==(CUTime mtTime);
	bool operator>(CUTime mtTime);
	bool operator<(CUTime mtTime);
	bool operator<=(CUTime mtTime);
	bool operator>=(CUTime mtTime);

	// microsecond operation
	bool SetTime(long long microseconds);
	bool GetTime(long long& microseconds);

	bool SetTime(int iYear, int iMonth, int iDay, int iHour, int iMinute, int iSecond, int iUSecond);
	bool GetTime(int& iYear, int& iMonth, int& iDay, int& iHour, int& iMinute, int& iSecond, int& iUSecond);

	bool SetTime(char *pszTime);	// YYYY:MM:DD:HH:MM:SS.mmmmmm
	bool GetTime(char **ppszTime);

	//int GetMonth();
	//int	GetYear();

	// frame operation
	//int MsecToFrameCnt(int microseconds);

	//bool SetFrameTime(long long llFrames, float fFrameRate);
	//bool GetFrameTime(long long& llFrames, float fFrameRate);

	//bool SetFrameTime(int iYear, int iMonth, int iDay, int iHour, int iMinute, int iSecond, int iFrame, float fFrameRate);
	//bool GetFrameTime(int& iYear, int& iMonth, int& iDay, int& iHour, int& iMinute, int& iSecond, int& iFrame, float fFrameRate);

	//bool SetFrameTime(char *pszFrameTime, float fFrameRate);	// YYYY:MM:DD:HH:MM:SS.ff
	//bool GetFrameTime(char **ppszFrameTime, float fFrameRate);

	//char * GetFullStringforDBAPI( int iCase = MTIME_CASE_FRAMECOUNT );		// by sasgas 2003.8.2, YYYYMMDDHHMMSSmmmmmm
	//bool Set(const char *pszTime);		// by sasgas 2003.8.2, YYYYMMDDHHMMSSmmmmmm
	//CUTime(const char* pszTime);				// by sasgas 2003.8.2, YYYYMMDDHHMMSSmmmmmm
};
