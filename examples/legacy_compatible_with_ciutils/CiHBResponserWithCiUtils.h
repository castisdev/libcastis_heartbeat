#pragma once
#include "CiHBResponser.h"
#include "common_CiUtils.h"

class CCiHBResponserWithCiUtils	: public CCiHBResponser, public CCiThread2
{
public:
	CCiHBResponserWithCiUtils(const std::string& szRepresentativeIP,
		unsigned short iPortNumber,
		const std::string& szLocalIP,
		int iTimeoutMillisec = 0);

	bool CreateThread();
	bool InitInstance();
	bool ExitInstance();
	bool Run();
	bool RunThreadHere();
};
