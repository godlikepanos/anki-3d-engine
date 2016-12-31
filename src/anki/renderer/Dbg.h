// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RenderingPass.h>
#include <anki/Gr.h>
#include <anki/util/BitMask.h>
#include <anki/util/Enum.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Dbg flags. Define them first so they can be parameter to the bitset
enum class DbgFlag : U16
{
	NONE = 0,
	SPATIAL_COMPONENT = 1 << 0,
	FRUSTUM_COMPONENT = 1 << 1,
	SECTOR_COMPONENT = 1 << 2,
	DECAL_COMPONENT = 1 << 3,
	PHYSICS = 1 << 4,
	ALL = SPATIAL_COMPONENT | FRUSTUM_COMPONENT | SECTOR_COMPONENT | DECAL_COMPONENT | PHYSICS
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(DbgFlag, inline)

/// Debugging stage
class Dbg : public RenderingPass
{
public:
	Bool getEnabled() const
	{
		return m_enabled;
	}

	void setEnabled(Bool e)
	{
		m_enabled = e;
	}

	Bool getDepthTestEnabled() const;
	void setDepthTestEnabled(Bool enable);
	void switchDepthTestEnabled();

	void setFlags(DbgFlag flags)
	{
		m_flags.set(flags);
	}

	void unsetFlags(DbgFlag flags)
	{
		m_flags.unset(flags);
	}

	void flipFlags(DbgFlag flags)
	{
		m_flags.flip(flags);
	}

	TexturePtr getRt() const
	{
		return m_rt;
	}

anki_internal:
	Dbg(Renderer* r);

	~Dbg();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	ANKI_USE_RESULT Error run(RenderingContext& ctx);

private:
	Bool8 m_enabled = false;
	Bool8 m_initialized = false; ///< Lazily initialize.
	TexturePtr m_rt;
	FramebufferPtr m_fb;
	DebugDrawer* m_drawer = nullptr;
	BitMask<DbgFlag> m_flags;

	ANKI_USE_RESULT Error lazyInit();
};
/// @}

} // end namespace anki
