#include "tests/framework/Framework.h"
#include <iostream>

namespace anki {

//==============================================================================
void Tester::addTest(const char* name, const char* suiteName,
	TestCallback callback)
{
	PtrVector<TestSuite>::iterator it;
	for(it = suites.begin(); it != suites.end(); it++)
	{
		if((*it)->name == suiteName)
		{
			break;
		}
	}

	// Not found
	TestSuite* suite = nullptr;
	if(it == suites.end())
	{
		suite = new TestSuite;
		suite->name = suiteName;
		suites.push_back(suite);
	}
	else
	{
		suite = *it;
	}

	// Sanity check
	PtrVector<Test>::iterator it1;
	for(it1 = suite->tests.begin(); it1 != suite->tests.end(); it1++)
	{
		if((*it)->name == name)
		{
			std::cerr << "Test already exists: " << name << std::endl;
			return;
		}
	}

	// Add the test
	Test* test = new Test;
	suite->tests.push_back(test);
	test->name = name;
	test->suite = suite;
	test->callback = callback;
}

//==============================================================================
int Tester::runAll()
{
	int errors = 0;
	for(TestSuite* suite : suites)
	{
		for(Test* test : suite->tests)
		{
			test->callback(*test);
		}
	}

	return errors;
}

} // end namespace anki
