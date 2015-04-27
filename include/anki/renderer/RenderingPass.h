// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_RENDERING_PASS_H
#define ANKI_RENDERER_RENDERING_PASS_H

#include "anki/renderer/Common.h"
#include "anki/util/StdTypes.h"
#include "anki/Gr.h"
#include "anki/core/Timestamp.h"
#include "anki/resource/ResourceManager.h"
#include "anki/resource/ShaderResource.h"

namespace anki {

// Forward
class Renderer;
class ResourceManager;
class ConfigSet;

/// @addtogroup renderer
/// @{

/// Rendering pass
class RenderingPass
{
public:
	RenderingPass(Renderer* r)
	:	m_r(r)
	{}

	~RenderingPass()
	{}

	Bool getEnabled() const
	{
		return m_enabled;
	}
	void setEnabled(Bool e)
	{
		m_enabled = e;
	}

	Timestamp getGlobalTimestamp() const;

protected:
	Renderer* m_r; ///< Know your father
	Bool8 m_enabled = false;

	HeapAllocator<U8>& getAllocator();

	GrManager& getGrManager();
	const GrManager& getGrManager() const;

	ResourceManager& getResourceManager();
};

/// Blurring pass
class BlurringRenderingPass: public RenderingPass
{
protected:
	U32 m_blurringIterationsCount = 1; ///< The blurring iterations

	BlurringRenderingPass(Renderer* r)
	:	RenderingPass(r)
	{}

	class Direction
	{
	public:
		FramebufferHandle m_fb;
		TextureHandle m_rt; 
		ShaderResourcePointer m_frag;
		PipelineHandle m_ppline;
	};

	enum class DirectionEnum: U
	{
		VERTICAL,
		HORIZONTAL
	};

	Array<Direction, 2> m_dirs;

	ANKI_USE_RESULT Error initBlurring(Renderer& r, U width, U height, U samples, 
		F32 blurringDistance);

	ANKI_USE_RESULT Error runBlurring(Renderer& r, CommandBufferHandle& jobs);
};

/// @}

} // end namespace anki

#endif
