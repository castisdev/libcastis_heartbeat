#include "internal_CiUtils.h"

#include "CiSafeString.h"
#include "UTimeSpan.h"


#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

#define MEGA (1000000)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUTimeSpan::CUTimeSpan(void)
{
	m_usec = 0;
}

CUTimeSpan::CUTimeSpan(int iDays, int iHours, int iMinutes, int iSeconds, int iUSeconds )
{
	SetTimeSpan(iDays, iHours, iMinutes, iSeconds, iUSeconds);
}

CUTimeSpan::~CUTimeSpan()
{

}

//long long CUTimeSpan::FrameToMSecond(long long llFrames, float fFrameRate)
//{
//	int iTenPower = 1;
//
//	while ( iTenPower*fFrameRate != (int)(iTenPower*fFrameRate) ) {
//		iTenPower *= 10;
//	}
//
//	long long llQuotient = (long long)(llFrames/(iTenPower*fFrameRate));
//	long long llRemainder = llFrames-llQuotient*iTenPower;
//
//	double dFrameMSeconds = 1000.0 / (double)fFrameRate;
//
//	return (long long)(llQuotient*iTenPower*1000
//		+ (llRemainder*dFrameMSeconds+0.5));
//}
//
//long long CUTimeSpan::MSecondToFrame(long long llMSeconds, float fFrameRate)
//{
//	int iTenPower = 1;
//
//	while ( iTenPower*fFrameRate != (int)(iTenPower*fFrameRate) ) {
//		iTenPower *= 10;
//	}
//
//	long long llQuotient = (long long)(llMSeconds/(iTenPower*1000));
//	long long llRemainder = llMSeconds-llQuotient*(iTenPower*1000);
//
//	double dFrameMSeconds = 1000.0 / (double)fFrameRate;
//
//	return (long long)(llQuotient*iTenPower*fFrameRate
//		+ (llRemainder/dFrameMSeconds+0.5));
//}

CUTimeSpan CUTimeSpan::operator+(CUTimeSpan timeSpan)
{
	CUTimeSpan result;
	result.SetTimeSpan(m_usec + timeSpan.m_usec);
	return result;
}

CUTimeSpan CUTimeSpan::operator-(CUTimeSpan timeSpan)
{
	CUTimeSpan result;
	result.SetTimeSpan(m_usec - timeSpan.m_usec);
	return result;
}

bool CUTimeSpan::operator==(CUTimeSpan timeSpan)
{
	return (m_usec == timeSpan.m_usec);
}

bool CUTimeSpan::operator>(CUTimeSpan timeSpan)
{
	return (m_usec > timeSpan.m_usec);
}

bool CUTimeSpan::operator<(CUTimeSpan timeSpan)
{
	return (m_usec < timeSpan.m_usec);
}

bool CUTimeSpan::operator<=(CUTimeSpan timeSpan)
{
	return (m_usec <= timeSpan.m_usec);
}

bool CUTimeSpan::operator>=(CUTimeSpan timeSpan)
{
	return (m_usec >= timeSpan.m_usec);
}

// microsecond operation
bool CUTimeSpan::SetTimeSpan(long long microseconds)
{
	m_usec = microseconds;
	return true;
}

bool CUTimeSpan::GetTimeSpan(long long& microseconds)
{
	microseconds = m_usec;
	return true;
}

bool CUTimeSpan::SetTimeSpan(int iDays, int iHours, int iMinutes, int iSeconds, int iUSeconds)
{
	long sec = iDays*24*60*60 + iHours*60*60 + iMinutes*60 + iSeconds;
	m_usec = (long long)sec*MEGA + iUSeconds;
	return true;
}

bool CUTimeSpan::GetTimeSpan(int& iDays, int& iHours, int& iMinutes, int& iSeconds, int& iUSeconds)
{
	long sec = (long)(m_usec/MEGA);
	long usec =(long)(m_usec%MEGA);

	iDays = (int)(sec/(24*60*60));
	sec -= iDays*24*60*60;

	iHours = (int)(sec/(60*60));
	sec -= iHours*60*60;

	iMinutes = (int)(sec/60);
	sec-= iMinutes*60;

	iSeconds = (int)sec;

	iUSeconds = usec;

	return true;
}

bool CUTimeSpan::SetTimeSpan(char *pszTimeSpan)	// (+)/-DD:HH:MM:SS.mmmmmm
{
	int iDays, iHours, iMinutes, iSeconds, iUSeconds;
	int minus = 1;

	char szTemp[256];
	int iNTriedToCreate;
	CiStrCpy(szTemp, pszTimeSpan, sizeof(szTemp), &iNTriedToCreate);
	char *ptr = szTemp;

	if ( *ptr == '+' ) {
		minus = -1;
		ptr++;
	}
	else if ( *ptr == '-' ) {
		minus = 1;
		ptr++;
	}
	else {
		minus = -1;
	}

	char *token;
	char *saveptr = NULL;
	token = CiStrtok(ptr, ":.", &saveptr);
	iDays = atoi(token);

	token = CiStrtok(NULL, ":.", &saveptr);
	iHours = atoi(token);

	token = CiStrtok(NULL, ":.", &saveptr);
	iMinutes = atoi(token);

	token = CiStrtok(NULL, ":.", &saveptr);
	iSeconds = atoi(token);

	token = CiStrtok(NULL, ":.", &saveptr);
	iUSeconds = atoi(token);

	SetTimeSpan(iDays*minus, iHours*minus, iMinutes*minus, iSeconds*minus, iUSeconds*minus);
	return true;
}

bool CUTimeSpan::GetTimeSpan(char **ppszTimeSpan)		// (+)/-DD:HH:MM:SS.mmmmmm
{
	int iDays, iHours, iMinutes, iSeconds, iUSeconds;
	if ( !GetTimeSpan(iDays, iHours, iMinutes, iSeconds, iUSeconds) ) {
		return false;
	}

	bool bMinus = false;
	if ( iUSeconds < 0 ) {
		bMinus = true;
		iDays = abs(iDays);
		iHours = abs(iHours);
		iMinutes = abs(iMinutes);
		iSeconds = abs(iSeconds);
		iUSeconds = abs(iUSeconds);
	}
	else {
		bMinus = false;
	}

	int iStringLength = static_cast<int>(strlen("-DD:HH:MM:SS.mmmmmm") + 1);
	char *pszTimeSpanTemp = new char[iStringLength];
	int iNTriedToCreate;
	if ( bMinus ) {
		CiStrCpy(pszTimeSpanTemp, "-", iStringLength, &iNTriedToCreate);
	}
	else {
		pszTimeSpanTemp[0] = '\0';
	}

	char szBuffer[256];
	sprintf( szBuffer, "%d", iDays );
	CiStrCat(pszTimeSpanTemp, szBuffer, iStringLength, &iNTriedToCreate);
	CiStrCat(pszTimeSpanTemp, ":", iStringLength, &iNTriedToCreate);

	sprintf( szBuffer, "%d", iHours );
	CiStrCat(pszTimeSpanTemp, szBuffer, iStringLength, &iNTriedToCreate);
	CiStrCat(pszTimeSpanTemp, ":", iStringLength, &iNTriedToCreate);

	sprintf( szBuffer, "%d", iMinutes );
	CiStrCat(pszTimeSpanTemp, szBuffer, iStringLength, &iNTriedToCreate);
	CiStrCat(pszTimeSpanTemp, ":", iStringLength, &iNTriedToCreate);

	sprintf( szBuffer, "%d", iSeconds );
	CiStrCat(pszTimeSpanTemp, szBuffer, iStringLength, &iNTriedToCreate);
	CiStrCat(pszTimeSpanTemp, ".", iStringLength, &iNTriedToCreate);

	sprintf( szBuffer, "%d", iUSeconds );
	CiStrCat(pszTimeSpanTemp, szBuffer, iStringLength, &iNTriedToCreate);

	*ppszTimeSpan = pszTimeSpanTemp;

	return true;

	// delete the *ppszTimeSpan outside
}

// frame operation
//bool CUTimeSpan::SetFrameTimeSpan(long long llFrames, float fFrameRate)
//{
//	m_llMSeconds = FrameToMSecond(llFrames, fFrameRate);
//	return true;
//}
//
//bool CUTimeSpan::GetFrameTimeSpan(long long& llFrames, float fFrameRate)
//{
//	llFrames = MSecondToFrame(m_llMSeconds, fFrameRate);
//	return true;
//}
//
//bool CUTimeSpan::SetFrameTimeSpan(int iDays, int iHours, int iMinutes, int iSeconds, int iFrames, float fFrameRate)
//{
//	int iMSeconds = (int)FrameToMSecond(iFrames, fFrameRate);
//	return SetTimeSpan(iDays, iHours, iMinutes, iSeconds, iMSeconds);
//}
//
//bool CUTimeSpan::GetFrameTimeSpan(int& iDays,int& iHours, int& iMinutes, int& iSeconds, int& iFrames, float fFrameRate)
//{
//	int iMSeconds;
//	if ( !GetTimeSpan(iDays, iHours, iMinutes, iSeconds, iMSeconds) ) {
//		return false;
//	}
//	iFrames = (int)MSecondToFrame(iMSeconds, fFrameRate);
//	return true;
//}
//
//bool CUTimeSpan::SetFrameTimeSpan(char *pszFrameTimeSpan, float fFrameRate)	// (+)/-DD:HH:MM:SS.ff
//{
//	int iDays, iHours, iMinutes, iSeconds, iFrames;
//	bool bMinus;
//
//	char szTemp[256];
//	int iNTriedToCreate;
//	CiStrCpy(szTemp, pszFrameTimeSpan, sizeof(szTemp), &iNTriedToCreate);
//	char *ptr = szTemp;
//
//	if ( *ptr == '+' ) {
//		bMinus = false;
//		ptr++;
//	}
//	else if ( *ptr == '-' ) {
//		bMinus = true;
//		ptr++;
//	}
//	else {
//		bMinus = false;
//	}
//
//	char *token;
//	char *saveptr = NULL;
//
//	token = CiStrtok(ptr, ":.", &saveptr);
//	iDays = atoi(token);
//
//	token = CiStrtok(NULL, ":.", &saveptr);
//	iHours = atoi(token);
//
//	token = CiStrtok(NULL, ":.", &saveptr);
//	iMinutes = atoi(token);
//
//	token = CiStrtok(NULL, ":.", &saveptr);
//	iSeconds = atoi(token);
//
//	token = CiStrtok(NULL, ":.", &saveptr);
//	iFrames = atoi(token);
//
//	return SetFrameTimeSpan(iDays, iHours, iMinutes, iSeconds, iFrames, fFrameRate);
//}
//
//bool CUTimeSpan::GetFrameTimeSpan(char **ppszFrameTimeSpan, float fFrameRate)	// (+)/-DD:HH:MM:SS.ff
//{
//	int iDays, iHours, iMinutes, iSeconds, iFrames;
//	if ( !GetFrameTimeSpan(iDays, iHours, iMinutes, iSeconds, iFrames, fFrameRate) ) {
//		return false;
//	}
//
//	bool bMinus;
//	if ( m_llMSeconds < 0 ) {
//		bMinus = true;
//		iDays = abs(iDays);
//		iHours = abs(iHours);
//		iMinutes = abs(iMinutes);
//		iSeconds = abs(iSeconds);
//		iFrames = abs(iFrames);
//	}
//	else {
//		bMinus = false;
//	}
//
//	int iStringLength = static_cast<int>(strlen("-DD:HH:MM:SS.ff") + 1);
//	char *pszFrameTimeSpanTemp = new char[iStringLength];
//	int iNTriedToCreate;
//	if ( bMinus ) {
//		CiStrCpy(pszFrameTimeSpanTemp, "-", iStringLength, &iNTriedToCreate);
//	}
//	else {
//		pszFrameTimeSpanTemp[0] = '\0';
//	}
//
//	char szBuffer[256];
//	sprintf( szBuffer, "%d", iDays );
//	CiStrCat(pszFrameTimeSpanTemp, szBuffer, iStringLength, &iNTriedToCreate);
//	CiStrCat(pszFrameTimeSpanTemp, ":", iStringLength, &iNTriedToCreate);
//
//	sprintf( szBuffer, "%d", iHours );
//	CiStrCat(pszFrameTimeSpanTemp, szBuffer, iStringLength, &iNTriedToCreate);
//	CiStrCat(pszFrameTimeSpanTemp, ":", iStringLength, &iNTriedToCreate);
//
//	sprintf( szBuffer, "%d", iMinutes );
//	CiStrCat(pszFrameTimeSpanTemp, szBuffer, iStringLength, &iNTriedToCreate);
//	CiStrCat(pszFrameTimeSpanTemp, ":", iStringLength, &iNTriedToCreate);
//
//	sprintf( szBuffer, "%d", iSeconds );
//	CiStrCat(pszFrameTimeSpanTemp, szBuffer, iStringLength, &iNTriedToCreate);
//	CiStrCat(pszFrameTimeSpanTemp, ".", iStringLength, &iNTriedToCreate);
//
//	sprintf( szBuffer, "%d", iFrames );
//	CiStrCat(pszFrameTimeSpanTemp, szBuffer, iStringLength, &iNTriedToCreate);
//
//	*ppszFrameTimeSpan = pszFrameTimeSpanTemp;
//
//	return true;
//
//	// delete the *ppszFrameTimeSpan outside
//}
//
//// by sasgas 2003.8.2
//char* CUTimeSpan::GetDurationStringforDBAPI()
//{
//	int iDays = 0, iHours = 0, iMinutes = 0, iSeconds = 0, iMSeconds = 0;
//
//	if ( !GetTimeSpan(iDays, iHours, iMinutes, iSeconds, iMSeconds) )
//	{
//		return NULL;
//	}
//
//	if ( m_llMSeconds < 0 )
//	{// 발생할 수 있는 상황인가?
//		iDays = abs(iDays);
//		iHours = abs(iHours);
//		iMinutes = abs(iMinutes);
//		iSeconds = abs(iSeconds);
//		iMSeconds = abs(iMSeconds);
//	}
//
//	int iStringLength = static_cast<int>(strlen("HHMMSSmmm") + 1);
//	char *pszTimeSpan = new char[iStringLength];
//
//	sprintf( pszTimeSpan, "%02d%02d%02d%03d",
//		iHours,
//		iMinutes,
//		iSeconds,
//		iMSeconds );
//
//
//	return pszTimeSpan;
//
//	// delete the *ppszTimeSpan outside
//}
//
//// by sasgas 2003.8.2
//bool CUTimeSpan::Set(const char* pszTimeSpan)
//{
//	char szTemp[256];
//	const char *ptr = pszTimeSpan;
//
//	int iDay = 0, iHour = 0, iMinute = 0, iSecond = 0, iMSecond = 0, iFrameCount = 0;
//
//	if( CiStrLen("HHMMSSmm") == CiStrLen( pszTimeSpan ) )
//	{
//		strncpy(szTemp, ptr, 2);		//HH
//		szTemp[2] = '\0';
//		iHour	= atoi(szTemp);
//		iDay	= iHour / 24;
//		iHour	= iHour % 24;
//		ptr += 2;
//
//		strncpy(szTemp, ptr, 2);		//MM
//		szTemp[2] = '\0';
//		iMinute = atoi(szTemp);
//		ptr += 2;
//
//		strncpy(szTemp, ptr, 2);		//SS
//		szTemp[2] = '\0';
//		iSecond = atoi(szTemp);
//		ptr += 2;
//
//		strncpy(szTemp, ptr, 2);		//mmm
//		szTemp[2] = '\0';
//		iFrameCount = atoi(szTemp);
//
//		iMSecond = (int)FrameToMSecond( (long long)iFrameCount, (float)29.97 );
//
//	}else if( CiStrLen("HHMMSSmmm") == CiStrLen( pszTimeSpan ) )
//	{//CiStrLen("HHMMSSmmm")
//
//		strncpy(szTemp, ptr, 2);		//HH
//		szTemp[2] = '\0';
//		iHour	= atoi(szTemp);
//		iDay	= iHour / 24;
//		iHour	= iHour % 24;
//		ptr		+= 2;
//
//		strncpy(szTemp, ptr, 2);		//MM
//		szTemp[2]	= '\0';
//		iMinute		= atoi(szTemp);
//		ptr			+= 2;
//
//		strncpy(szTemp, ptr, 2);		//SS
//		szTemp[2]	= '\0';
//		iSecond		= atoi(szTemp);
//		ptr			+= 2;
//
//		strncpy(szTemp, ptr, 3);		//mmm
//		szTemp[3]	= '\0';
//		iMSecond	= atoi(szTemp);
//
//	}else
//		return false;
//
//	return SetTimeSpan(iDay, iHour, iMinute, iSecond, iMSecond);
//}
//
//// by sasgas 2003.8.2
//CUTimeSpan::CUTimeSpan(const char* pszTimeSpan)
//{
//	Set(pszTimeSpan);
//}

