// CiThreadRealTimeComplex.cpp: implementation of the CCiThreadRealTimeComplex class.
//
//////////////////////////////////////////////////////////////////////

#include "internal_CiUtils.h"
#include "CiThreadRealTimeComplex.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCiThreadRealTimeComplex::CCiThreadRealTimeComplex()
{

}

CCiThreadRealTimeComplex::~CCiThreadRealTimeComplex()
{

}

bool CCiThreadRealTimeComplex::Run()
{
	/*                  */
	/* real time flavor */
	/*                  */

	/* in case of the type limit */
	if ( m_llExecutionCounter == 0x7FFFFFFFFFFFFFFFLL ) {
		SetBaseTime();
	}

	m_llExecutionCounter++;

	long long llUSeconds = GetPaceInMicroSec();
	/* A plus value means that the pace is faster than expected */

	if ( llUSeconds < 0 ) llUSeconds = 0;

	castis::microsleep(static_cast<unsigned long>(llUSeconds));

	return CCiThreadComplex::Run();
}
