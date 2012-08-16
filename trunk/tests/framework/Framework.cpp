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
int Tester::run(int argc, char** argv)
{
	const char* helpMessage = ""

	programName = argv[0];

	// Parse args
	std::string suite;
	std::string test;

	for(int i = 1; i < argc; i++)
	{
		const char* arg = args[i];
		if(strcmp(arg, "--list-tests") == 0)
		{
			listTests();
			return 0;
		}
		else if(strcmp(arg, "--help") == 0)
		{
			// XXX
		}
		else if(strcmp(arg, "--suite") == 0)
		{
			if(i + 1 >= argc)
			{
				// XXX
			}
			suite = arg;
			++i;
		}
		else if(strcmp(arg, "--test") == 0)
		{
			if(i + 1 >= argc)
			{
				// XXX
			}
			test = arg;
			++i;
		}
	}

	// Run tests
	try
	{
		if(argc == 1)
		{
			// Run all
			for(TestSuite* suite : suites)
			{
				for(Test* test : suite->tests)
				{
					test->callback(*test);
				}
			}
		}
		else
		{
			// Run some
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}

	return 0;
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

//==============================================================================
int Tester::listTests()
{
	for(TestSuite* suite : suites)
	{
		for(Test* test : suite->tests)
		{
			std::cout << "./" << args[0] << " --suite " << suite->name 
				<< " --test " << test->name << std::endl;
		}
	}

	return 0;
}

} // end namespace anki
