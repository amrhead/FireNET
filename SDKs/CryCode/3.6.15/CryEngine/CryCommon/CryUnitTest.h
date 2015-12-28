///////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   CryUnitTest.h
//  Version:     v1.00
//  Created:     19/10/2004 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: Defines namespace PathUtil for operations on files paths.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __CryUnitTest_h__
#define __CryUnitTest_h__
#pragma once

#include <ITestSystem.h>

#if defined(CRY_UNIT_TESTING_USE_EXCEPTIONS)
	#include <exception>
#endif

////////////////////////////////////////////////////////////////////////////
// supported Interface macros for Cry Unit Tests
// CRY_UNIT_TEST_SUITE:
//		- Specify a suit name to group tests locally together
//
// CRY_UNIT_TEST:
//		- Specify a new test which is automatically registered
//
// CRY_UNIT_TEST_ASSERT:
//		- Fails and reports if the specified contion evaluates to false
//
// CRY_UNIT_TEST_CHECK_CLOSE:
//		- Fails and reports if the specified floating pointer values are not equal with respect to epsilon
//
// CRY_UNIT_TEST_CHECK_EQUAL:
//		- Fails and reports if the specified values are not equal
////////////////////////////////////////////////////////////////////////////
namespace CryUnitTest
{
	struct Test;
	struct IUnitTestReporter;

	enum TReporterToUse
	{
		ExcelReporter,
		MinimalReporter,
		RegularReporter
	};

	struct UnitTestInfo
	{
		char const* module;
		char const* suite;
		char const* name;
		char const* filename;
		int lineNumber;
		Test* pTestImpl;  // Pointer to the actual test class implementation.
    string sFilename; // storage to keep Lua test file for test reporting

		UnitTestInfo() : module(""),suite(""),name(""),filename(""),lineNumber(0),pTestImpl(0) {};
	};

  struct AutoTestInfo
  {
    bool runNextTest; // to organize auto tests cycle
    int waitMSec; // identify waiting period after each auto test
    const char* szTaskName; // current test task

    AutoTestInfo() : runNextTest(true), waitMSec(0), szTaskName(0) {};
  };

	// Base class for all user tests.
	struct Test
	{
		virtual ~Test(){}
		// Must be implemented by the test creator.
		virtual void Run() = 0;

		// optional methods called by the system at the begining of the testing and at the end of the testing.
		virtual void Init() {};
		virtual void Done() {};

		UnitTestInfo m_unitTestInfo;
    AutoTestInfo m_autoTestInfo;
		Test* m_pNext;
		static Test *m_pFirst;
		static Test *m_pLast;
	};

	struct UnitTestRunContext
	{
		int testCount;
		int failedTestCount;
		int succedTestCount;
		IUnitTestReporter *pReporter;

		UnitTestRunContext() : testCount(0),failedTestCount(0),succedTestCount(0),pReporter(0) {};
	};

	struct IUnitTest
	{
		virtual ~IUnitTest(){}
		virtual void GetInfo( UnitTestInfo &info ) = 0;
    virtual void GetAutoTestInfo(AutoTestInfo& info) = 0;
		virtual void Run( UnitTestRunContext &context ) = 0;
		virtual void Init() = 0;
		virtual void Done() = 0;
	};

	struct IUnitTestReporter
	{
		virtual ~IUnitTestReporter(){}
		virtual void OnStartTesting( UnitTestRunContext &context ) = 0;
		virtual void OnFinishTesting( UnitTestRunContext &context ) = 0;

		virtual void OnTestStart( IUnitTest *pTest ) = 0;
		virtual void OnTestFinish( IUnitTest *pTest,float fRunTimeInMs,bool bSuccess,char const* failureDescription ) = 0;
	};

	struct IUnitTestManager
	{
		virtual ~IUnitTestManager(){}
		virtual IUnitTest* CreateTest( const UnitTestInfo &info ) = 0;
		virtual int RunAllTests( CryUnitTest::TReporterToUse ) = 0;
		virtual void RunMatchingTests( const char *sName,UnitTestRunContext &context ) = 0;
    virtual void RunAutoTests(const char* sSuiteName, const char* sTestName) = 0;
    virtual void Update() = 0;
    virtual void RemoveTests() = 0;

		// 'callback' for failed tests, to prevent storing information in the exception, allowing use of setjmp/longjmp
		virtual void SetExceptionCause( const char *expression, const char *file,  int line ) =0;
	};

#if defined(CRY_UNIT_TESTING_USE_EXCEPTIONS)
	// Helper classes.
	class assert_exception
		: public std::exception
	{
	public:
		virtual ~assert_exception() throw(){}
		char m_description[256];
		char m_filename[256];
		int  m_lineNumber;
	public:
		assert_exception(char const* description, char const* filename, int lineNumber)
		{
			strcpy_s(m_description,description);
			strcpy_s(m_filename,filename);
			m_lineNumber = lineNumber;
		}
		virtual char const* what() const throw() { return m_description; };
	};
#endif

	//////////////////////////////////////////////////////////////////////////
	class CAutoRegisterUnitTest
	{
	public:
		CAutoRegisterUnitTest( Test *pTest,const char *suite,const char *name,const char *filename,int line )
		{
			if (!pTest->m_pLast)
				pTest->m_pFirst = pTest;
			else
				pTest->m_pLast->m_pNext = pTest;
			pTest->m_pLast = pTest;

			pTest->m_unitTestInfo.module = "";
			pTest->m_unitTestInfo.suite = suite;
			pTest->m_unitTestInfo.name = name;
			pTest->m_unitTestInfo.filename = filename;
			pTest->m_unitTestInfo.lineNumber = line;
			pTest->m_unitTestInfo.pTestImpl = pTest;
		}
	};
};

// Global Suite for all tests that do not specify suite.
namespace CryUnitTestSuite
{
	inline const char* GetSuiteName() { return ""; }
};

#define CRY_UNIT_TEST_NAME(ClassName,TestName) \
class ClassName : public CryUnitTest::Test \
{ \
	virtual void Run();\
} auto_unittest_instance_##ClassName; \
	CryUnitTest::CAutoRegisterUnitTest autoreg_unittest_##ClassName( &auto_unittest_instance_##ClassName,CryUnitTestSuite::GetSuiteName(),TestName,__FILE__,__LINE__ ); \
	void ClassName::Run()

#define CRY_UNIT_TEST(ClassName) CRY_UNIT_TEST_NAME(ClassName,#ClassName)

#define CRY_UNIT_TEST_REGISTER(ClassName) \
	ClassName auto_unittest_instance_##ClassName; \
	CryUnitTest::CAutoRegisterUnitTest autoreg_unittest_##ClassName( &auto_unittest_instance_##ClassName,CryUnitTestSuite::GetSuiteName(),#ClassName,__FILE__,__LINE__ );

#define CRY_UNIT_TEST_REGISTER_NAME(ClassName,TestName) \
	ClassName auto_unittest_instance_##ClassName; \
	CryUnitTest::CAutoRegisterUnitTest autoreg_unittest_##ClassName( &auto_unittest_instance_##ClassName,CryUnitTestSuite::GetSuiteName(),TestName,__FILE__,__LINE__ );


#define CRY_UNIT_TEST_SUITE(SuiteName)                                 \
	namespace SuiteName {                                              \
	namespace CryUnitTestSuite {                                   \
	inline char const* GetSuiteName () { return #SuiteName; }  \
}                                                              \
}                                                                  \
	namespace SuiteName


#define CRY_UNIT_TEST_ASSERT(condition)							                \
	do																																\
	{																																	\
		if(!(condition))														                    \
		{																																\
			gEnv->pSystem->GetITestSystem()->GetIUnitTestManager()->SetExceptionCause(#condition,__FILE__,__LINE__);\
		}																																\
} while(0)


#define CRY_UNIT_TEST_CHECK_CLOSE(valueA, valueB, epsilon)					\
	do																																\
	{																																	\
		if(!(fcmp(valueA,valueB,epsilon)))													    \
		{																																\
			gEnv->pSystem->GetITestSystem()->GetIUnitTestManager()->SetExceptionCause(#valueA " != " #valueB,__FILE__,__LINE__);\
		}																																\
} while(0)


#define CRY_UNIT_TEST_CHECK_EQUAL(valueA, valueB)							           \
	do																																\
	{																																	\
		if(!(valueA == valueB))														    \
		{																																\
			gEnv->pSystem->GetITestSystem()->GetIUnitTestManager()->SetExceptionCause(#valueA " != " #valueB,__FILE__,__LINE__);\
		}																																\
} while(0)


#endif //__CryUnitTest_h__
