// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Physics2/Common.h>
#include <AnKi/Util/ClassWrapper.h>

namespace anki {
namespace v2 {

/// @addtogroup physics
/// @{

/// Init info for PhysicsBody.
class PhysicsBodyInitInfo
{
public:
	PhysicsCollisionShape* m_shape = nullptr;
	F32 m_mass = 0.0f; ///< Zero mass means static object.
	Transform m_transform = Transform::getIdentity();
	F32 m_friction = 0.5f;
	PhysicsLayer m_layer = PhysicsLayer::kStatic;
};

/// Rigid body.
class PhysicsBody
{
	ANKI_PHYSICS_COMMON_FRIENDS
	friend class PhysicsBodyPtrDeleter;

public:
	PhysicsBody(const PhysicsBody&) = delete;

	PhysicsBody& operator=(const PhysicsBody&) = delete;

	const Transform& getTransform(U32* version = nullptr) const
	{
		if(version)
		{
			*version = m_worldTrfVersion;
		}
		return m_worldTrf;
	}

	void setTransform(const Transform& trf);

	void applyForce(const Vec3& force, const Vec3& relPos);

	/// Apply force to the center of mass.
	void applyForce(const Vec3& force);

	void activate(Bool activate);

	/// Zero means no gravity, 1 means normal gravity.
	void setGravityFactor(F32 factor);

private:
	JPH::Body* m_jphBody = nullptr;
	PhysicsCollisionShapePtr m_primaryShape;
	PhysicsCollisionShapePtr m_scaledShape;
	mutable Atomic<U32> m_refcount = {0};

	Transform m_worldTrf;
	U32 m_worldTrfVersion = 1;

	U32 m_arrayIndex : 31 = kMaxU32 >> 1u;
	U32 m_activated : 1 = false;

	PhysicsBody() = default;

	~PhysicsBody() = default;

	void retain() const
	{
		m_refcount.fetchAdd(1);
	}

	U32 release() const
	{
		return m_refcount.fetchSub(1);
	}

	void postPhysicsUpdate();
};
/// @}

} // namespace v2
} // end namespace anki
