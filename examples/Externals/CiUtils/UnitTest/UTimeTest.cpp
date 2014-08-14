#include "gtest/gtest.h"
#include "CiGlobals.h"
#include "common_CiUtils.h"

TEST(TestCUTime, GETSET_longlong)
{
	CUTime lhs;
	long long lhs_microseconds;
	lhs.GetTime(lhs_microseconds);

	CUTime rhs;
	rhs.SetTime(lhs_microseconds);
	long long rhs_microseconds;
	rhs.GetTime(rhs_microseconds);

	ASSERT_EQ(lhs_microseconds, rhs_microseconds);
}

TEST(TestCUTime, GETSET_char)
{
	char* lhs = "2013:05:10:15:45:01.123456";
	CUTime time;
	time.SetTime(lhs);

	char* rhs = NULL;
	time.GetTime(&rhs);
	
	EXPECT_STREQ(lhs, rhs);

	if (rhs) delete [] rhs;
}

TEST(TestCUTime, OPERATION_PLUS_longlong)
{
	long long rhs = 100;
	CUTime lhs;
	CUTime result = lhs + rhs;

	long long lllhs;
	lhs.GetTime(lllhs);
	long long llresult;
	result.GetTime(llresult);

	ASSERT_EQ(rhs, llresult - lllhs);
}

TEST(TestCUTime, OPERATION_PLUS_CUTimeSpan)
{
	long long rhs = 100;
	CUTimeSpan rhsTimeSpan;
	rhsTimeSpan.SetTimeSpan(rhs);
	
	CUTime lhs;
	CUTime result = lhs + rhsTimeSpan;

	long long lllhs;
	lhs.GetTime(lllhs);
	long long llresult;
	result.GetTime(llresult);

	ASSERT_EQ(rhs, llresult - lllhs);
}

TEST(TestCUTime, OPERATION_MINUS_longlong)
{
	long long rhs = 100;
	CUTime lhs;
	CUTime result = lhs - rhs;

	long long lllhs;
	lhs.GetTime(lllhs);
	long long llresult;
	result.GetTime(llresult);

	ASSERT_EQ(rhs, lllhs - llresult);
}

TEST(TestCUTime, OPERATION_MINUS_CUTimeSpan)
{
	long long rhs = 100;
	CUTimeSpan rhsTimeSpan;
	rhsTimeSpan.SetTimeSpan(rhs);

	CUTime lhs;
	CUTime result = lhs - rhsTimeSpan;

	long long lllhs;
	lhs.GetTime(lllhs);
	long long llresult;
	result.GetTime(llresult);

	ASSERT_EQ(rhs, lllhs - llresult);
}

TEST(TestCUTime, OPERATION_MINUS_CUTime)
{
	long long diff = 100;
	long long lllhs;
	CUTime lhs;
	lhs.GetTime(lllhs);
	CUTime rhs;
	rhs.SetTime(lllhs-diff);
	long long llrhs;
	rhs.GetTime(llrhs);
	CUTimeSpan result = lhs - rhs;

	long long diff_result;
	result.GetTimeSpan(diff_result);

	ASSERT_EQ(diff_result, diff);
}

TEST(TestCUTime, OPERATION_EQ)
{
	CUTime lhs;
	CUTime rhs_eq;
	CUTime rhs_plus_1microsec;
	CUTime rhs_plus_1sec;

	lhs.SetTime("2013:01:01:01:01:01.000001");
	rhs_eq.SetTime("2013:01:01:01:01:01.000001");
	rhs_plus_1microsec.SetTime("2013:01:01:01:01:01.000002");
	rhs_plus_1sec.SetTime("2013:01:01:01:01:02.000001");

	EXPECT_TRUE(lhs==rhs_eq);
	EXPECT_FALSE(lhs==rhs_plus_1microsec);
	EXPECT_FALSE(lhs==rhs_plus_1sec);
}

TEST(TestCUTime, OPERATION_BT)
{
	CUTime lhs;
	CUTime rhs_eq;
	CUTime rhs_plus_1microsec;
	CUTime rhs_plus_1sec;
	CUTime rhs_minus_1microsec;
	CUTime rhs_minus_1sec;

	lhs.SetTime("2013:01:01:01:01:01.000001");
	rhs_eq.SetTime("2013:01:01:01:01:01.000001");
	rhs_plus_1microsec.SetTime("2013:01:01:01:01:01.000002");
	rhs_plus_1sec.SetTime("2013:01:01:01:01:02.000001");
	rhs_minus_1microsec.SetTime("2013:01:01:01:01:01.000000");
	rhs_minus_1sec.SetTime("2013:01:01:01:01:00.000001");

	EXPECT_FALSE(lhs>rhs_eq);
	EXPECT_FALSE(lhs>rhs_plus_1microsec);
	EXPECT_FALSE(lhs>rhs_plus_1sec);
	EXPECT_TRUE(lhs>rhs_minus_1microsec);
	EXPECT_TRUE(lhs>rhs_minus_1sec);
}

TEST(TestCUTime, OPERATION_LT)
{
	CUTime lhs;
	CUTime rhs_eq;
	CUTime rhs_plus_1microsec;
	CUTime rhs_plus_1sec;
	CUTime rhs_minus_1microsec;
	CUTime rhs_minus_1sec;

	lhs.SetTime("2013:01:01:01:01:01.000001");
	rhs_eq.SetTime("2013:01:01:01:01:01.000001");
	rhs_plus_1microsec.SetTime("2013:01:01:01:01:01.000002");
	rhs_plus_1sec.SetTime("2013:01:01:01:01:02.000001");
	rhs_minus_1microsec.SetTime("2013:01:01:01:01:01.000000");
	rhs_minus_1sec.SetTime("2013:01:01:01:01:00.000001");

	EXPECT_FALSE(lhs<rhs_eq);
	EXPECT_TRUE(lhs<rhs_plus_1microsec);
	EXPECT_TRUE(lhs<rhs_plus_1sec);
	EXPECT_FALSE(lhs<rhs_minus_1microsec);
	EXPECT_FALSE(lhs<rhs_minus_1sec);
}


TEST(TestCUTime, OPERATION_BE)
{
	CUTime lhs;
	CUTime rhs_eq;
	CUTime rhs_plus_1microsec;
	CUTime rhs_plus_1sec;
	CUTime rhs_minus_1microsec;
	CUTime rhs_minus_1sec;

	lhs.SetTime("2013:01:01:01:01:01.000001");
	rhs_eq.SetTime("2013:01:01:01:01:01.000001");
	rhs_plus_1microsec.SetTime("2013:01:01:01:01:01.000002");
	rhs_plus_1sec.SetTime("2013:01:01:01:01:02.000001");
	rhs_minus_1microsec.SetTime("2013:01:01:01:01:01.000000");
	rhs_minus_1sec.SetTime("2013:01:01:01:01:00.000001");

	EXPECT_TRUE(lhs>=rhs_eq);
	EXPECT_FALSE(lhs>=rhs_plus_1microsec);
	EXPECT_FALSE(lhs>=rhs_plus_1sec);
	EXPECT_TRUE(lhs>=rhs_minus_1microsec);
	EXPECT_TRUE(lhs>=rhs_minus_1sec);
}

TEST(TestCUTime, OPERATION_LE)
{
	CUTime lhs;
	CUTime rhs_eq;
	CUTime rhs_plus_1microsec;
	CUTime rhs_plus_1sec;
	CUTime rhs_minus_1microsec;
	CUTime rhs_minus_1sec;

	lhs.SetTime("2013:01:01:01:01:01.000001");
	rhs_eq.SetTime("2013:01:01:01:01:01.000001");
	rhs_plus_1microsec.SetTime("2013:01:01:01:01:01.000002");
	rhs_plus_1sec.SetTime("2013:01:01:01:01:02.000001");
	rhs_minus_1microsec.SetTime("2013:01:01:01:01:01.000000");
	rhs_minus_1sec.SetTime("2013:01:01:01:01:00.000001");

	EXPECT_TRUE(lhs<=rhs_eq);
	EXPECT_TRUE(lhs<=rhs_plus_1microsec);
	EXPECT_TRUE(lhs<=rhs_plus_1sec);
	EXPECT_FALSE(lhs<=rhs_minus_1microsec);
	EXPECT_FALSE(lhs<=rhs_minus_1sec);
}
