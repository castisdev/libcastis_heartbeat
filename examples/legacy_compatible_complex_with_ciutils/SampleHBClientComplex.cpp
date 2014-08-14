#include "SampleHBClientComplex.h"

SampleHBClientComplex::SampleHBClientComplex(const std::string& szRepresentativeIP
													 , unsigned short iPortNumber
													 , const std::string& szLocalIP
													 , int iPeriodMillieSec/*=50*/)
{
	m_pHBResponser.reset(new CCiHBResponserWithCiUtils(szRepresentativeIP, iPortNumber, szLocalIP));
	m_pHBResponser->PrepareComplexing();
	/* add it to a complex */
	AddCiThread(m_pHBResponser.get());

	SetPeriodMillieSec(iPeriodMillieSec);
}

SampleHBClientComplex::~SampleHBClientComplex()
{
	m_pHBResponser->EndComplexing();
	DeleteCiThread(m_pHBResponser.get());
}
