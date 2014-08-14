#ifndef _PERFMON_H
#define _PERFMON_H

#define MAX_RAW_VALUES 20

//// DEFINES FOR COUNTER NAMES ////
#define CNTR_CPU "\\Processor(_Total)\\% Processor Time" // % of cpu in use
#define CNTR_CPU0 "\\Processor(0)\\% Processor Time" // % of cpu in use
#define CNTR_CPU1 "\\Processor(1)\\% Processor Time" // % of cpu in use
#define CNTR_CPU2 "\\Processor(2)\\% Processor Time" // % of cpu in use
#define CNTR_CPU3 "\\Processor(3)\\% Processor Time" // % of cpu in use
#define CNTR_CPU4 "\\Processor(4)\\% Processor Time" // % of cpu in use
#define CNTR_CPU5 "\\Processor(5)\\% Processor Time" // % of cpu in use
#define CNTR_CPU6 "\\Processor(6)\\% Processor Time" // % of cpu in use

#define CNTR_MEMINUSE_BYTES "\\Memory\\Committed Bytes" // mem in use measured in bytes
#define CNTR_MEMAVAIL_BYTES "\\Memory\\Available Bytes" // mem available measured in bytes
#define CNTR_MEMAVAIL_KB "\\Memory\\Available KBytes" // mem avail in kilobytes
#define CNTR_MEMAVAIL_MB "\\Memory\\Available MBytes" // mem avail in megabytes
#define CNTR_MEMINUSE_PERCENT "\\Memory\\% Committed Bytes In Use" // % of mem in use
#define CNTR_MEMLIMIT_BYTES "\\Memory\\Commit Limit" // commit limit on memory in bytes

typedef struct _tag_PDHCounterStruct {
	int nIndex;				// The index of this counter, returned by AddCounter()
	LONG lValue;			// The current value of this counter
    HCOUNTER hCounter;      // Handle to the counter - given to use by PDH Library
    int nNextIndex;         // element to get the next raw value
    int nOldestIndex;       // element containing the oldes raw value
    int nRawCount;          // number of elements containing raw values
    PDH_RAW_COUNTER a_RawValue[MAX_RAW_VALUES]; // Ring buffer to contain raw values
} PDHCOUNTERSTRUCT, *PPDHCOUNTERSTRUCT;

class CPerfMon
{
public:
	bool GetHDDInfo(unsigned long *total, unsigned long *usage, unsigned long *free, _TCHAR *Drive);
	CPerfMon();
	~CPerfMon();

	//// SETUP ////
	BOOL Initialize(void);
	void Uninitialize(void);

	//// COUNTERS ////
	int AddCounter(const _TCHAR *pszCounterName);
	BOOL RemoveCounter(int nIndex);

	//// DATA ////
	BOOL CollectQueryData(void);
	BOOL GetStatistics(long *nMin, long *nMax, long *nMean, int nIndex);
	long GetCounterValue(int nIndex);

protected:
	//// COUNTERS ////
	PPDHCOUNTERSTRUCT GetCounterStruct(int nIndex);

	//// VALUES ////
	BOOL UpdateValue(PPDHCOUNTERSTRUCT pCounter);
	BOOL UpdateRawValue(PPDHCOUNTERSTRUCT pCounter);

	//// VARIABLES ////
	HQUERY m_hQuery; // the query to the PDH
	CArray<PPDHCOUNTERSTRUCT, PPDHCOUNTERSTRUCT> m_aCounters; // the current counters
	int m_nNextIndex;
};

#endif
