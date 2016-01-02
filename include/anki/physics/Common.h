// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/StdTypes.h>
#include <anki/util/Enum.h>
#include <anki/util/Ptr.h>
#include <anki/Math.h>
#include <Newton.h>

namespace anki {

// Forward
class PhysicsObject;
class PhysicsWorld;
class PhysicsCollisionShape;
class PhysicsBody;
class PhysicsPlayerController;

/// @addtogroup physics
/// @{

/// PhysicsPtr custom deleter.
class PhysicsPtrDeleter
{
public:
	void operator()(PhysicsObject* ptr);
};

/// Smart pointer for physics objects.
template<typename T>
using PhysicsPtr = IntrusivePtr<T, PhysicsPtrDeleter>;

using PhysicsCollisionShapePtr = PhysicsPtr<PhysicsCollisionShape>;
using PhysicsBodyPtr = PhysicsPtr<PhysicsBody>;
using PhysicsPlayerControllerPtr = PhysicsPtr<PhysicsPlayerController>;

/// Material types.
enum class PhysicsMaterialBit: U16
{
	NONE = 0,
	STATIC_GEOMETRY = 1 << 0,
	DYNAMIC_GEOMETRY = 1 << 1,
	RAGDOLL = 1 << 2,
	PARTICLES = 1 << 3,
	ALL = MAX_U16
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(PhysicsMaterialBit, inline)

/// Convert newton to AnKi.
ANKI_USE_RESULT inline Quat toAnki(const Quat& q)
{
	return Quat(q.y(), q.z(), q.w(), q.x());
}

/// Convert AnKi to Newton.
ANKI_USE_RESULT inline Quat toNewton(const Quat& q)
{
	return Quat(q.w(), q.x(), q.y(), q.z());
}

/// Convert newton to AnKi.
ANKI_USE_RESULT inline Mat4 toAnki(const Mat4& m)
{
	return m.getTransposed();
}

/// Convert AnKi to Newton.
ANKI_USE_RESULT inline Mat4 toNewton(const Mat4& m)
{
	return m.getTransposed();
}
/// @}

} // end namespace anki

