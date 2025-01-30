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

/// An interface to process contacts.
/// @memberof PhysicsBody
class PhysicsTriggerCallbacks
{
public:
	/// Will be called whenever a contact first touches a trigger.
	virtual void onTriggerEnter([[maybe_unused]] const PhysicsBody& trigger, [[maybe_unused]] const PhysicsObjectBase& obj)
	{
	}

	/// Will be called whenever a contact stops touching a trigger.
	virtual void onTriggerExit([[maybe_unused]] const PhysicsBody& trigger, [[maybe_unused]] const PhysicsObjectBase& obj)
	{
	}
};

/// Init info for PhysicsBody.
class PhysicsBodyInitInfo
{
public:
	PhysicsCollisionShape* m_shape = nullptr;
	F32 m_mass = 0.0f; ///< Zero mass means static object.
	Transform m_transform = Transform::getIdentity();
	F32 m_friction = 0.5f;
	PhysicsLayer m_layer = PhysicsLayer::kStatic;
	Bool m_isTrigger = false;
	void* m_userData = nullptr;
};

/// Rigid body.
class PhysicsBody : public PhysicsObjectBase
{
	ANKI_PHYSICS_COMMON_FRIENDS
	friend class PhysicsBodyPtrDeleter;

public:
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

	void setPhysicsTriggerCallbacks(PhysicsTriggerCallbacks* callbacks)
	{
		ANKI_ASSERT(m_isTrigger);
		m_triggerCallbacks = callbacks;
	}

private:
	JPH::Body* m_jphBody = nullptr;
	PhysicsCollisionShapePtr m_primaryShape;
	PhysicsCollisionShapePtr m_scaledShape;

	PhysicsTriggerCallbacks* m_triggerCallbacks = nullptr;

	Transform m_worldTrf;
	U32 m_worldTrfVersion = 1;

	U32 m_activated : 1 = false;
	U32 m_isTrigger : 1 = false;

	PhysicsBody()
		: PhysicsObjectBase(PhysicsObjectType::kBody)
	{
	}

	~PhysicsBody() = default;

	void init(const PhysicsBodyInitInfo& init);

	void postPhysicsUpdate();
};
/// @}

} // namespace v2
} // end namespace anki
