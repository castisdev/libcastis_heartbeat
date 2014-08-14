// MTimeSpan2.h: interface for the CMTimeSpan2 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MTIMESPAN2_H__991B0FD9_4B13_4299_96A3_932C9320FCAE__INCLUDED_)
#define AFX_MTIMESPAN2_H__991B0FD9_4B13_4299_96A3_932C9320FCAE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CMTimeSpan2
{
public:
	CMTimeSpan2();					// Initialized to 0
	virtual ~CMTimeSpan2();

// Attribute
public:
	long long m_llMSeconds;		// in mSec unit

// Operations
public:
	CMTimeSpan2(int iDays, int iHours, int iMinutes, int iSeconds, int iMSeconds );
	static long long FrameToMSecond(long long llFrames, float fFrameRate);
	static long long MSecondToFrame(long long llMSeconds, float fFrameRate);

	CMTimeSpan2 operator+(CMTimeSpan2 mtsTimeSpan);
	CMTimeSpan2 operator-(CMTimeSpan2 mtsTimeSpan);
	bool operator==(CMTimeSpan2 mtsTimeSpan);
	bool operator>(CMTimeSpan2 mtsTimeSpan);
	bool operator<(CMTimeSpan2 mtsTimeSpan);
	bool operator<=(CMTimeSpan2 mtsTimeSpan);
	bool operator>=(CMTimeSpan2 mtsTimeSpan);

	// millisecond operation
	bool SetTimeSpan(long long llMSeconds);
	bool GetTimeSpan(long long& llMSeconds);

	bool SetTimeSpan(int iDays, int iHours, int iMinutes, int iSeconds, int iMSeconds);
	bool GetTimeSpan(int& iDays, int& iHours, int& iMinutes, int& iSeconds, int& iMSeconds);

	bool SetTimeSpan(char *pszTimeSpan);	// +/-DD:HH:MM:SS.mmm
	bool GetTimeSpan(char **ppszTimeSpan);

	// frame operation
	bool SetFrameTimeSpan(long long llFrames, float fFrameRate);
	bool GetFrameTimeSpan(long long& llFrames, float fFrameRate);

	bool SetFrameTimeSpan(int iDays, int iHours, int iMinutes, int iSeconds, int iFrames, float fFrameRate);
	bool GetFrameTimeSpan(int& iDays,int& iHours, int& iMinutes, int& iSeconds, int& iFrames, float fFrameRate);

	bool SetFrameTimeSpan(char *pszFrameTimeSpan, float fFrameRate);	// +/-DD:HH:MM:SS.ff
	bool GetFrameTimeSpan(char **ppszFrameTimeSpan, float fFrameRate);

	char* GetDurationStringforDBAPI();		// by sasgas 2003.8.2, DDHHMMSSmmm
	bool Set(const char* pszTimeSpan);		// by sasgas 2003.8.2, DDHHMMSSmmm
	CMTimeSpan2(const char* pszTimeSpan);			// by sasgas 2003.8.2, DDHHMMSSmmm
};

#endif // !defined(AFX_MTIMESPAN2_H__991B0FD9_4B13_4299_96A3_932C9320FCAE__INCLUDED_)
