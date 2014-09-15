// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "tests/framework/Framework.h"
#include "anki/resource/ResourceManager.h"
#include "anki/core/Config.h"
#include "anki/Gl.h"

namespace anki {

//==============================================================================
ANKI_TEST(Resource, ResourceManager)
{
	// Create
	Config config;
	
	HeapAllocator<U8> alloc(HeapMemoryPool(allocAligned, nullptr));

	GlDevice* gl = alloc.newInstance<GlDevice>(
		allocAligned, nullptr, "/tmp/");

	ResourceManager::Initializer rinit;
	rinit.m_gl = gl;
	rinit.m_config = &config;
	rinit.m_cacheDir = "/tmp/";
	rinit.m_allocCallback = allocAligned;
	rinit.m_allocCallbackData = nullptr;
	ResourceManager* resources = alloc.newInstance<ResourceManager>(rinit);

	// Celete
	alloc.deleteInstance(resources);
	alloc.deleteInstance(gl);
}

} // end namespace anki
