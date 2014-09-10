// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_DBG_H
#define ANKI_RENDERER_DBG_H

#include "anki/renderer/RenderingPass.h"
#include "anki/Gl.h"
#include "anki/renderer/DebugDrawer.h"
#include "anki/util/Bitset.h"

namespace anki {

/// @addtogroup renderer
/// @{

/// Debugging stage
class Dbg: public RenderingPass, public Bitset<U8>
{
	friend class Renderer;

public:
	enum class Flag
	{
		NONE = 0,
		SPATIAL = 1 << 0,
		FRUSTUMABLE = 1 << 1,
		SECTOR = 1 << 2,
		OCTREE = 1 << 3,
		PHYSICS = 1 << 4,
		ALL = SPATIAL | FRUSTUMABLE | SECTOR | OCTREE | PHYSICS
	};

	Bool getDepthTestEnabled() const
	{
		return m_depthTest;
	}

	void setDepthTestEnabled(Bool enable)
	{
		m_depthTest = enable;
	}

	void switchDepthTestEnabled()
	{
		m_depthTest = !m_depthTest;
	}

private:
	GlFramebufferHandle m_fb;
	std::unique_ptr<DebugDrawer> m_drawer;
	// Have it as ptr because the constructor calls opengl
	std::unique_ptr<SceneDebugDrawer> m_sceneDrawer;
	Bool8 m_depthTest = true;

	Dbg(Renderer* r)
	:	RenderingPass(r)
	{}
	
	~Dbg();

	void init(const ConfigSet& initializer);
	void run(GlCommandBufferHandle& jobs);
};

/// @}

} // end namespace

#endif
