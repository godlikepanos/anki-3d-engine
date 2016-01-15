// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/Common.h>
#include <anki/util/StdTypes.h>
#include <anki/Gr.h>
#include <anki/core/Timestamp.h>
#include <anki/resource/ResourceManager.h>
#include <anki/resource/ShaderResource.h>

namespace anki
{

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
	Bool getEnabled() const
	{
		return m_enabled;
	}

	void setEnabled(Bool e)
	{
		m_enabled = e;
	}

anki_internal:
	RenderingPass(Renderer* r)
		: m_r(r)
	{
	}

	~RenderingPass()
	{
	}

	Timestamp getGlobalTimestamp() const;

	HeapAllocator<U8> getAllocator() const;

	StackAllocator<U8> getFrameAllocator() const;

protected:
	Bool8 m_enabled = false;
	Renderer* m_r; ///< Know your father

	GrManager& getGrManager();
	const GrManager& getGrManager() const;

	ResourceManager& getResourceManager();
};
/// @}

} // end namespace anki
