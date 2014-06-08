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
#include <memory>

namespace anki {

/// @addtogroup renderer
/// @{

/// Debugging stage
class Dbg: public SwitchableRenderingPass, public Bitset<U8>
{
	friend class Renderer;

public:
	enum DebugFlag
	{
		DF_NONE = 0,
		DF_SPATIAL = 1 << 0,
		DF_FRUSTUMABLE = 1 << 1,
		DF_SECTOR = 1 << 2,
		DF_OCTREE = 1 << 3,
		DF_PHYSICS = 1 << 4,
		DF_ALL = DF_SPATIAL | DF_FRUSTUMABLE | DF_SECTOR | DF_OCTREE
			| DF_PHYSICS
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
		: SwitchableRenderingPass(r)
	{}
	
	~Dbg();

	void init(const ConfigSet& initializer);
	void run(GlJobChainHandle& jobs);
};

/// @}

} // end namespace

#endif
