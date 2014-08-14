// MTimeSpan2.cpp: implementation of the CMTimeSpan2 class.
//
//////////////////////////////////////////////////////////////////////

#include "internal_CiUtils.h"
#include "CiSafeString.h"

#include "MTimeSpan2.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMTimeSpan2::CMTimeSpan2()
{
	m_llMSeconds = 0;
}

CMTimeSpan2::CMTimeSpan2(int iDays, int iHours, int iMinutes, int iSeconds, int iMSeconds )
{
	SetTimeSpan(iDays, iHours, iMinutes, iSeconds, iMSeconds);
}

CMTimeSpan2::~CMTimeSpan2()
{

}

long long CMTimeSpan2::FrameToMSecond(long long llFrames, float fFrameRate)
{
	int iTenPower = 1;

	while ( iTenPower*fFrameRate != (int)(iTenPower*fFrameRate) ) {
		iTenPower *= 10;
	}

	long long llQuotient = (long long)(llFrames/(iTenPower*fFrameRate));
	long long llRemainder = llFrames-llQuotient*iTenPower;

	double dFrameMSeconds = 1000.0 / (double)fFrameRate;

	return (long long)(llQuotient*iTenPower*1000
							+ (llRemainder*dFrameMSeconds+0.5));
}

long long CMTimeSpan2::MSecondToFrame(long long llMSeconds, float fFrameRate)
{
	int iTenPower = 1;

	while ( iTenPower*fFrameRate != (int)(iTenPower*fFrameRate) ) {
		iTenPower *= 10;
	}

	long long llQuotient = (long long)(llMSeconds/(iTenPower*1000));
	long long llRemainder = llMSeconds-llQuotient*(iTenPower*1000);

	double dFrameMSeconds = 1000.0 / (double)fFrameRate;

	return (long long)(llQuotient*iTenPower*fFrameRate
							+ (llRemainder/dFrameMSeconds+0.5));
}

CMTimeSpan2 CMTimeSpan2::operator+(CMTimeSpan2 mtsTimeSpan)
{
	long long llMSecondsOperand;
	mtsTimeSpan.GetTimeSpan(llMSecondsOperand);

	long long llMSecondsSum = this->m_llMSeconds + llMSecondsOperand;

	CMTimeSpan2 mtsResult;
	mtsResult.SetTimeSpan(llMSecondsSum);

	return mtsResult;
}

CMTimeSpan2 CMTimeSpan2::operator-(CMTimeSpan2 mtsTimeSpan)
{
	long long llMSecondsOperand;
	mtsTimeSpan.GetTimeSpan(llMSecondsOperand);

	long long llMSecondsSum = this->m_llMSeconds - llMSecondsOperand;

	CMTimeSpan2 mtsResult;
	mtsResult.SetTimeSpan(llMSecondsSum);

	return mtsResult;
}

bool CMTimeSpan2::operator==(CMTimeSpan2 mtsTimeSpan)
{
	long long llMSecondsOperand;
	mtsTimeSpan.GetTimeSpan(llMSecondsOperand);

	return (this->m_llMSeconds == llMSecondsOperand);
}

bool CMTimeSpan2::operator>(CMTimeSpan2 mtsTimeSpan)
{
	long long llMSecondsOperand;
	mtsTimeSpan.GetTimeSpan(llMSecondsOperand);

	return (this->m_llMSeconds > llMSecondsOperand);
}

bool CMTimeSpan2::operator<(CMTimeSpan2 mtsTimeSpan)
{
	long long llMSecondsOperand;
	mtsTimeSpan.GetTimeSpan(llMSecondsOperand);

	return (this->m_llMSeconds < llMSecondsOperand);
}

bool CMTimeSpan2::operator<=(CMTimeSpan2 mtsTimeSpan)
{
	long long llMSecondsOperand;
	mtsTimeSpan.GetTimeSpan(llMSecondsOperand);

	return (this->m_llMSeconds <= llMSecondsOperand);
}

bool CMTimeSpan2::operator>=(CMTimeSpan2 mtsTimeSpan)
{
	long long llMSecondsOperand;
	mtsTimeSpan.GetTimeSpan(llMSecondsOperand);

	return (this->m_llMSeconds >= llMSecondsOperand);
}

// millisecond operation
bool CMTimeSpan2::SetTimeSpan(long long llMSeconds)
{
	m_llMSeconds = llMSeconds;
	return true;
}

bool CMTimeSpan2::GetTimeSpan(long long& llMSeconds)
{
	llMSeconds = m_llMSeconds;
	return true;
}

bool CMTimeSpan2::SetTimeSpan(int iDays, int iHours, int iMinutes, int iSeconds, int iMSeconds)
{
	m_llMSeconds =
		(long long)iDays*(long long)24*(long long)60*(long long)60*(long long)1000
		+ (long long)iHours*(long long)60*(long long)60*(long long)1000
		+ (long long)iMinutes*(long long)60*(long long)1000
		+ (long long)iSeconds*(long long)1000
		+ (long long)iMSeconds;
	return true;
}

bool CMTimeSpan2::GetTimeSpan(int& iDays, int& iHours, int& iMinutes, int& iSeconds, int& iMSeconds)
{
	long long llMSeconds;
	bool bMinus;
	if ( m_llMSeconds < 0 ) {
		llMSeconds = -m_llMSeconds;
		bMinus = true;
	}
	else {
		llMSeconds = m_llMSeconds;
		bMinus = false;
	}

	iDays = (int)(llMSeconds/((long long)24*(long long)60*(long long)60*(long long)1000));
	llMSeconds -= (long long)iDays*(long long)24*(long long)60*(long long)60*(long long)1000;

	iHours = (int)(llMSeconds/((long long)60*(long long)60*(long long)1000));
	llMSeconds -= (long long)iHours*(long long)60*(long long)60*(long long)1000;

	iMinutes = (int)(llMSeconds/((long long)60*(long long)1000));
	llMSeconds -= (long long)iMinutes*(long long)60*(long long)1000;

	iSeconds = (int)(llMSeconds/((long long)1000));
	llMSeconds -= (long long)iSeconds*(long long)1000;

	iMSeconds = (int)llMSeconds;

	if ( bMinus ) {
		iDays = -iDays;
		iHours = -iHours;
		iMinutes = -iMinutes;
		iSeconds = -iSeconds;
		iMSeconds = -iMSeconds;
	}
	return true;
}

bool CMTimeSpan2::SetTimeSpan(char *pszTimeSpan)	// DD:HH:MM:SS.mmm
{
	int iDays, iHours, iMinutes, iSeconds, iMSeconds;

	char szTemp[256];
	int iNTriedToCreate;
	CiStrCpy(szTemp, pszTimeSpan, sizeof(szTemp), &iNTriedToCreate);
	char *ptr = szTemp;

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
	iMSeconds = atoi(token);

	SetTimeSpan(iDays, iHours, iMinutes, iSeconds, iMSeconds);
	return true;
}

bool CMTimeSpan2::GetTimeSpan(char **ppszTimeSpan)		// (+)/-DD:HH:MM:SS.mmm
{
	int iDays, iHours, iMinutes, iSeconds, iMSeconds;
	if ( !GetTimeSpan(iDays, iHours, iMinutes, iSeconds, iMSeconds) ) {
		return false;
	}

	bool bMinus;
	if ( m_llMSeconds < 0 ) {
		bMinus = true;
		iDays = abs(iDays);
		iHours = abs(iHours);
		iMinutes = abs(iMinutes);
		iSeconds = abs(iSeconds);
		iMSeconds = abs(iMSeconds);
	}
	else {
		bMinus = false;
	}

	int iStringLength = static_cast<int>(strlen("-DD:HH:MM:SS.mmm") + 1);
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

	sprintf( szBuffer, "%d", iMSeconds );
	CiStrCat(pszTimeSpanTemp, szBuffer, iStringLength, &iNTriedToCreate);

	*ppszTimeSpan = pszTimeSpanTemp;

	return true;

	// delete the *ppszTimeSpan outside
}

// frame operation
bool CMTimeSpan2::SetFrameTimeSpan(long long llFrames, float fFrameRate)
{
	m_llMSeconds = FrameToMSecond(llFrames, fFrameRate);
	return true;
}

bool CMTimeSpan2::GetFrameTimeSpan(long long& llFrames, float fFrameRate)
{
	llFrames = MSecondToFrame(m_llMSeconds, fFrameRate);
	return true;
}

bool CMTimeSpan2::SetFrameTimeSpan(int iDays, int iHours, int iMinutes, int iSeconds, int iFrames, float fFrameRate)
{
	int iMSeconds = (int)FrameToMSecond(iFrames, fFrameRate);
	return SetTimeSpan(iDays, iHours, iMinutes, iSeconds, iMSeconds);
}

bool CMTimeSpan2::GetFrameTimeSpan(int& iDays,int& iHours, int& iMinutes, int& iSeconds, int& iFrames, float fFrameRate)
{
	int iMSeconds;
	if ( !GetTimeSpan(iDays, iHours, iMinutes, iSeconds, iMSeconds) ) {
		return false;
	}
	iFrames = (int)MSecondToFrame(iMSeconds, fFrameRate);
	return true;
}

bool CMTimeSpan2::SetFrameTimeSpan(char *pszFrameTimeSpan, float fFrameRate)	// DD:HH:MM:SS.ff
{
	int iDays, iHours, iMinutes, iSeconds, iFrames;

	char szTemp[256];
	int iNTriedToCreate;
	CiStrCpy(szTemp, pszFrameTimeSpan, sizeof(szTemp), &iNTriedToCreate);
	char *ptr = szTemp;

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
	iFrames = atoi(token);

	return SetFrameTimeSpan(iDays, iHours, iMinutes, iSeconds, iFrames, fFrameRate);
}

bool CMTimeSpan2::GetFrameTimeSpan(char **ppszFrameTimeSpan, float fFrameRate)	// (+)/-DD:HH:MM:SS.ff
{
	int iDays, iHours, iMinutes, iSeconds, iFrames;
	if ( !GetFrameTimeSpan(iDays, iHours, iMinutes, iSeconds, iFrames, fFrameRate) ) {
		return false;
	}

	bool bMinus;
	if ( m_llMSeconds < 0 ) {
		bMinus = true;
		iDays = abs(iDays);
		iHours = abs(iHours);
		iMinutes = abs(iMinutes);
		iSeconds = abs(iSeconds);
		iFrames = abs(iFrames);
	}
	else {
		bMinus = false;
	}

	int iStringLength = static_cast<int>(strlen("-DD:HH:MM:SS.ff") + 1);
	char *pszFrameTimeSpanTemp = new char[iStringLength];
	int iNTriedToCreate;
	if ( bMinus ) {
		CiStrCpy(pszFrameTimeSpanTemp, "-", iStringLength, &iNTriedToCreate);
	}
	else {
		pszFrameTimeSpanTemp[0] = '\0';
	}

	char szBuffer[256];
	sprintf( szBuffer, "%d", iDays );
	CiStrCat(pszFrameTimeSpanTemp, szBuffer, iStringLength, &iNTriedToCreate);
	CiStrCat(pszFrameTimeSpanTemp, ":", iStringLength, &iNTriedToCreate);

	sprintf( szBuffer, "%d", iHours );
	CiStrCat(pszFrameTimeSpanTemp, szBuffer, iStringLength, &iNTriedToCreate);
	CiStrCat(pszFrameTimeSpanTemp, ":", iStringLength, &iNTriedToCreate);

	sprintf( szBuffer, "%d", iMinutes );
	CiStrCat(pszFrameTimeSpanTemp, szBuffer, iStringLength, &iNTriedToCreate);
	CiStrCat(pszFrameTimeSpanTemp, ":", iStringLength, &iNTriedToCreate);

	sprintf( szBuffer, "%d", iSeconds );
	CiStrCat(pszFrameTimeSpanTemp, szBuffer, iStringLength, &iNTriedToCreate);
	CiStrCat(pszFrameTimeSpanTemp, ".", iStringLength, &iNTriedToCreate);

	sprintf( szBuffer, "%d", iFrames );
	CiStrCat(pszFrameTimeSpanTemp, szBuffer, iStringLength, &iNTriedToCreate);

	*ppszFrameTimeSpan = pszFrameTimeSpanTemp;

	return true;

	// delete the *ppszFrameTimeSpan outside
}

// by sasgas 2003.8.2
char* CMTimeSpan2::GetDurationStringforDBAPI()
{
	int iDays = 0, iHours = 0, iMinutes = 0, iSeconds = 0, iMSeconds = 0;

	if ( !GetTimeSpan(iDays, iHours, iMinutes, iSeconds, iMSeconds) )
	{
		return NULL;
	}

	if ( m_llMSeconds < 0 )
	{// 발생할 수 있는 상황인가?
		iDays = abs(iDays);
		iHours = abs(iHours);
		iMinutes = abs(iMinutes);
		iSeconds = abs(iSeconds);
		iMSeconds = abs(iMSeconds);
	}

	int iStringLength = static_cast<int>(strlen("HHMMSSmmm") + 1);
	char *pszTimeSpan = new char[iStringLength];

	sprintf( pszTimeSpan, "%02d%02d%02d%03d",
											 iHours,
											 iMinutes,
											 iSeconds,
											 iMSeconds );


	return pszTimeSpan;

	// delete the *ppszTimeSpan outside
}

// by sasgas 2003.8.2
bool CMTimeSpan2::Set(const char* pszTimeSpan)
{
	char szTemp[256];
	const char *ptr = pszTimeSpan;

	int iDay = 0, iHour = 0, iMinute = 0, iSecond = 0, iMSecond = 0, iFrameCount = 0;

	if( CiStrLen("HHMMSSmm") == CiStrLen( pszTimeSpan ) )
	{
		strncpy(szTemp, ptr, 2);		//HH
		szTemp[2] = '\0';
		iHour	= atoi(szTemp);
		iDay	= iHour / 24;
		iHour	= iHour % 24;
		ptr += 2;

		strncpy(szTemp, ptr, 2);		//MM
		szTemp[2] = '\0';
		iMinute = atoi(szTemp);
		ptr += 2;

		strncpy(szTemp, ptr, 2);		//SS
		szTemp[2] = '\0';
		iSecond = atoi(szTemp);
		ptr += 2;

		strncpy(szTemp, ptr, 2);		//mmm
		szTemp[2] = '\0';
		iFrameCount = atoi(szTemp);

		iMSecond = (int)FrameToMSecond( (long long)iFrameCount, (float)29.97 );

	}else if( CiStrLen("HHMMSSmmm") == CiStrLen( pszTimeSpan ) )
	{//CiStrLen("HHMMSSmmm")

		strncpy(szTemp, ptr, 2);		//HH
		szTemp[2] = '\0';
		iHour	= atoi(szTemp);
		iDay	= iHour / 24;
		iHour	= iHour % 24;
		ptr		+= 2;

		strncpy(szTemp, ptr, 2);		//MM
		szTemp[2]	= '\0';
		iMinute		= atoi(szTemp);
		ptr			+= 2;

		strncpy(szTemp, ptr, 2);		//SS
		szTemp[2]	= '\0';
		iSecond		= atoi(szTemp);
		ptr			+= 2;

		strncpy(szTemp, ptr, 3);		//mmm
		szTemp[3]	= '\0';
		iMSecond	= atoi(szTemp);

	}else
		return false;

	return SetTimeSpan(iDay, iHour, iMinute, iSecond, iMSecond);
}

// by sasgas 2003.8.2
CMTimeSpan2::CMTimeSpan2(const char* pszTimeSpan)
{
	Set(pszTimeSpan);
}


