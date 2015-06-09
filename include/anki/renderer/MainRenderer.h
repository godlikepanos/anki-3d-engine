// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_MAIN_RENDERER_H
#define ANKI_RENDERER_MAIN_RENDERER_H

#include "anki/renderer/Common.h"
#include "anki/core/Timestamp.h"
#include "anki/resource/Forward.h"

namespace anki {

// Forward
class ResourceManager;
class ConfigSet;
class SceneGraph;
class Camera;

/// @addtogroup renderer
/// @{

/// Main onscreen renderer
class MainRenderer
{
public:
	MainRenderer();

	~MainRenderer();

	ANKI_USE_RESULT Error create(
		Threadpool* threadpool,
		ResourceManager* resources,
		GrManager* gl,
		AllocAlignedCallback allocCb,
		void* allocCbUserData,
		const ConfigSet& config,
		const Timestamp* globalTimestamp);

	ANKI_USE_RESULT Error render(SceneGraph& scene);

	void prepareForVisibilityTests(Camera& cam);

	const String& getMaterialShaderSource() const
	{
		return m_materialShaderSource;
	}

	Dbg& getDbg();

	F32 getAspectRatio() const;

private:
	HeapAllocator<U8> m_alloc;
	StackAllocator<U8> m_frameAlloc;

	UniquePtr<Renderer> m_r;

	ShaderResourcePtr m_blitFrag;
	PipelinePtr m_blitPpline;

	FramebufferPtr m_defaultFb;
	U32 m_width = 0; ///< Default FB size.
	U32 m_height = 0; ///< Default FB size.

	String m_materialShaderSource; ///< String to append in user shaders

	F32 m_renderingQuality = 1.0;

	/// Optimize job chain
	Array<CommandBufferInitHints, RENDERER_COMMAND_BUFFERS_COUNT> m_cbInitHints;

	void initGl();
};
/// @}

} // end namespace anki

#endif
