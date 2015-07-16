// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/renderer/RenderingPass.h"
#include "anki/Gr.h"
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
public:
	using Flag = detail::DbgFlag;

	/// @privatesection
	/// @{
	Dbg(Renderer* r)
		: RenderingPass(r)
	{}

	~Dbg();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	ANKI_USE_RESULT Error run(CommandBufferPtr& jobs);

	Bool getDepthTestEnabled() const;
	void setDepthTestEnabled(Bool enable);
	void switchDepthTestEnabled();
	/// @}

private:
	FramebufferPtr m_fb;
	DebugDrawer* m_drawer = nullptr;
	// Have it as ptr because the constructor calls opengl
	SceneDebugDrawer* m_sceneDrawer = nullptr;
};
/// @}

} // end namespace anki

