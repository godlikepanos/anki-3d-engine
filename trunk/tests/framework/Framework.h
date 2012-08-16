#ifndef ANKI_TESTS_FRAMEWORK_FRAMEWORK_H
#define ANKI_TESTS_FRAMEWORK_FRAMEWORK_H

#include "anki/util/Vector.h"
#include "anki/util/Singleton.h"
#include <stdexcept>
#include <string>
#include <iostream>
#include <sstream>
#include <cmath>

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

	void run();
};

/// A container of test suites
struct Tester
{
	PtrVector<TestSuite> suites;
	std::string programName;

	void addTest(const char* name, const char* suite, TestCallback callback);

	int run(int argc, char** argv);

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
			TesterSingleton::get().addTest(#name_, #suiteName_, \
				test_##suiteName_##name_); \
		} \
	}; \
	static Foo##suiteName_##name_ yada##suiteName_##name_; \
	void test_##suiteName_##name_(Test&)

/// Intermediate macro
#define ANKI_TEST_EXPECT_EQ_IMPL(file_, line_, func_, x, y) \
	do { \
		if((x) != (y)) { \
			std::stringstream ss; \
			ss << #x << " != " << #y << " [" << x << " != " << y << "] (" \
				<< file_ << ":" << line_ << ")"; \
			throw std::runtime_error(ss.str()); \
		} \
	} while(0);

/// Intermediate macro
#define ANKI_TEST_EXPECT_NEQ_IMPL(file_, line_, func_, x, y) \
	do { \
		if((x) == (y)) { \
			std::stringstream ss; \
			ss << #x << " == " << #y << " [" << x << " == " << y << "] (" \
				<< file_ << ":" << line_ << ")"; \
			throw std::runtime_error(ss.str()); \
		} \
	} while(0);

/// Intermediate macro
#define ANKI_TEST_EXPECT_NEAR_IMPL(file_, line_, func_, x, y, epsilon_) \
	do { \
		if(fabs((x) - (y)) > (epsilon_)) { \
			std::stringstream ss; \
			ss << #x << " != " << #y << " [" << x << " - " << y << " > " \
				<< epsilon_ << "] (" \
				<< file_ << ":" << line_ << ")"; \
			throw std::runtime_error(ss.str()); \
		} \
	} while(0);

/// Macro to compare equal
#define ANKI_TEST_EXPECT_EQ(x_, y_) \
	ANKI_TEST_EXPECT_EQ_IMPL(__FILE__, __LINE__, __func__, x_, y_)

/// Macro to compare equal
#define ANKI_TEST_EXPECT_NEQ(x_, y_) \
	ANKI_TEST_EXPECT_NEQ_IMPL(__FILE__, __LINE__, __func__, x_, y_)

/// XXX
#define ANKI_TEST_EXPECT_NEAR(x_, y_, e_) \
	ANKI_TEST_EXPECT_NEAR_IMPL(__FILE__, __LINE__, __func__, x_, y_, e_)

} // end namespace anki

#endif
