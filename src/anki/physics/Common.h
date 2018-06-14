// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/StdTypes.h>
#include <anki/util/Enum.h>
#include <anki/util/Ptr.h>
#include <anki/Math.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winfinite-recursion"
#define BT_THREADSAFE 0
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#pragma GCC diagnostic pop

// Have all the newton headers here because they polute the global namespace
#include <Newton.h>
#include <dLinearAlgebra.h>
#include <dCustomPlayerControllerManager.h>
#include <anki/util/CleanupWindows.h>

namespace anki
{

#define ANKI_PHYS_LOGI(...) ANKI_LOG("PHYS", NORMAL, __VA_ARGS__)
#define ANKI_PHYS_LOGE(...) ANKI_LOG("PHYS", ERROR, __VA_ARGS__)
#define ANKI_PHYS_LOGW(...) ANKI_LOG("PHYS", WARNING, __VA_ARGS__)
#define ANKI_PHYS_LOGF(...) ANKI_LOG("PHYS", FATAL, __VA_ARGS__)

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
enum class PhysicsMaterialBit : U16
{
	NONE = 0,
	STATIC_GEOMETRY = 1 << 0,
	DYNAMIC_GEOMETRY = 1 << 1,
	RAGDOLL = 1 << 2,
	PARTICLES = 1 << 3,
	ALL = MAX_U16
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(PhysicsMaterialBit, inline)

ANKI_USE_RESULT inline Vec3 toAnki(const btVector3& v)
{
	return Vec3(v.getX(), v.getY(), v.getZ());
}

ANKI_USE_RESULT inline btVector3 toBt(const Vec3& v)
{
	return btVector3(v.x(), v.y(), v.z());
}
/// @}

} // end namespace anki
