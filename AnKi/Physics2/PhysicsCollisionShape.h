// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Physics2/Common.h>
#include <AnKi/Util/WeakArray.h>
#include <AnKi/Util/ClassWrapper.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

namespace anki {

/// Wrapper on top of JPH collision shapes.
class Physics2CollisionShape
{
	friend class Physics2World;
	template<typename, typename>
	friend class IntrusivePtr;
	template<typename, typename, typename>
	friend class BlockArray;

protected:
	enum ShapeType : U8
	{
		kBox,
		kSphere,
		kConvex,
		kTrimesh
	};

	union
	{
		ClassWrapper<JPH::BoxShape> m_box;
		ClassWrapper<JPH::SphereShape> m_sphere;
	};

	mutable Atomic<U32> m_refcount = {0};
	U32 m_arrayIndex : 24 = 0b111111111111111111111111;
	U32 m_type : 8;

	Physics2CollisionShape(ShapeType type)
		: m_type(type)
	{
	}

	~Physics2CollisionShape()
	{
		switch(m_type)
		{
		case ShapeType::kBox:
			m_box.destroy();
			break;
		case ShapeType::kSphere:
			m_sphere.destroy();
			break;
		default:
			ANKI_ASSERT(0);
		}
	}

	void retain() const
	{
		m_refcount.fetchAdd(1);
	}

	U32 release() const
	{
		return m_refcount.fetchSub(1);
	}
};

} // end namespace anki
