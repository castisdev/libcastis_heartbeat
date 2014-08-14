#include "CiHBResponserWithCiUtils.h"

CCiHBResponserWithCiUtils::CCiHBResponserWithCiUtils(const std::string& szRepresentativeIP
													 , unsigned short iPortNumber
													 , const std::string& szLocalIP
													 , int iTimeoutMillisec/*=0*/)
													 : CCiHBResponser(szRepresentativeIP, iPortNumber, szLocalIP, iTimeoutMillisec)
{
}

bool CCiHBResponserWithCiUtils::CreateThread()
{
	return CCiHBResponser::CreateThread();
}

bool CCiHBResponserWithCiUtils::InitInstance()
{
	return CCiHBResponser::InitInstance();
}

bool CCiHBResponserWithCiUtils::ExitInstance()
{
	return CCiHBResponser::ExitInstance();
}

bool CCiHBResponserWithCiUtils::Run()
{
	return CCiHBResponser::Run();
}

bool CCiHBResponserWithCiUtils::RunThreadHere()
{
	return CCiHBResponser::RunThreadHere();
}
