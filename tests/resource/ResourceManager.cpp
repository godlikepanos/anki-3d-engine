// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "tests/framework/Framework.h"
#include "anki/resource/ResourceManager.h"
#include "anki/resource/DummyRsrc.h"
#include "anki/core/Config.h"
#include "anki/Gl.h"

namespace anki {

//==============================================================================
ANKI_TEST(Resource, ResourceManager)
{
	// Create
	Config config;
	
	HeapAllocator<U8> alloc(allocAligned, nullptr);

	GlDevice* gl = alloc.newInstance<GlDevice>();
	Error err = gl->create(allocAligned, nullptr, "/tmp/");
	ANKI_TEST_EXPECT_EQ(err, ErrorCode::NONE);

	ResourceManager::Initializer rinit;
	rinit.m_gl = gl;
	rinit.m_config = &config;
	rinit.m_cacheDir = "/tmp/";
	rinit.m_allocCallback = allocAligned;
	rinit.m_allocCallbackData = nullptr;
	ResourceManager* resources = alloc.newInstance<ResourceManager>(rinit);

	// Load a resource
	{
		DummyResourcePointer a;

		a.load("blah", resources);

		{
			DummyResourcePointer b = a;
			a = b;
			b = a;
		}
	}

	// Load and load again
	{
		DummyResourcePointer a;
		a.load("blah", resources);
		auto refcount = a.getReferenceCount();

		DummyResourcePointer b;
		b.load("blah", resources);
		ANKI_TEST_EXPECT_EQ(b.getReferenceCount(), a.getReferenceCount());
		ANKI_TEST_EXPECT_EQ(a.getReferenceCount(), refcount + 1);

		ANKI_TEST_EXPECT_EQ(b.get(), a.get());

		// Again
		DummyResourcePointer c;
		c.load("blah", resources);
		ANKI_TEST_EXPECT_EQ(a.getReferenceCount(), refcount + 2);

		// Load something else
		DummyResourcePointer d;
		d.load("blih", resources);
		ANKI_TEST_EXPECT_EQ(a.getReferenceCount(), refcount + 2);
	}

	// Exception
	{
		try
		{
			DummyResourcePointer a;
			a.load("exception", resources);
		}
		catch(...)
		{}

		DummyResourcePointer a;
		try
		{
			a.load("exception", resources);
		}
		catch(...)
		{}
	}

	// Delete
	alloc.deleteInstance(resources);
	alloc.deleteInstance(gl);
}

} // end namespace anki
