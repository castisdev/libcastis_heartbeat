#pragma once

class CUTimeSpan
{
public:
	CUTimeSpan(void);
	virtual ~CUTimeSpan(void);

	// Attribute
public:
	long long m_usec;

	// Operations
public:
	CUTimeSpan(int iDays, int iHours, int iMinutes, int iSeconds, int iUSeconds);
	//static long long FrameToMSecond(long long llFrames, float fFrameRate);
	//static long long MSecondToFrame(long long llMSeconds, float fFrameRate);

	CUTimeSpan operator+(CUTimeSpan timeSpan);
	CUTimeSpan operator-(CUTimeSpan timeSpan);
	bool operator==(CUTimeSpan timeSpan);
	bool operator>(CUTimeSpan timeSpan);
	bool operator<(CUTimeSpan timeSpan);
	bool operator<=(CUTimeSpan timeSpan);
	bool operator>=(CUTimeSpan timeSpan);

	// microsecond operation
	bool SetTimeSpan(long long microseconds);
	bool GetTimeSpan(long long& microseconds);

	bool SetTimeSpan(int iDays, int iHours, int iMinutes, int iSeconds, int iUSeconds);
	bool GetTimeSpan(int& iDays, int& iHours, int& iMinutes, int& iSeconds, int& iUSeconds);

	bool SetTimeSpan(char *pszTimeSpan);	// +/-DD:HH:MM:SS.mmmmmm
	bool GetTimeSpan(char **ppszTimeSpan);

	// frame operation
	//bool SetFrameTimeSpan(long long llFrames, float fFrameRate);
	//bool GetFrameTimeSpan(long long& llFrames, float fFrameRate);

	//bool SetFrameTimeSpan(int iDays, int iHours, int iMinutes, int iSeconds, int iFrames, float fFrameRate);
	//bool GetFrameTimeSpan(int& iDays,int& iHours, int& iMinutes, int& iSeconds, int& iFrames, float fFrameRate);

	//bool SetFrameTimeSpan(char *pszFrameTimeSpan, float fFrameRate);	// +/-DD:HH:MM:SS.ff
	//bool GetFrameTimeSpan(char **ppszFrameTimeSpan, float fFrameRate);

	//char* GetDurationStringforDBAPI();		// by sasgas 2003.8.2, DDHHMMSSmmm
	//bool Set(const char* pszTimeSpan);		// by sasgas 2003.8.2, DDHHMMSSmmm
	//CUTimeSpan(const char* pszTimeSpan);			// by sasgas 2003.8.2, DDHHMMSSmmm
};
