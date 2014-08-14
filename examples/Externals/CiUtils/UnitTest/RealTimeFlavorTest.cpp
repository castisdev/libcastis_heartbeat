#include "gtest/gtest.h"
#include "CiGlobals.h"
#include "common_CiUtils.h"

TEST(TestCRealTimeFlavor, GETSET_Period)
{
	CRealTimeFlavor rtf;
	int iPeriodMicroSec = 1000;

	rtf.SetPeriodMicroSec(iPeriodMicroSec);
	
	ASSERT_EQ(iPeriodMicroSec, rtf.GetPeriodMicroSec());
}

TEST(TestCRealTimeFlavor, PeriodStartEndTime)
{
	CRealTimeFlavor rtf;
	int iPeriodMicroSec = 1000;

	rtf.SetPeriodMicroSec(iPeriodMicroSec);
	CUTime start = rtf.GetPeriodStartTime();
	CUTime end = rtf.GetPeriodEndTime();

	CUTimeSpan diff = end - start;

	ASSERT_EQ(diff.m_usec, iPeriodMicroSec);
}

TEST(TestCRealTimeFlavor, PeriodStartWithExecutionCounter)
{
	CRealTimeFlavor rtf;
	int iPeriodMicroSec = 1000;
	rtf.SetPeriodMicroSec(iPeriodMicroSec);

	CUTime start = rtf.GetPeriodStartTime();

	rtf.m_llExecutionCounter++;
	CUTime start_plus_1_execution = rtf.GetPeriodStartTime();

	CUTimeSpan diff = start_plus_1_execution - start;

	ASSERT_EQ(diff.m_usec, iPeriodMicroSec);
}
