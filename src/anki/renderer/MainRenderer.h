// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/Common.h>
#include <anki/resource/Forward.h>
#include <anki/core/Timestamp.h>

namespace anki
{

// Forward
class ResourceManager;
class ConfigSet;
class SceneGraph;
class SceneNode;
class ThreadPool;
class StagingGpuMemoryManager;

/// @addtogroup renderer
/// @{

/// Main onscreen renderer
class MainRenderer
{
public:
	MainRenderer();

	~MainRenderer();

	ANKI_USE_RESULT Error create(ThreadPool* threadpool,
		ResourceManager* resources,
		GrManager* gl,
		StagingGpuMemoryManager* stagingMem,
		AllocAlignedCallback allocCb,
		void* allocCbUserData,
		const ConfigSet& config,
		Timestamp* globTimestamp);

	ANKI_USE_RESULT Error render(SceneGraph& scene);

	const String& getMaterialShaderSource() const
	{
		return m_materialShaderSource;
	}

	Dbg& getDbg();

	F32 getAspectRatio() const;

	const Renderer& getOffscreenRenderer() const
	{
		return *m_r;
	}

	Renderer& getOffscreenRenderer()
	{
		return *m_r;
	}

private:
	HeapAllocator<U8> m_alloc;
	StackAllocator<U8> m_frameAlloc;

	UniquePtr<Renderer> m_r;
	Bool8 m_rDrawToDefaultFb = false;

	ShaderResourcePtr m_blitFrag;
	ShaderProgramPtr m_blitProg;

	FramebufferPtr m_defaultFb;
	U32 m_width = 0; ///< Default FB size.
	U32 m_height = 0; ///< Default FB size.

	String m_materialShaderSource; ///< String to append in user shaders

	F32 m_renderingQuality = 1.0;

	/// Optimize job chain
	CommandBufferInitHints m_cbInitHints;
};
/// @}

} // end namespace anki
