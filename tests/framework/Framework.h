#ifndef ANKI_TESTS_FRAMEWORK_FRAMEWORK_H
#define ANKI_TESTS_FRAMEWORK_FRAMEWORK_H

#include "anki/util/Vector.h"
#include "anki/util/Singleton.h"
#include "anki/util/Exception.h"
#include <string>
#include <iostream>

namespace anki {

// Forward
class TestSuite;
class Test;
class Tester;

/// The actual test
typedef void (*TestCallback)(Test&);

/// Test suite
struct TestSuite
{
	std::string name;
	PtrVector<Test> tests;
};

/// Test
struct Test
{
	std::string name;
	TestSuite* suite = nullptr;
	TestCallback callback = nullptr;

	void assertion(const char* str);
};

/// A container of test suites
struct Tester
{
	PtrVector<TestSuite> suites;
	std::string programName;

	void addTest(const char* name, const char* suite, TestCallback callback);

	int run(int argc, char** argv);

	int runAll();

	int runSome(const char* suite, const char* test);

	int listTests();
};

/// Singleton so we can do the ANKI_TEST trick
typedef Singleton<Tester> TesterSingleton;

//==============================================================================
// Macros

/// Create a new test and add it. It does a trick to add the test by using a
/// static function
#define ANKI_TEST(suiteName_, name_) \
	using namespace anki; \
	void test_##suiteName_##name_(Test&); \
	\
	struct Foo##suiteName_##name_ { \
		Foo##suiteName_##name_() { \
			TesterSingleton::get().addTest(#suiteName_, #name_, \
				test_##suiteName_##name_); \
		} \
	}; \
	static Foo##suiteName_##name_ lala; \
	void test_##suiteName_##name_(Test&)

/// XXX
#define ANKI_TEST_EXPECT_EQ_IMPL(file_, line_, func_, x, y) \
	do { \
		if((x) != (y)) { \
			std::stringstream ss; \
			ss << #x << " != " << #y << " (" << x << " != " << y; \
			throw anki::Excption(ss.str().c_str(), file_, line_, func_); \
		} \
	} while(0); 

/// XXX
#define ANKI_TEST_EXPECT_EQ(x, y) \
	ANKI_TEST_EXPECT_EQ_IMPL(__FILE__, __LINE__, __func__, x, y)

} // end namespace anki

#endif
