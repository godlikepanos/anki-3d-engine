// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "tests/framework/Framework.h"
#include "anki/resource/DummyResource.h"
#include "anki/resource/ResourceManager.h"
#include "anki/core/Config.h"

namespace anki
{

ANKI_TEST(Resource, ResourceManager)
{
	// Create
	Config config;

	HeapAllocator<U8> alloc(allocAligned, nullptr);

	ResourceManagerInitInfo rinit;
	rinit.m_gr = nullptr;
	rinit.m_config = &config;
	rinit.m_cacheDir = "/tmp/";
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
		auto refcount = a->getRefcount().load();

		DummyResourcePtr b;
		ANKI_TEST_EXPECT_NO_ERR(resources->loadResource("blah", b));
		ANKI_TEST_EXPECT_EQ(b->getRefcount().load(), a->getRefcount().load());
		ANKI_TEST_EXPECT_EQ(a->getRefcount().load(), refcount + 1);

		ANKI_TEST_EXPECT_EQ(b.get(), a.get());

		// Again
		DummyResourcePtr c;
		ANKI_TEST_EXPECT_NO_ERR(resources->loadResource("blah", c));
		ANKI_TEST_EXPECT_EQ(a->getRefcount().load(), refcount + 2);

		// Load something else
		DummyResourcePtr d;
		ANKI_TEST_EXPECT_NO_ERR(resources->loadResource("blih", d));
		ANKI_TEST_EXPECT_EQ(a->getRefcount().load(), refcount + 2);
	}

	// Error
	{
		{
			DummyResourcePtr a;
			ANKI_TEST_EXPECT_EQ(resources->loadResource("error", a), Error::USER_DATA);
		}

		{
			DummyResourcePtr a;
			ANKI_TEST_EXPECT_EQ(resources->loadResource("error", a), Error::USER_DATA);
		}
	}

	// Delete
	alloc.deleteInstance(resources);
}

} // end namespace anki
