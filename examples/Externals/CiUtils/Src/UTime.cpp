#include "internal_CiUtils.h"
#include "CiSafeString.h"
#include "UTimeSpan.h"
#include "UTime.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

#define MEGA (1000000)

#ifdef _WIN32
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif
struct timezone
{
	int  tz_minuteswest; /* minutes W of Greenwich */
	int  tz_dsttime;     /* type of dst correction */
};

// gettimeofday in windows
int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	FILETIME ft;
	unsigned __int64 tmpres = 0;
	static int tzflag;

	if (NULL != tv)
	{
		GetSystemTimeAsFileTime(&ft);

		tmpres |= ft.dwHighDateTime;
		tmpres <<= 32;
		tmpres |= ft.dwLowDateTime;

		// 100nano to 1micro
		tmpres /= 10;

		// to epoch time
		tmpres -= DELTA_EPOCH_IN_MICROSECS;    

		tv->tv_sec = (long)(tmpres / MEGA);
		tv->tv_usec = (long)(tmpres % MEGA);
	}

	if (NULL != tz)
	{
		if (!tzflag)
		{
			_tzset();
			tzflag++;
		}
		tz->tz_minuteswest = _timezone / 60;
		tz->tz_dsttime = _daylight;
	}

	return 0;
}
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CUTime::CUTime(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	m_usec = ((long long)tv.tv_sec*MEGA) + (long long)tv.tv_usec;
}

CUTime::~CUTime(void)
{
}

CUTime CUTime::operator+(long long microseconds)
{
	CUTime result;
	result.m_usec = m_usec + microseconds;
	return result;
}

CUTime CUTime::operator+(CUTimeSpan timeSpan)
{
	long long microseconds;
	timeSpan.GetTimeSpan(microseconds);

	CUTime result;
	result.m_usec = m_usec + microseconds;
	return result;
}

CUTime CUTime::operator-(long long microseconds)
{
	CUTime result;
	result.m_usec = m_usec - microseconds;
	return result;
}

CUTime CUTime::operator-(CUTimeSpan timeSpan)
{
	long long microseconds;
	timeSpan.GetTimeSpan(microseconds);

	CUTime result;
	result.m_usec = m_usec - microseconds;
	return result;
}

CUTimeSpan CUTime::operator-(CUTime time)
{
	long long diff = m_usec - time.m_usec;

	CUTimeSpan result;
	result.SetTimeSpan(diff);

	return result;
}

bool CUTime::operator==(CUTime time)
{
	return (m_usec == time.m_usec);
}

bool CUTime::operator>(CUTime time)
{
	return (m_usec > time.m_usec);
}

bool CUTime::operator<(CUTime time)
{
	return (m_usec < time.m_usec);
}

bool CUTime::operator<=(CUTime time)
{
	return (m_usec <= time.m_usec);

}

bool CUTime::operator>=(CUTime time)
{
	return (m_usec >= time.m_usec);
}

// microsecond operation
bool CUTime::SetTime(long long microseconds)
{
	m_usec = microseconds;
	return true;
}

bool CUTime::GetTime(long long& microseconds)
{
	microseconds = m_usec;
	return true;
}

bool CUTime::SetTime(int iYear, int iMonth, int iDay, int iHour, int iMinute, int iSecond, int iUSecond)
{

	struct tm stTime;
	stTime.tm_year	= iYear	 - 1900;
	stTime.tm_mon	= iMonth - 1;
	stTime.tm_mday	= iDay;
	stTime.tm_hour	= iHour;
	stTime.tm_min	= iMinute;
	stTime.tm_sec	= iSecond;

	time_t tTime = mktime(&stTime);
	if ( tTime == (time_t)-1 ) {
		return false;
	}

	m_usec = ((long long)tTime*MEGA) + iUSecond;
	return true;
}

bool CUTime::GetTime(int& iYear, int& iMonth, int& iDay, int& iHour, int& iMinute, int& iSecond, int& iUSecond)
{
	time_t tTime = (time_t)((long)(m_usec/MEGA));
	struct tm *pstTime = localtime(&tTime);
	if ( pstTime == NULL ) {
		return false;
	}

	iYear = pstTime->tm_year + 1900;
	iMonth = pstTime->tm_mon + 1;
	iDay = pstTime->tm_mday;
	iHour = pstTime->tm_hour;
	iMinute = pstTime->tm_min;
	iSecond = pstTime->tm_sec;
	iUSecond = (int)(m_usec%MEGA);

	return true;
}

bool CUTime::SetTime(char *pszTime)	// YYYY:MM:DD:HH:MM:SS.mmmmmm
{
	char szTemp[256];
	int iNTriedToCreate;
	CiStrCpy(szTemp, pszTime, sizeof(szTemp), &iNTriedToCreate);
	char *ptr = szTemp;
	char *saveptr =NULL;

	int iYear, iMonth, iDay, iHour, iMinute, iSecond, iUSecond;

	char *token;
	token = CiStrtok(ptr, ":." , &saveptr);
	iYear = atoi(token);

	token = CiStrtok(NULL, ":." , &saveptr);
	iMonth = atoi(token);

	token = CiStrtok(NULL, ":." , &saveptr);
	iDay = atoi(token);

	token = CiStrtok(NULL, ":." , &saveptr);
	iHour = atoi(token);

	token = CiStrtok(NULL, ":." , &saveptr);
	iMinute = atoi(token);

	token = CiStrtok(NULL, ":." , &saveptr);
	iSecond = atoi(token);

	token = CiStrtok(NULL, ":." , &saveptr);
	iUSecond = atoi(token);

	return SetTime(iYear, iMonth, iDay, iHour, iMinute, iSecond, iUSecond);
}

bool CUTime::GetTime(char **ppszTime)
{
	int iYear, iMonth, iDay, iHour, iMinute, iSecond, iUSecond;
	if ( !GetTime(iYear, iMonth, iDay, iHour, iMinute, iSecond, iUSecond) ) {
		return false;
	}

	int iStringLength = static_cast<int>(strlen("YYYY:MM:DD:HH:MM:SS.mmmmmm") + 1);
	char *pszTimeTemp = new char[iStringLength];

	sprintf( pszTimeTemp, "%04d:%02d:%02d:%02d:%02d:%02d.%06d", iYear, iMonth, iDay, iHour, iMinute, iSecond, iUSecond);

	*ppszTime = pszTimeTemp;

	return true;

	// delete the *ppszTime outside
}
//
//// frame operation
//bool CUTime::SetFrameTime(long long llFrame, float fFrameRate)
//{
//	long long microseconds = CUTimeSpan::FrameToUSecond(llFrame, fFrameRate);
//	return SetTime(microseconds);
//}
//
//bool CUTime::GetFrameTime(long long& llFrame, float fFrameRate)
//{
//	long long llMSecond;
//	if ( !GetTime(llMSecond) ) {
//		return false;
//	}
//
//	llFrame = CMTimeSpan2::MSecondToFrame(llMSecond, fFrameRate);
//	return true;
//}
//
//bool CUTime::SetFrameTime(int iYear, int iMonth, int iDay, int iHour, int iMinute, int iSecond, int iFrame, float fFrameRate)
//{
//	long long llMSecond = CMTimeSpan2::FrameToMSecond(iFrame, fFrameRate);
//	return SetTime(iYear, iMonth, iDay, iHour, iMinute, iSecond, (int)llMSecond);
//}
//
//bool CUTime::GetFrameTime(int& iYear, int& iMonth, int& iDay, int& iHour, int& iMinute, int& iSecond, int& iFrame, float fFrameRate)
//{
//	int iMSecond;
//	if ( !GetTime(iYear, iMonth, iDay, iHour, iMinute, iSecond, iMSecond) ) {
//		return false;
//	}
//
//	iFrame = (int)CMTimeSpan2::MSecondToFrame(iMSecond, fFrameRate);
//
//	return true;
//}
//
//bool CUTime::SetFrameTime(char *pszFrameTime, float fFrameRate)	// YYYY:MM:DD:HH:MM:SS.ff
//{
//	int iYear, iMonth, iDay, iHour, iMinute, iSecond, iFrame;
//
//	char szTemp[256];
//	int iNTriedToCreate;
//	CiStrCpy(szTemp, pszFrameTime, sizeof(szTemp), &iNTriedToCreate);
//	char *ptr = szTemp;
//	char *saveptr = NULL;
//
//	char *token;
//	token = CiStrtok(ptr, ":." , &saveptr);
//	iYear = atoi(token);
//
//	token = CiStrtok(NULL, ":.", &saveptr);
//	iMonth = atoi(token);
//
//	token = CiStrtok(NULL, ":.", &saveptr);
//	iDay = atoi(token);
//
//	token = CiStrtok(NULL, ":.", &saveptr);
//	iHour = atoi(token);
//
//	token = CiStrtok(NULL, ":.", &saveptr);
//	iMinute = atoi(token);
//
//	token = CiStrtok(NULL, ":.", &saveptr);
//	iSecond = atoi(token);
//
//	token = CiStrtok(NULL, ":.", &saveptr);
//	iFrame = atoi(token);
//
//	return SetFrameTime(iYear, iMonth, iDay, iHour, iMinute, iSecond, iFrame, fFrameRate);
//}
//
//bool CUTime::GetFrameTime(char **ppszFrameTime, float fFrameRate)
//{
//	int iYear, iMonth, iDay, iHour, iMinute, iSecond, iFrame;
//	if ( !GetFrameTime(iYear, iMonth, iDay, iHour, iMinute, iSecond, iFrame, fFrameRate) ) {
//		return false;
//	}
//
//	int iStringLength = static_cast<int>(strlen("YYYY:MM:DD:HH:MM:SS.ff") + 1);
//	char *pszFrameTimeTemp = new char[iStringLength];
//	int iNTriedToCreate;
//
//	char szBuffer[256];
//	sprintf( szBuffer, "%04d", iYear );
//	CiStrCpy(pszFrameTimeTemp, szBuffer, iStringLength, &iNTriedToCreate);
//	CiStrCat(pszFrameTimeTemp, ":", iStringLength, &iNTriedToCreate);
//
//	sprintf( szBuffer, "%02d", iMonth );
//	CiStrCat(pszFrameTimeTemp, szBuffer, iStringLength, &iNTriedToCreate);
//	CiStrCat(pszFrameTimeTemp, ":", iStringLength, &iNTriedToCreate);
//
//	sprintf( szBuffer, "%02d", iDay );
//	CiStrCat(pszFrameTimeTemp, szBuffer, iStringLength, &iNTriedToCreate);
//	CiStrCat(pszFrameTimeTemp, ":", iStringLength, &iNTriedToCreate);
//
//	sprintf( szBuffer, "%02d", iHour );
//	CiStrCat(pszFrameTimeTemp, szBuffer, iStringLength, &iNTriedToCreate);
//	CiStrCat(pszFrameTimeTemp, ":", iStringLength, &iNTriedToCreate);
//
//	sprintf( szBuffer, "%02d", iMinute );
//	CiStrCat(pszFrameTimeTemp, szBuffer, iStringLength, &iNTriedToCreate);
//	CiStrCat(pszFrameTimeTemp, ":", iStringLength, &iNTriedToCreate);
//
//	sprintf( szBuffer, "%02d", iSecond );
//	CiStrCat(pszFrameTimeTemp, szBuffer, iStringLength, &iNTriedToCreate);
//	CiStrCat(pszFrameTimeTemp, ".", iStringLength, &iNTriedToCreate);
//
//	sprintf( szBuffer, "%02d", iFrame );
//	CiStrCat(pszFrameTimeTemp, szBuffer, iStringLength, &iNTriedToCreate);
//
//	*ppszFrameTime = pszFrameTimeTemp;
//
//	return true;
//
//	// delete the *ppszFrameTime outside
//}
//
//// by sasgas 2003.8.2
//char* CUTime::GetFullStringforDBAPI( int iCase )
//{
//	int iYear = 0, iMonth = 0, iDay = 0, iHour = 0, iMinute = 0, iSecond = 0, iMSecond = 0;
//
//	if ( !GetTime(iYear, iMonth, iDay, iHour, iMinute, iSecond, iMSecond) )
//	{
//		return NULL;
//	}
//
//
//	int iStringLength = static_cast<int>(strlen("YYYYMMDDHHMMSSmmm") + 1);
//	char* pszTime = new char[iStringLength];
//
//	if( iCase == MTIME_CASE_FRAMECOUNT )
//	{
//		int iFrameCount = MsecToFrameCnt( iMSecond );
//
//		sprintf( pszTime, "%04d%02d%02d%02d%02d%02d%02d",
//			iYear,
//			iMonth,
//			iDay,
//			iHour,
//			iMinute,
//			iSecond,
//			iFrameCount );
//	}else
//	{
//		sprintf( pszTime, "%04d%02d%02d%02d%02d%02d%03d",
//			iYear,
//			iMonth,
//			iDay,
//			iHour,
//			iMinute,
//			iSecond,
//			iMSecond );
//	}
//
//	return pszTime;
//
//	// delete the pszTime outside
//}
//
//// by sasgas 2003.8.2
//bool CUTime::Set(const char *pszTime)	// YYYYMMDDHHMMSSmmm
//{
//	if ( CiStrLen("YYYYMMDDHHMMSSmmm") != CiStrLen(pszTime) )
//	{
//		return false;
//	}
//
//	char szTemp[256];
//	const char *ptr = pszTime;
//
//	int iYear = 0, iMonth = 0, iDay = 0, iHour = 0, iMinute = 0, iSecond = 0, iMSecond = 0;
//
//	strncpy(szTemp, ptr, 4);		// YYYY
//	szTemp[4] = '\0';
//	iYear = atoi(szTemp);
//	ptr += 4;
//
//	strncpy(szTemp, ptr, 2);		// MM
//	szTemp[2] = '\0';
//	iMonth = atoi(szTemp);
//	ptr += 2;
//
//	strncpy(szTemp, ptr, 2);		// DD
//	szTemp[2] = '\0';
//	iDay = atoi(szTemp);
//	ptr += 2;
//
//	strncpy(szTemp, ptr, 2);		//HH
//	szTemp[2] = '\0';
//	iHour = atoi(szTemp);
//	ptr += 2;
//
//	strncpy(szTemp, ptr, 2);		//MM
//	szTemp[2] = '\0';
//	iMinute = atoi(szTemp);
//	ptr += 2;
//
//	strncpy(szTemp, ptr, 2);		//SS
//	szTemp[2] = '\0';
//	iSecond = atoi(szTemp);
//	ptr += 2;
//
//	strncpy(szTemp, ptr, 3);		//mmm
//	szTemp[3] = '\0';
//	iMSecond = atoi(szTemp);
//	ptr += 3;
//
//	return SetTime(iYear, iMonth, iDay, iHour, iMinute, iSecond, iMSecond);
//}
//
//// by sasgas 2003.8.2
//CUTime::CUTime(const char* pszTime)
//{
//	Set(pszTime);
//}
//
//int CUTime::MsecToFrameCnt(int iMillieSecond)
//{
//	return (int)((double)(iMillieSecond*g_dFramePerSec/1000 + 0.5));
//}
//
//int CUTime::GetYear()
//{
//	time_t tTime = (time_t)(m_llMSecond / 1000);
//	struct tm *pstTime = localtime(&tTime);
//	if ( pstTime == NULL ) {
//		return false;
//	}
//
//	return ( pstTime->tm_year + 1900 );
//
//}
//
//int CUTime::GetMonth()
//{
//
//	time_t tTime = (time_t)(m_llMSecond / 1000);
//	struct tm *pstTime = localtime(&tTime);
//	if ( pstTime == NULL ) {
//		return false;
//	}
//
//
//	return ( pstTime->tm_mon + 1 );
//
//}
