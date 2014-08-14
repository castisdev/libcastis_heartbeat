// PerfMon.h: interface for the CPerfMon class.
// Compatibility:
//     Windows 98, Windows NT 4.0 SP 3 (Dlls required), Windows 2000
//
// Development Environ:
//     Visual C++ 6.0
//
// Libraries / DLLs:
//     pdh.lib (linked in)
//     pdh.dll (provided with Windows 2000, must copy in for NT 4.0)
//
//////////////////////////////////////////////////////////////////////
#include "internal_CiUtils.h"	/* CiUtils.h includes CiGlobals.h hence windows.h */
#include <afxtempl.h>
#include <pdh.h>
#include <pdhmsg.h>
#include "perfmon.h"
#include <tlhelp32.h>
#include <nb30.h>

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

CPerfMon::CPerfMon()
{
	// m_nNextIndex is a unique value.  It will not be decremented, even if you remove counters.
	m_nNextIndex = 0;
}

CPerfMon::~CPerfMon()
{
}


// Function name	: CPerfMon::Initialize
// Description	    : Initialize the query and memory
// Return type		: BOOL ; true on success; false on fail
//
// R E V I S I O N S:
// DATE       PROGRAMMER      CHANGES
//
BOOL CPerfMon::Initialize()
{
	if (PdhOpenQuery(NULL, 1, &m_hQuery) != ERROR_SUCCESS)
		return false;

	return true;
}


// Function name	: CPerfMon::Uninitialize
// Description	    : Closes the query and fress all memory
// Return type		: void
//
// R E V I S I O N S:
// DATE       PROGRAMMER      CHANGES
//
void CPerfMon::Uninitialize()
{
	PdhCloseQuery(m_hQuery);

	// clean memory
	for (int i=0;i<m_aCounters.GetSize();i++)
	{
		delete m_aCounters.GetAt(i);
	}
}


// Function name	: CPerfMon::AddCounter
// Description	    : Adds a counter to the query.
// Return type		: int ; -1 on fail, index to counter on success.
// Argument         : const char *pszCounterName
//
// R E V I S I O N S:
// DATE       PROGRAMMER      CHANGES
//
int CPerfMon::AddCounter(const _TCHAR *pszCounterName)
{
	PPDHCOUNTERSTRUCT pCounter;
	pCounter = new PDHCOUNTERSTRUCT;
	if (pCounter == NULL ) return -1;

	// add to current query
	if (PdhAddCounter(m_hQuery, pszCounterName, (DWORD_PTR)pCounter, &(pCounter->hCounter)) != ERROR_SUCCESS)
	{
		delete pCounter; // clean mem
		return -1;
	}

	// insert counter into array(s)
	pCounter->nIndex = m_nNextIndex++;
	pCounter->lValue = 0;
	pCounter->nNextIndex = 0;
	pCounter->nOldestIndex = 0;
	pCounter->nRawCount = 0;
	m_aCounters.Add(pCounter);

	return pCounter->nIndex;
}


// Function name	: CPerfMon::RemoveCounter
// Description	    : remove a counter from the query based on the index returned from AddCounter
// Return type		: BOOL ; false on fail ; true on success
// Argument         : int nIndex
//
// R E V I S I O N S:
// DATE       PROGRAMMER      CHANGES
//
BOOL CPerfMon::RemoveCounter(int nIndex)
{
	PPDHCOUNTERSTRUCT pCounter = GetCounterStruct(nIndex);
	if ( pCounter == NULL ) return false;

	if (PdhRemoveCounter(pCounter->hCounter) != ERROR_SUCCESS)
		return false;

	return true;
}


// Function name	: CPerfMon::CollectQueryData
// Description	    : Collects the data for all the counters added with AddCounter()
// Return type		: BOOL ; false fail ; true success
//
// R E V I S I O N S:
// DATE       PROGRAMMER      CHANGES
//
BOOL CPerfMon::CollectQueryData()
{
	if (PdhCollectQueryData(m_hQuery) != ERROR_SUCCESS) return false;

	return true;
}


// Function name	: CPerfMon::UpdateValue
// Description	    : Updates the counter value for the counter in pCounter
// Return type		: BOOL ; false fail ; true success
// Argument         : PPDHCOUNTERSTRUCT pCounter
//
// R E V I S I O N S:
// DATE       PROGRAMMER      CHANGES
//
BOOL CPerfMon::UpdateValue(PPDHCOUNTERSTRUCT pCounter)
{
	PDH_FMT_COUNTERVALUE pdhFormattedValue;

	// get the value from the PDH
	if (PdhGetFormattedCounterValue(pCounter->hCounter, PDH_FMT_LONG, NULL, &pdhFormattedValue) != ERROR_SUCCESS)
		return false;

	// test the value for validity
	if (pdhFormattedValue.CStatus != ERROR_SUCCESS)
		return false;

	// set value
	pCounter->lValue = pdhFormattedValue.longValue;

	return true;
}


// Function name	: CPerfMon::UpdateRawValue
// Description	    : Update the raw values for the counter in pCounter
// Return type		: BOOL ; false fail ; true success
// Argument         : PPDHCOUNTERSTRUCT pCounter
//
// R E V I S I O N S:
// DATE       PROGRAMMER      CHANGES
//
BOOL CPerfMon::UpdateRawValue(PPDHCOUNTERSTRUCT pCounter)
{
    PPDH_RAW_COUNTER ppdhRawCounter;

    // Assign the next value into the array
    ppdhRawCounter = &(pCounter->a_RawValue[pCounter->nNextIndex]);

	if (PdhGetRawCounterValue(pCounter->hCounter, NULL, ppdhRawCounter) != ERROR_SUCCESS)
		return false;

    // update raw counter - up to MAX_RAW_VALUES
    pCounter->nRawCount = min(pCounter->nRawCount + 1, MAX_RAW_VALUES);

    // Update next index - rolls back to zero upon reaching MAX_RAW_VALUES
    pCounter->nNextIndex = (pCounter->nNextIndex + 1) % MAX_RAW_VALUES;

    // The Oldest index remains zero until the array is filled.
    // It will now be the same as the 'next' index since it was previously assigned.
    if (pCounter->nRawCount >= MAX_RAW_VALUES)
        pCounter->nOldestIndex = pCounter->nNextIndex;

	return true;
}

BOOL CPerfMon::GetStatistics(long *nMin, long *nMax, long *nMean, int nIndex)
{
	PDH_STATISTICS pdhStats;
	PPDHCOUNTERSTRUCT pCounter = GetCounterStruct(nIndex);
	if ( pCounter == NULL ) return false;

	if (PdhComputeCounterStatistics(pCounter->hCounter, PDH_FMT_LONG, pCounter->nOldestIndex, pCounter->nRawCount, pCounter->a_RawValue, &pdhStats) != ERROR_SUCCESS)
		return false;

	// set values
	if (pdhStats.min.CStatus != ERROR_SUCCESS)
		*nMin = 0;
	else
		*nMin = pdhStats.min.longValue;

	if (pdhStats.max.CStatus != ERROR_SUCCESS)
		*nMax = 0;
	else
		*nMax = pdhStats.max.longValue;

	if (pdhStats.mean.CStatus != ERROR_SUCCESS)
		*nMean = 0;
	else
		*nMean = pdhStats.mean.longValue;

	return true;
}


// Function name	: CPerfMon::GetCounterValue
// Description	    : return the value of the counter
// Return type		: long ; -999 on failed ; value on success
// Argument         : int nIndex
//
// R E V I S I O N S:
// DATE       PROGRAMMER      CHANGES
//
long CPerfMon::GetCounterValue(int nIndex)
{
	PPDHCOUNTERSTRUCT pCounter = GetCounterStruct(nIndex);
	if ( pCounter == NULL ) return -999L;

	// update the value(s)
	if (!UpdateValue(pCounter)) return -999L;
	if (!UpdateRawValue(pCounter)) return -999L;

	// return the value
	return pCounter->lValue;
}


// Function name	: CPerfMon::GetCounterStruct
// Description	    : Lookup a counterstruct based on the index
// Return type		: PPDHCOUNTERSTRUCT ; null on failed ; pointer to counter struct on success
// Argument         : int nIndex
//
// R E V I S I O N S:
// DATE       PROGRAMMER      CHANGES
//
PPDHCOUNTERSTRUCT CPerfMon::GetCounterStruct(int nIndex)
{
	for (int i=0;i<m_aCounters.GetSize();i++)
	{
		if (m_aCounters.GetAt(i)->nIndex == nIndex)
			return m_aCounters.GetAt(i);
	}

	return NULL;
}

bool CPerfMon::GetHDDInfo(unsigned long *total, unsigned long *usage, unsigned long *free, _TCHAR *Drive)
{
    unsigned int nDiskType;
	double t, f, u;

    ULARGE_INTEGER m_lFreeBytesAvailableToCaller;
    ULARGE_INTEGER m_lTotalNumberOfBytes;
    ULARGE_INTEGER m_lTotalNumberOfFreeBytes;

    nDiskType = GetDriveType(Drive);

    if (nDiskType == DRIVE_FIXED)   // HDD
    {
        if(!GetDiskFreeSpaceEx( Drive,
                        &m_lFreeBytesAvailableToCaller,
                        &m_lTotalNumberOfBytes,
                        &m_lTotalNumberOfFreeBytes ))
			return false;

        *total = (unsigned long)m_lTotalNumberOfBytes.QuadPart/1024;
        *free = (unsigned long)m_lTotalNumberOfFreeBytes.QuadPart/1024;

		//�Ǽ� ó�� ����� �ֱ� ����
		t = *total;
		f = *free;

		u = t - f;
		*usage = static_cast<unsigned long>(u / t * 100);
    }
	else 	return false;

	return true;
}

