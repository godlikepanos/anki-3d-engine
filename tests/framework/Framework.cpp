// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "tests/framework/Framework.h"
#include <iostream>
#include <cstring>
#include <malloc.h>

namespace anki
{

TestSuite::~TestSuite()
{
	for(Test* t : tests)
	{
		delete t;
	}
}

void Test::run()
{
	std::cout << "========\nRunning " << suite->name << " " << name << "\n========" << std::endl;

#if ANKI_OS == ANKI_OS_LINUX
	struct mallinfo a = mallinfo();
#endif

	callback(*this);

#if ANKI_OS == ANKI_OS_LINUX
	struct mallinfo b = mallinfo();

	int diff = b.uordblks - a.uordblks;
	if(diff > 0)
	{
		std::cerr << "Test leaks memory: " << diff << std::endl;
	}
#endif

	std::cout << std::endl;
}

void Tester::addTest(const char* name, const char* suiteName, TestCallback callback)
{
	std::vector<TestSuite*>::iterator it;
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
	std::vector<Test*>::iterator it1;
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

int Tester::run(int argc, char** argv)
{
	// Parse args
	//
	programName = argv[0];

	std::string helpMessage = "Usage: " + programName + R"( [options]
Options:
  --help         Print this message
  --list-tests   List all the tests
  --suite <name> Run tests only from this suite
  --test <name>  Run this test. --suite needs to be specified)";

	std::string suiteName;
	std::string testName;

	for(int i = 1; i < argc; i++)
	{
		const char* arg = argv[i];
		if(strcmp(arg, "--list-tests") == 0)
		{
			listTests();
			return 0;
		}
		else if(strcmp(arg, "--help") == 0)
		{
			std::cout << helpMessage << std::endl;
			return 0;
		}
		else if(strcmp(arg, "--suite") == 0)
		{
			++i;
			if(i >= argc)
			{
				std::cerr << "<name> is missing after --suite" << std::endl;
				return 1;
			}
			suiteName = argv[i];
		}
		else if(strcmp(arg, "--test") == 0)
		{
			++i;
			if(i >= argc)
			{
				std::cerr << "<name> is missing after --test" << std::endl;
				return 1;
			}
			testName = argv[i];
		}
	}

	// Sanity check
	if(testName.length() > 0 && suiteName.length() == 0)
	{
		std::cout << "Specify --suite as well" << std::endl;
		return 1;
	}

	// Run tests
	//
	int passed = 0;
	int run = 0;
	if(argc == 1)
	{
		// Run all
		for(TestSuite* suite : suites)
		{
			for(Test* test : suite->tests)
			{
				++run;
				test->run();
				++passed;
			}
		}
	}
	else
	{
		for(TestSuite* suite : suites)
		{
			if(suite->name == suiteName)
			{
				for(Test* test : suite->tests)
				{
					if(test->name == testName || testName.length() == 0)
					{
						++run;
						test->run();
						++passed;
					}
				}
			}
		}
	}

	int failed = run - passed;
	std::cout << "========\nRun " << run << " tests, failed " << failed << std::endl;

	if(failed == 0)
	{
		std::cout << "SUCCESS!" << std::endl;
	}
	else
	{
		std::cout << "FAILURE" << std::endl;
	}

	return run - passed;
}

int Tester::listTests()
{
	for(TestSuite* suite : suites)
	{
		for(Test* test : suite->tests)
		{
			std::cout << programName << " --suite \"" << suite->name << "\" --test \"" << test->name << "\""
					  << std::endl;
		}
	}

	return 0;
}

static Tester* testerInstance = nullptr;

Tester& getTesterSingleton()
{
	return *(testerInstance ? testerInstance : (testerInstance = new Tester));
}

void deleteTesterSingleton()
{
	if(testerInstance != nullptr)
	{
		delete testerInstance;
	}
}

NativeWindow* createWindow(U width, U height)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);

	NativeWindowInitInfo inf;
	inf.m_width = width;
	inf.m_height = height;
	inf.m_title = "AnKi unit tests";
	NativeWindow* win = new NativeWindow();

	ANKI_TEST_EXPECT_NO_ERR(win->init(inf, alloc));

	return win;
}

GrManager* createGrManager(NativeWindow* win)
{
	GrManager* gr = new GrManager();

	Config cfg;
	cfg.set("debugContext", true);
	cfg.set("vsync", false);
	GrManagerInitInfo inf;
	inf.m_allocCallback = allocAligned;
	inf.m_cacheDirectory = "./";
	inf.m_config = &cfg;
	inf.m_window = win;
	ANKI_TEST_EXPECT_NO_ERR(gr->init(inf));

	return gr;
}

ResourceManager* createResourceManager(GrManager* gr, PhysicsWorld*& physics, ResourceFilesystem*& resourceFs)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);

	physics = new PhysicsWorld();

	ANKI_TEST_EXPECT_NO_ERR(physics->create(allocAligned, nullptr));

	resourceFs = new ResourceFilesystem(alloc);
	Config cfg;
	ANKI_TEST_EXPECT_NO_ERR(resourceFs->init(cfg, "./"));

	ResourceManagerInitInfo rinit;
	rinit.m_gr = gr;
	rinit.m_physics = physics;
	rinit.m_resourceFs = resourceFs;
	rinit.m_config = &cfg;
	rinit.m_cacheDir = "./";
	rinit.m_allocCallback = allocAligned;
	rinit.m_allocCallbackData = nullptr;
	ResourceManager* resources = new ResourceManager();

	ANKI_TEST_EXPECT_NO_ERR(resources->init(rinit));

	return resources;
}

} // end namespace anki
