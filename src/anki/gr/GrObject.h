// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Common.h>
#include <anki/util/Atomic.h>
#include <anki/util/NonCopyable.h>

namespace anki
{

/// @addtogroup graphics
/// @{

/// Graphics object type.
enum class GrObjectType : U8
{
	BUFFER,
	COMMAND_BUFFER,
	FRAMEBUFFER,
	OCCLUSION_QUERY,
	TIMESTAMP_QUERY,
	SAMPLER,
	SHADER,
	TEXTURE,
	TEXTURE_VIEW,
	SHADER_PROGRAM,
	FENCE,
	RENDER_GRAPH,
	ACCELERATION_STRUCTURE,
	COUNT
};

/// Base of all graphics objects.
class GrObject : public NonCopyable
{
public:
	GrObject(GrManager* manager, GrObjectType type, CString name);

	virtual ~GrObject();

	GrObjectType getType() const
	{
		return m_type;
	}

	GrManager& getManager()
	{
		return *m_manager;
	}

	const GrManager& getManager() const
	{
		return *m_manager;
	}

	GrAllocator<U8> getAllocator() const;

	Atomic<I32>& getRefcount()
	{
		return m_refcount;
	}

	/// A unique identifier for caching objects.
	U64 getUuid() const
	{
		return m_uuid;
	}

	/// Get its name.
	CString getName() const
	{
		return m_name;
	}

private:
	GrManager* m_manager;
	CString m_name;
	U64 m_uuid;
	Atomic<I32> m_refcount;
	GrObjectType m_type;
};
/// @}

} // end namespace anki
