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
#include "anki/util/Enum.h"

namespace anki {

namespace detail {

/// Dbg flags. Define them first so they can be parameter to the bitset
enum class DbgFlag
{
	NONE = 0,
	SPATIAL = 1 << 0,
	FRUSTUMABLE = 1 << 1,
	SECTOR = 1 << 2,
	OCTREE = 1 << 3,
	PHYSICS = 1 << 4,
	ALL = SPATIAL | FRUSTUMABLE | SECTOR | OCTREE | PHYSICS
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(DbgFlag, inline)

} // end namespace detail

/// @addtogroup renderer
/// @{

/// Debugging stage
class Dbg: public RenderingPass, public Bitset<detail::DbgFlag>
{
	friend class Renderer;

public:
	using Flag = detail::DbgFlag;

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
	DebugDrawer* m_drawer = nullptr;
	// Have it as ptr because the constructor calls opengl
	SceneDebugDrawer* m_sceneDrawer = nullptr;
	Bool8 m_depthTest = true;

	Dbg(Renderer* r)
	:	RenderingPass(r)
	{}
	
	~Dbg();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	ANKI_USE_RESULT Error run(GlCommandBufferHandle& jobs);
};

/// @}

} // end namespace

#endif
