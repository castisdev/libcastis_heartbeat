// MTime2.h: interface for the CMTime2 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MTIME2_H__394EB11D_FC33_460A_B867_C745AEDA5D23__INCLUDED_)
#define AFX_MTIME2_H__394EB11D_FC33_460A_B867_C745AEDA5D23__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMTimeSpan2;

class CMTime2
{
public:
	CMTime2();
	virtual ~CMTime2();

// Attribute
public:
	long long m_llMSecond; // time_t + millisecond

	enum {
		MTIME_CASE_FRAMECOUNT,
		MTIME_CASE_MILLIESECOND
	};

// Operations
public:
	int GetMonth();
	int	GetYear();
	int MsecToFrameCnt(int iMillieSecond);
	CMTime2 operator+(long long llMSeconds);
	CMTime2 operator+(CMTimeSpan2 mtsTimeSpan);

	CMTime2 operator-(long long llMSeconds);
	CMTime2 operator-(CMTimeSpan2 mtsTimeSpan);
	CMTimeSpan2 operator-(CMTime2 mtTime);

	bool operator==(CMTime2 mtTime);
	bool operator>(CMTime2 mtTime);
	bool operator<(CMTime2 mtTime);
	bool operator<=(CMTime2 mtTime);
	bool operator>=(CMTime2 mtTime);

	// millisecond operation
	bool SetTime(long long llMSecond);
	bool GetTime(long long& llMSecond);

	bool SetTime(int iYear, int iMonth, int iDay, int iHour, int iMinute, int iSecond, int iMSecond);
	bool GetTime(int& iYear, int& iMonth, int& iDay, int& iHour, int& iMinute, int& iSecond, int& iMSecond);

	bool SetTime(char *pszTime);	// YYYY:MM:DD:HH:MM:SS.mmm
	bool GetTime(char **ppszTime);

	// frame operation
	bool SetFrameTime(long long llFrames, float fFrameRate);
	bool GetFrameTime(long long& llFrames, float fFrameRate);

	bool SetFrameTime(int iYear, int iMonth, int iDay, int iHour, int iMinute, int iSecond, int iFrame, float fFrameRate);
	bool GetFrameTime(int& iYear, int& iMonth, int& iDay, int& iHour, int& iMinute, int& iSecond, int& iFrame, float fFrameRate);

	bool SetFrameTime(char *pszFrameTime, float fFrameRate);	// YYYY:MM:DD:HH:MM:SS.ff
	bool GetFrameTime(char **ppszFrameTime, float fFrameRate);

	char * GetFullStringforDBAPI( int iCase = MTIME_CASE_FRAMECOUNT );		// by sasgas 2003.8.2, YYYYMMDDHHMMSSmmm
	bool Set(const char *pszTime);		// by sasgas 2003.8.2, YYYYMMDDHHMMSSmmm
	CMTime2(const char* pszTime);				// by sasgas 2003.8.2, YYYYMMDDHHMMSSmmm
};

#endif // !defined(AFX_MTIME2_H__394EB11D_FC33_460A_B867_C745AEDA5D23__INCLUDED_)

