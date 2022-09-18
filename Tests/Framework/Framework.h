// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/Singleton.h>
#include <AnKi/Math.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Core.h>
#include <AnKi/Gr.h>
#include <AnKi/Resource.h>
#include <AnKi/Physics.h>
#include <stdexcept>
#include <vector>
#include <string>
#include <sstream>

namespace anki {

// Forward
class TestSuite;
class Test;
class Tester;

#define ANKI_TEST_LOGI(...) ANKI_LOG("TEST", kNormal, __VA_ARGS__)
#define ANKI_TEST_LOGE(...) ANKI_LOG("TEST", kError, __VA_ARGS__)
#define ANKI_TEST_LOGW(...) ANKI_LOG("TEST", kWarning, __VA_ARGS__)
#define ANKI_TEST_LOGF(...) ANKI_LOG("TEST", kFatal, __VA_ARGS__)

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
#define ANKI_TEST(suiteName_, name_) \
	using namespace anki; \
	void test_##suiteName_##name_(Test&); \
	struct Foo##suiteName_##name_ \
	{ \
		Foo##suiteName_##name_() \
		{ \
			getTesterSingleton().addTest(#name_, #suiteName_, test_##suiteName_##name_); \
		} \
	}; \
	static Foo##suiteName_##name_ yada##suiteName_##name_; \
	void test_##suiteName_##name_(Test&)

/// Intermediate macro
#define ANKI_TEST_EXPECT_EQ_IMPL(file_, line_, func_, x, y) \
	do \
	{ \
		if((x) != (y)) \
		{ \
			std::stringstream ss; \
			ss << "FAILURE: " << #x << " != " << #y << " (" << file_ << ":" << line_ << ")"; \
			fprintf(stderr, "%s\n", ss.str().c_str()); \
			ANKI_DEBUG_BREAK(); \
		} \
	} while(0);

/// Intermediate macro
#define ANKI_TEST_EXPECT_NEQ_IMPL(file_, line_, func_, x, y) \
	do \
	{ \
		if((x) == (y)) \
		{ \
			std::stringstream ss; \
			ss << "FAILURE: " << #x << " == " << #y << " (" << file_ << ":" << line_ << ")"; \
			fprintf(stderr, "%s\n", ss.str().c_str()); \
			ANKI_DEBUG_BREAK(); \
		} \
	} while(0);

/// Intermediate macro
#define ANKI_TEST_EXPECT_GT_IMPL(file_, line_, func_, x, y) \
	do \
	{ \
		if((x) <= (y)) \
		{ \
			std::stringstream ss; \
			ss << "FAILURE: " << #x << " > " << #y << " (" << file_ << ":" << line_ << ")"; \
			fprintf(stderr, "%s\n", ss.str().c_str()); \
			ANKI_DEBUG_BREAK(); \
		} \
	} while(0);

/// Intermediate macro
#define ANKI_TEST_EXPECT_GEQ_IMPL(file_, line_, func_, x, y) \
	do \
	{ \
		if((x) < (y)) \
		{ \
			std::stringstream ss; \
			ss << "FAILURE: " << #x << " >= " << #y << " (" << file_ << ":" << line_ << ")"; \
			fprintf(stderr, "%s\n", ss.str().c_str()); \
			ANKI_DEBUG_BREAK(); \
		} \
	} while(0);

/// Intermediate macro
#define ANKI_TEST_EXPECT_LT_IMPL(file_, line_, func_, x, y) \
	do \
	{ \
		if((x) >= (y)) \
		{ \
			std::stringstream ss; \
			ss << "FAILURE: " << #x << " < " << #y << " (" << file_ << ":" << line_ << ")"; \
			fprintf(stderr, "%s\n", ss.str().c_str()); \
			ANKI_DEBUG_BREAK(); \
		} \
	} while(0);

/// Intermediate macro
#define ANKI_TEST_EXPECT_LEQ_IMPL(file_, line_, func_, x, y) \
	do \
	{ \
		if((x) > (y)) \
		{ \
			std::stringstream ss; \
			ss << "FAILURE: " << #x << " <= " << #y << " (" << file_ << ":" << line_ << ")"; \
			fprintf(stderr, "%s\n", ss.str().c_str()); \
			ANKI_DEBUG_BREAK(); \
		} \
	} while(0);

/// Intermediate macro
#define ANKI_TEST_EXPECT_NEAR_IMPL(file_, line_, func_, x, y, epsilon_) \
	do \
	{ \
		auto maxVal = ((x) > (y)) ? (x) : (y); \
		auto minVal = ((x) < (y)) ? (x) : (y); \
		if((maxVal - minVal) > (epsilon_)) \
		{ \
			std::stringstream ss; \
			ss << "FAILURE: " << #x << " != " << #y << " (" << file_ << ":" << line_ << ")"; \
			fprintf(stderr, "%s\n", ss.str().c_str()); \
			ANKI_DEBUG_BREAK(); \
		} \
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
#define ANKI_TEST_EXPECT_NO_ERR(x_) ANKI_TEST_EXPECT_EQ_IMPL(__FILE__, __LINE__, __func__, x_, Error::kNone)

/// Check error code.
#define ANKI_TEST_EXPECT_ANY_ERR(x_) ANKI_TEST_EXPECT_NEQ_IMPL(__FILE__, __LINE__, __func__, x_, Error::kNone)

/// Check error code.
#define ANKI_TEST_EXPECT_ERR(x_, y_) ANKI_TEST_EXPECT_EQ_IMPL(__FILE__, __LINE__, __func__, x_, y_)

void initConfig(ConfigSet& cfg);

NativeWindow* createWindow(ConfigSet& cfg);

GrManager* createGrManager(ConfigSet* cfg, NativeWindow* win);

ResourceManager* createResourceManager(ConfigSet* cfg, GrManager* gr, PhysicsWorld*& physics,
									   ResourceFilesystem*& resourceFs);

/// Stolen from https://en.cppreference.com/w/cpp/algorithm/random_shuffle because std::random_suffle got deprecated
template<class TRandomIt>
static void randomShuffle(TRandomIt first, TRandomIt last)
{
	typename std::iterator_traits<TRandomIt>::difference_type i, n;
	n = last - first;
	for(i = n - 1; i > 0; --i)
	{
		using std::swap;
		swap(first[i], first[std::rand() % (i + 1)]);
		// rand() % (i+1) isn't actually correct, because the generated number
		// is not uniformly distributed for most values of i. A correct implementation
		// will need to essentially reimplement C++11 std::uniform_int_distribution,
		// which is beyond the scope of this example.
	}
}

} // end namespace anki
