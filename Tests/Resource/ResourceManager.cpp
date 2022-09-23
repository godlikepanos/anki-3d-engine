// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <AnKi/Resource/DummyResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Core/ConfigSet.h>

ANKI_TEST(Resource, ResourceManager)
{
	// Create
	ConfigSet config;

	HeapAllocator<U8> alloc(allocAligned, nullptr);

	ResourceManagerInitInfo rinit;
	rinit.m_gr = nullptr;
	rinit.m_config = &config;
	rinit.m_allocCallback = allocAligned;
	rinit.m_allocCallbackData = nullptr;
	ResourceManager* resources = alloc.newInstance<ResourceManager>();
	ANKI_TEST_EXPECT_NO_ERR(resources->init(rinit));

	// Very simple
	{
		DummyResourcePtr a;
		ANKI_TEST_EXPECT_NO_ERR(resources->loadResource("blah", a));
	}

	// Load a resource
	{
		DummyResourcePtr a;

		ANKI_TEST_EXPECT_NO_ERR(resources->loadResource("blah", a));

		{
			DummyResourcePtr b = a;
			a = b;
			b = a;
		}
	}

	// Load and load again
	{
		DummyResourcePtr a;
		ANKI_TEST_EXPECT_NO_ERR(resources->loadResource("blah", a));
		auto refcount = a->getRefcount();

		DummyResourcePtr b;
		ANKI_TEST_EXPECT_NO_ERR(resources->loadResource("blah", b));
		ANKI_TEST_EXPECT_EQ(b->getRefcount(), a->getRefcount());
		ANKI_TEST_EXPECT_EQ(a->getRefcount(), refcount + 1);

		ANKI_TEST_EXPECT_EQ(b.get(), a.get());

		// Again
		DummyResourcePtr c;
		ANKI_TEST_EXPECT_NO_ERR(resources->loadResource("blah", c));
		ANKI_TEST_EXPECT_EQ(a->getRefcount(), refcount + 2);

		// Load something else
		DummyResourcePtr d;
		ANKI_TEST_EXPECT_NO_ERR(resources->loadResource("blih", d));
		ANKI_TEST_EXPECT_EQ(a->getRefcount(), refcount + 2);
	}

	// Error
	{
		{
			DummyResourcePtr a;
			ANKI_TEST_EXPECT_EQ(resources->loadResource("error", a), Error::kUserData);
		}

		{
			DummyResourcePtr a;
			ANKI_TEST_EXPECT_EQ(resources->loadResource("error", a), Error::kUserData);
		}
	}

	// Delete
	alloc.deleteInstance(resources);
}
