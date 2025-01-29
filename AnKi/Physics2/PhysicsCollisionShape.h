// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Physics2/Common.h>
#include <AnKi/Util/WeakArray.h>
#include <AnKi/Util/ClassWrapper.h>

namespace anki {
namespace v2 {

/// Wrapper on top of JPH collision shapes.
class PhysicsCollisionShape : public PhysicsObjectBase
{
	ANKI_PHYSICS_COMMON_FRIENDS
	friend class PhysicsCollisionShapePtrDeleter;
	friend class PhysicsPlayerController;
	friend class PhysicsBody;

public:
	PhysicsCollisionShape(const PhysicsCollisionShape&) = delete;

	PhysicsCollisionShape& operator=(const PhysicsCollisionShape&) = delete;

private:
	enum class ShapeType : U8
	{
		kBox,
		kSphere,
		kCapsule,
		kConvex,
		kTrimesh,
		kScaled, ///< This is for internal use

		kCount
	};

	union
	{
		ClassWrapper<JPH::Shape> m_shapeBase;
		ClassWrapper<JPH::BoxShape> m_box;
		ClassWrapper<JPH::SphereShape> m_sphere;
		ClassWrapper<JPH::CapsuleShape> m_capsule;
		ClassWrapper<JPH::ScaledShape> m_scaled; ///< We don't hold a reference to the target shape to avoid locking mutexes twice.
	};

	U32 m_arrayIndex = kMaxU32;
	ShapeType m_type;

	PhysicsCollisionShape(ShapeType type)
		: PhysicsObjectBase(PhysicsObjectType::kCollisionShape, nullptr)
		, m_type(type)
	{
		ANKI_ASSERT(type < ShapeType::kCount);
		ANKI_ASSERT(&m_shapeBase == static_cast<JPH::Shape*>(&m_box));
		ANKI_ASSERT(&m_shapeBase == static_cast<JPH::Shape*>(&m_sphere));
		ANKI_ASSERT(&m_shapeBase == static_cast<JPH::Shape*>(&m_capsule));
		ANKI_ASSERT(&m_shapeBase == static_cast<JPH::Shape*>(&m_scaled));
	}

	~PhysicsCollisionShape()
	{
		m_shapeBase.destroy();
	}
};

} // namespace v2
} // end namespace anki
