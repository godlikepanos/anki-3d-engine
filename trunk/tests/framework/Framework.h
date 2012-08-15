#ifndef ANKI_TESTS_FRAMEWORK_TESTER_H
#define ANKI_TESTS_FRAMEWORK_TESTER_H

#include "anki/util/Vector.h"
#include "anki/util/Singleton.h"
#include <string>
#include <iostream>

namespace anki {

class TestSuite;
class Test;
class Tester;

typedef void (*TestCallback)(Test&);

struct TestSuite
{
	std::string name;
	PtrVector<Test> tests;
};

struct Test
{
	std::string name;
	TestSuite* suite = nullptr;
	TestCallback callback = nullptr;
};

struct Tester
{
	PtrVector<TestSuite> suites;

	void addTest(const char* name, const char* suite, TestCallback callback);

	int runAll();
};

typedef Singleton<Tester> TesterSingleton;

//==============================================================================
// Macros

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


} // end namespace anki

#endif
