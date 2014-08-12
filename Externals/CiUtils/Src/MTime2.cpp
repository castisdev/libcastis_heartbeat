// MTime2.cpp: implementation of the CMTime2 class.
//
//////////////////////////////////////////////////////////////////////

#include "internal_CiUtils.h"

#include "CiSafeString.h"
#include "MTimeSpan2.h"
#include "MTime2.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

double g_dFramePerSec = 29.97;		// NTSC -> 29.97 , PAL -> 25

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMTime2::CMTime2()
{
#ifdef WIN32
	struct _timeb currentTime;
	_ftime( &currentTime );

	m_llMSecond = ( long long )currentTime.time * ( long long)1000 + ( long long )currentTime.millitm;
#else
	struct timeb currentTime;
	ftime( &currentTime );

	m_llMSecond = ( long long )currentTime.time * ( long long)1000 + ( long long )currentTime.millitm;
#endif
}

CMTime2::~CMTime2()
{
}

CMTime2 CMTime2::operator+(long long llMSeconds)
{
	CMTime2 mtResult;
	mtResult.m_llMSecond = this->m_llMSecond + llMSeconds;

	return mtResult;
}

CMTime2 CMTime2::operator+(CMTimeSpan2 mtsTimeSpan)
{
	long long llMSeconds;
	mtsTimeSpan.GetTimeSpan(llMSeconds);

	CMTime2 mtResult;
	mtResult.m_llMSecond = this->m_llMSecond + llMSeconds;

	return mtResult;
}

CMTime2 CMTime2::operator-(long long llMSeconds)
{
	CMTime2 mtResult;
	mtResult.m_llMSecond = this->m_llMSecond - llMSeconds;

	return mtResult;
}

CMTime2 CMTime2::operator-(CMTimeSpan2 mtsTimeSpan)
{
	long long llMSeconds;
	mtsTimeSpan.GetTimeSpan(llMSeconds);

	CMTime2 mtResult;
	mtResult.m_llMSecond = this->m_llMSecond - llMSeconds;

	return mtResult;
}

CMTimeSpan2 CMTime2::operator-(CMTime2 mtTime)
{
	long long llMSeconds = this->m_llMSecond - mtTime.m_llMSecond;

	CMTimeSpan2 mtsResult;
	mtsResult.SetTimeSpan(llMSeconds);

	return mtsResult;
}

bool CMTime2::operator==(CMTime2 mtTime)
{
	return (this->m_llMSecond == mtTime.m_llMSecond);
}

bool CMTime2::operator>(CMTime2 mtTime)
{
	return (this->m_llMSecond > mtTime.m_llMSecond);
}

bool CMTime2::operator<(CMTime2 mtTime)
{
	return (this->m_llMSecond < mtTime.m_llMSecond);
}

bool CMTime2::operator<=(CMTime2 mtTime)
{
	return (this->m_llMSecond <= mtTime.m_llMSecond);
}

bool CMTime2::operator>=(CMTime2 mtTime)
{
	return (this->m_llMSecond >= mtTime.m_llMSecond);
}

// millisecond operation
bool CMTime2::SetTime(long long llMSecond)
{
	m_llMSecond = llMSecond;
	return true;
}

bool CMTime2::GetTime(long long& llMSecond)
{
	llMSecond = m_llMSecond;
	return true;
}

bool CMTime2::SetTime(int iYear, int iMonth, int iDay, int iHour, int iMinute, int iSecond, int iMSecond)
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

	m_llMSecond = (long long)tTime * (long long)1000 + (long long)iMSecond;

	return true;
}

bool CMTime2::GetTime(int& iYear, int& iMonth, int& iDay, int& iHour, int& iMinute, int& iSecond, int& iMSecond)
{
	time_t tTime = (time_t)(m_llMSecond / 1000);
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
	iMSecond = (int)(m_llMSecond % 1000);

	return true;
}

bool CMTime2::SetTime(char *pszTime)	// YYYY:MM:DD:HH:MM:SS.mmm
{
	char szTemp[256];
	int iNTriedToCreate;
	CiStrCpy(szTemp, pszTime, sizeof(szTemp), &iNTriedToCreate);
	char *ptr = szTemp;
	char *saveptr =NULL;

	int iYear, iMonth, iDay, iHour, iMinute, iSecond, iMSecond;

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
	iMSecond = atoi(token);

	return SetTime(iYear, iMonth, iDay, iHour, iMinute, iSecond, iMSecond);
}

bool CMTime2::GetTime(char **ppszTime)
{
	int iYear, iMonth, iDay, iHour, iMinute, iSecond, iMSecond;
	if ( !GetTime(iYear, iMonth, iDay, iHour, iMinute, iSecond, iMSecond) ) {
		return false;
	}

	int iStringLength = static_cast<int>(strlen("YYYY:MM:DD:HH:MM:SS.mmm") + 1);
	char *pszTimeTemp = new char[iStringLength];

	sprintf( pszTimeTemp, "%04d:%02d:%02d:%02d:%02d:%02d:%03d", iYear, iMonth, iDay, iHour, iMinute, iSecond, iMSecond);

	*ppszTime = pszTimeTemp;

	return true;

	// delete the *ppszTime outside
}

// frame operation
bool CMTime2::SetFrameTime(long long llFrame, float fFrameRate)
{
	long long llMSecond = CMTimeSpan2::FrameToMSecond(llFrame, fFrameRate);
	return SetTime(llMSecond);
}

bool CMTime2::GetFrameTime(long long& llFrame, float fFrameRate)
{
	long long llMSecond;
	if ( !GetTime(llMSecond) ) {
		return false;
	}

	llFrame = CMTimeSpan2::MSecondToFrame(llMSecond, fFrameRate);
	return true;
}

bool CMTime2::SetFrameTime(int iYear, int iMonth, int iDay, int iHour, int iMinute, int iSecond, int iFrame, float fFrameRate)
{
	long long llMSecond = CMTimeSpan2::FrameToMSecond(iFrame, fFrameRate);
	return SetTime(iYear, iMonth, iDay, iHour, iMinute, iSecond, (int)llMSecond);
}

bool CMTime2::GetFrameTime(int& iYear, int& iMonth, int& iDay, int& iHour, int& iMinute, int& iSecond, int& iFrame, float fFrameRate)
{
	int iMSecond;
	if ( !GetTime(iYear, iMonth, iDay, iHour, iMinute, iSecond, iMSecond) ) {
		return false;
	}

	iFrame = (int)CMTimeSpan2::MSecondToFrame(iMSecond, fFrameRate);

	return true;
}

bool CMTime2::SetFrameTime(char *pszFrameTime, float fFrameRate)	// YYYY:MM:DD:HH:MM:SS.ff
{
	int iYear, iMonth, iDay, iHour, iMinute, iSecond, iFrame;

	char szTemp[256];
	int iNTriedToCreate;
	CiStrCpy(szTemp, pszFrameTime, sizeof(szTemp), &iNTriedToCreate);
	char *ptr = szTemp;
	char *saveptr = NULL;

	char *token;
	token = CiStrtok(ptr, ":." , &saveptr);
	iYear = atoi(token);

	token = CiStrtok(NULL, ":.", &saveptr);
	iMonth = atoi(token);

	token = CiStrtok(NULL, ":.", &saveptr);
	iDay = atoi(token);

	token = CiStrtok(NULL, ":.", &saveptr);
	iHour = atoi(token);

	token = CiStrtok(NULL, ":.", &saveptr);
	iMinute = atoi(token);

	token = CiStrtok(NULL, ":.", &saveptr);
	iSecond = atoi(token);

	token = CiStrtok(NULL, ":.", &saveptr);
	iFrame = atoi(token);

	return SetFrameTime(iYear, iMonth, iDay, iHour, iMinute, iSecond, iFrame, fFrameRate);
}

bool CMTime2::GetFrameTime(char **ppszFrameTime, float fFrameRate)
{
	int iYear, iMonth, iDay, iHour, iMinute, iSecond, iFrame;
	if ( !GetFrameTime(iYear, iMonth, iDay, iHour, iMinute, iSecond, iFrame, fFrameRate) ) {
		return false;
	}

	int iStringLength = static_cast<int>(strlen("YYYY:MM:DD:HH:MM:SS.ff") + 1);
	char *pszFrameTimeTemp = new char[iStringLength];
	int iNTriedToCreate;

	char szBuffer[256];
	sprintf( szBuffer, "%04d", iYear );
	CiStrCpy(pszFrameTimeTemp, szBuffer, iStringLength, &iNTriedToCreate);
	CiStrCat(pszFrameTimeTemp, ":", iStringLength, &iNTriedToCreate);

	sprintf( szBuffer, "%02d", iMonth );
	CiStrCat(pszFrameTimeTemp, szBuffer, iStringLength, &iNTriedToCreate);
	CiStrCat(pszFrameTimeTemp, ":", iStringLength, &iNTriedToCreate);

	sprintf( szBuffer, "%02d", iDay );
	CiStrCat(pszFrameTimeTemp, szBuffer, iStringLength, &iNTriedToCreate);
	CiStrCat(pszFrameTimeTemp, ":", iStringLength, &iNTriedToCreate);

	sprintf( szBuffer, "%02d", iHour );
	CiStrCat(pszFrameTimeTemp, szBuffer, iStringLength, &iNTriedToCreate);
	CiStrCat(pszFrameTimeTemp, ":", iStringLength, &iNTriedToCreate);

	sprintf( szBuffer, "%02d", iMinute );
	CiStrCat(pszFrameTimeTemp, szBuffer, iStringLength, &iNTriedToCreate);
	CiStrCat(pszFrameTimeTemp, ":", iStringLength, &iNTriedToCreate);

	sprintf( szBuffer, "%02d", iSecond );
	CiStrCat(pszFrameTimeTemp, szBuffer, iStringLength, &iNTriedToCreate);
	CiStrCat(pszFrameTimeTemp, ".", iStringLength, &iNTriedToCreate);

	sprintf( szBuffer, "%02d", iFrame );
	CiStrCat(pszFrameTimeTemp, szBuffer, iStringLength, &iNTriedToCreate);

	*ppszFrameTime = pszFrameTimeTemp;

	return true;

	// delete the *ppszFrameTime outside
}

// by sasgas 2003.8.2
char* CMTime2::GetFullStringforDBAPI( int iCase )
{
	int iYear = 0, iMonth = 0, iDay = 0, iHour = 0, iMinute = 0, iSecond = 0, iMSecond = 0;

	if ( !GetTime(iYear, iMonth, iDay, iHour, iMinute, iSecond, iMSecond) )
	{
		return NULL;
	}


	int iStringLength = static_cast<int>(strlen("YYYYMMDDHHMMSSmmm") + 1);
	char* pszTime = new char[iStringLength];

	if( iCase == MTIME_CASE_FRAMECOUNT )
	{
		int iFrameCount = MsecToFrameCnt( iMSecond );

		sprintf( pszTime, "%04d%02d%02d%02d%02d%02d%02d",
														iYear,
														iMonth,
														iDay,
														iHour,
														iMinute,
														iSecond,
														iFrameCount );
	}else
	{
		sprintf( pszTime, "%04d%02d%02d%02d%02d%02d%03d",
														iYear,
														iMonth,
														iDay,
														iHour,
														iMinute,
														iSecond,
														iMSecond );
	}

	return pszTime;

	// delete the pszTime outside
}

// by sasgas 2003.8.2
bool CMTime2::Set(const char *pszTime)	// YYYYMMDDHHMMSSmmm
{
	if ( CiStrLen("YYYYMMDDHHMMSSmmm") != CiStrLen(pszTime) )
	{
		return false;
	}

	char szTemp[256];
	const char *ptr = pszTime;

	int iYear = 0, iMonth = 0, iDay = 0, iHour = 0, iMinute = 0, iSecond = 0, iMSecond = 0;

	strncpy(szTemp, ptr, 4);		// YYYY
	szTemp[4] = '\0';
	iYear = atoi(szTemp);
	ptr += 4;

	strncpy(szTemp, ptr, 2);		// MM
	szTemp[2] = '\0';
	iMonth = atoi(szTemp);
	ptr += 2;

	strncpy(szTemp, ptr, 2);		// DD
	szTemp[2] = '\0';
	iDay = atoi(szTemp);
	ptr += 2;

	strncpy(szTemp, ptr, 2);		//HH
	szTemp[2] = '\0';
	iHour = atoi(szTemp);
	ptr += 2;

	strncpy(szTemp, ptr, 2);		//MM
	szTemp[2] = '\0';
	iMinute = atoi(szTemp);
	ptr += 2;

	strncpy(szTemp, ptr, 2);		//SS
	szTemp[2] = '\0';
	iSecond = atoi(szTemp);
	ptr += 2;

	strncpy(szTemp, ptr, 3);		//mmm
	szTemp[3] = '\0';
	iMSecond = atoi(szTemp);
	ptr += 3;

	return SetTime(iYear, iMonth, iDay, iHour, iMinute, iSecond, iMSecond);
}

// by sasgas 2003.8.2
CMTime2::CMTime2(const char* pszTime)
{
	Set(pszTime);
}

int CMTime2::MsecToFrameCnt(int iMillieSecond)
{
	return (int)((double)(iMillieSecond*g_dFramePerSec/1000 + 0.5));
}

int CMTime2::GetYear()
{
	time_t tTime = (time_t)(m_llMSecond / 1000);
	struct tm *pstTime = localtime(&tTime);
	if ( pstTime == NULL ) {
		return false;
	}

	return ( pstTime->tm_year + 1900 );

}

int CMTime2::GetMonth()
{

	time_t tTime = (time_t)(m_llMSecond / 1000);
	struct tm *pstTime = localtime(&tTime);
	if ( pstTime == NULL ) {
		return false;
	}


	return ( pstTime->tm_mon + 1 );

}
