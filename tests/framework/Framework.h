// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Singleton.h>
#include <anki/Math.h>
#include <anki/util/Logger.h>
#include <stdexcept>
#include <vector>
#include <string>
#include <sstream>

namespace anki
{

// Forward
class TestSuite;
class Test;
class Tester;

#define ANKI_TEST_LOGI(...) ANKI_LOG("TEST", NORMAL, __VA_ARGS__)
#define ANKI_TEST_LOGE(...) ANKI_LOG("TEST", ERROR, __VA_ARGS__)
#define ANKI_TEST_LOGW(...) ANKI_LOG("TEST", WARNING, __VA_ARGS__)
#define ANKI_TEST_LOGF(...) ANKI_LOG("TEST", FATAL, __VA_ARGS__)

/// The actual test
using TestCallback = void (*)(Test&);

/// Test suite
class TestSuite
{
public:
	std::string name;
	std::vector<Test*> tests;

	~TestSuite();
};

/// Test
class Test
{
public:
	std::string name;
	TestSuite* suite = nullptr;
	TestCallback callback = nullptr;

	void run();
};

/// A container of test suites
class Tester
{
public:
	std::vector<TestSuite*> suites;
	std::string programName;

	void addTest(const char* name, const char* suite, TestCallback callback);

	int run(int argc, char** argv);

	int listTests();

	~Tester()
	{
		for(TestSuite* s : suites)
		{
			delete s;
		}
	}
};

/// Singleton so we can do the ANKI_TEST trick
extern Tester& getTesterSingleton();

/// Delete the instance to make valgrind a bit happy
extern void deleteTesterSingleton();

// Macros

/// Create a new test and add it. It does a trick to add the test by using a static function
#define ANKI_TEST(suiteName_, name_)                                                                                   \
	using namespace anki;                                                                                              \
	void test_##suiteName_##name_(Test&);                                                                              \
                                                                                                                       \
	struct Foo##suiteName_##name_                                                                                      \
	{                                                                                                                  \
		Foo##suiteName_##name_()                                                                                       \
		{                                                                                                              \
			getTesterSingleton().addTest(#name_, #suiteName_, test_##suiteName_##name_);                               \
		}                                                                                                              \
	};                                                                                                                 \
	static Foo##suiteName_##name_ yada##suiteName_##name_;                                                             \
	void test_##suiteName_##name_(Test&)

/// Intermediate macro
#define ANKI_TEST_EXPECT_EQ_IMPL(file_, line_, func_, x, y)                                                            \
	do                                                                                                                 \
	{                                                                                                                  \
		if((x) != (y))                                                                                                 \
		{                                                                                                              \
			std::stringstream ss;                                                                                      \
			ss << "FAILURE: " << #x << " != " << #y << " (" << file_ << ":" << line_ << ")";                           \
			fprintf(stderr, "%s\n", ss.str().c_str());                                                                 \
			abort();                                                                                                   \
		}                                                                                                              \
	} while(0);

/// Intermediate macro
#define ANKI_TEST_EXPECT_NEQ_IMPL(file_, line_, func_, x, y)                                                           \
	do                                                                                                                 \
	{                                                                                                                  \
		if((x) == (y))                                                                                                 \
		{                                                                                                              \
			std::stringstream ss;                                                                                      \
			ss << "FAILURE: " << #x << " == " << #y << " (" << file_ << ":" << line_ << ")";                           \
			fprintf(stderr, "%s\n", ss.str().c_str());                                                                 \
			abort();                                                                                                   \
		}                                                                                                              \
	} while(0);

/// Intermediate macro
#define ANKI_TEST_EXPECT_GT_IMPL(file_, line_, func_, x, y)                                                            \
	do                                                                                                                 \
	{                                                                                                                  \
		if((x) <= (y))                                                                                                 \
		{                                                                                                              \
			std::stringstream ss;                                                                                      \
			ss << "FAILURE: " << #x << " > " << #y << " (" << file_ << ":" << line_ << ")";                            \
			fprintf(stderr, "%s\n", ss.str().c_str());                                                                 \
			abort();                                                                                                   \
		}                                                                                                              \
	} while(0);

/// Intermediate macro
#define ANKI_TEST_EXPECT_GEQ_IMPL(file_, line_, func_, x, y)                                                           \
	do                                                                                                                 \
	{                                                                                                                  \
		if((x) < (y))                                                                                                  \
		{                                                                                                              \
			std::stringstream ss;                                                                                      \
			ss << "FAILURE: " << #x << " >= " << #y << " (" << file_ << ":" << line_ << ")";                           \
			fprintf(stderr, "%s\n", ss.str().c_str());                                                                 \
			abort();                                                                                                   \
		}                                                                                                              \
	} while(0);

/// Intermediate macro
#define ANKI_TEST_EXPECT_LT_IMPL(file_, line_, func_, x, y)                                                            \
	do                                                                                                                 \
	{                                                                                                                  \
		if((x) >= (y))                                                                                                 \
		{                                                                                                              \
			std::stringstream ss;                                                                                      \
			ss << "FAILURE: " << #x << " < " << #y << " (" << file_ << ":" << line_ << ")";                            \
			fprintf(stderr, "%s\n", ss.str().c_str());                                                                 \
			abort();                                                                                                   \
		}                                                                                                              \
	} while(0);

/// Intermediate macro
#define ANKI_TEST_EXPECT_LEQ_IMPL(file_, line_, func_, x, y)                                                           \
	do                                                                                                                 \
	{                                                                                                                  \
		if((x) > (y))                                                                                                  \
		{                                                                                                              \
			std::stringstream ss;                                                                                      \
			ss << "FAILURE: " << #x << " <= " << #y << " (" << file_ << ":" << line_ << ")";                           \
			fprintf(stderr, "%s\n", ss.str().c_str());                                                                 \
			abort();                                                                                                   \
		}                                                                                                              \
	} while(0);

/// Intermediate macro
#define ANKI_TEST_EXPECT_NEAR_IMPL(file_, line_, func_, x, y, epsilon_)                                                \
	do                                                                                                                 \
	{                                                                                                                  \
		if(absolute((x) - (y)) > (epsilon_))                                                                           \
		{                                                                                                              \
			std::stringstream ss;                                                                                      \
			ss << "FAILURE: " << #x << " != " << #y << " (" << file_ << ":" << line_ << ")";                           \
			fprintf(stderr, "%s\n", ss.str().c_str());                                                                 \
			abort();                                                                                                   \
		}                                                                                                              \
	} while(0);

/// Macro to compare equal
#define ANKI_TEST_EXPECT_EQ(x_, y_) ANKI_TEST_EXPECT_EQ_IMPL(__FILE__, __LINE__, __func__, x_, y_)

/// Macro to compare equal
#define ANKI_TEST_EXPECT_NEQ(x_, y_) ANKI_TEST_EXPECT_NEQ_IMPL(__FILE__, __LINE__, __func__, x_, y_)

/// Macro to compare greater than
#define ANKI_TEST_EXPECT_GT(x_, y_) ANKI_TEST_EXPECT_GT_IMPL(__FILE__, __LINE__, __func__, x_, y_)

/// Macro to compare greater than or equal
#define ANKI_TEST_EXPECT_GEQ(x_, y_) ANKI_TEST_EXPECT_GEQ_IMPL(__FILE__, __LINE__, __func__, x_, y_)

/// Macro to compare less than
#define ANKI_TEST_EXPECT_LT(x_, y_) ANKI_TEST_EXPECT_LT_IMPL(__FILE__, __LINE__, __func__, x_, y_)

/// Macro to compare less than or equal
#define ANKI_TEST_EXPECT_LEQ(x_, y_) ANKI_TEST_EXPECT_LEQ_IMPL(__FILE__, __LINE__, __func__, x_, y_)

/// Compare floats with epsilon
#define ANKI_TEST_EXPECT_NEAR(x_, y_, e_) ANKI_TEST_EXPECT_NEAR_IMPL(__FILE__, __LINE__, __func__, x_, y_, e_)

/// Check error code.
#define ANKI_TEST_EXPECT_NO_ERR(x_) ANKI_TEST_EXPECT_EQ_IMPL(__FILE__, __LINE__, __func__, x_, ErrorCode::NONE)

/// Check error code.
#define ANKI_TEST_EXPECT_ANY_ERR(x_) ANKI_TEST_EXPECT_NEQ_IMPL(__FILE__, __LINE__, __func__, x_, ErrorCode::NONE)

/// Check error code.
#define ANKI_TEST_EXPECT_ERR(x_, y_) ANKI_TEST_EXPECT_EQ_IMPL(__FILE__, __LINE__, __func__, x_, y_)

} // end namespace anki
