#pragma once
#include <boost/scoped_ptr.hpp>
#include "CiHBResponserWithCiUtils.h"
#include "common_CiUtils.h"

class CCiHBResponserWithCiUtils;

class SampleHBClientComplex	: public CCiThreadRealTimeComplex
{
public:
	SampleHBClientComplex(const std::string& szRepresentativeIP,
		unsigned short iPortNumber,
		const std::string& szLocalIP,
		int iPeriodMillieSec = 50);
	~SampleHBClientComplex();

	boost::scoped_ptr<CCiHBResponserWithCiUtils> m_pHBResponser;
};
