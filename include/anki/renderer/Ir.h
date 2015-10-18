// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/Renderer.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Image based reflections.
class Ir
{
anki_internal:
	Ir()
	{}

	~Ir();

	ANKI_USE_RESULT Error init(
		ThreadPool* threadpool,
		ResourceManager* resources,
		GrManager* gr,
		HeapAllocator<U8> alloc,
		StackAllocator<U8> frameAlloc,
		const ConfigSet& initializer,
		const Timestamp* globalTimestamp);

	ANKI_USE_RESULT Error run(SceneNode& frustumNode);

	HeapAllocator<U8> getAllocator() const
	{
		return m_r.getAllocator();
	}

	TexturePtr getCubemapArray() const
	{
		return m_cubemapArr;
	}

private:
	Renderer m_r;
	TexturePtr m_cubemapArr;
	U16 m_cubemapArrSize = 0;
	U16 m_fbSize = 0;

	ANKI_USE_RESULT Error renderReflection(SceneNode& node);
};
/// @}

} // end namespace anki

