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
#define BT_NO_PROFILE 1
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#pragma GCC diagnostic pop

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

ANKI_USE_RESULT inline btTransform toBt(const Transform& a)
{
	Mat4 mat(a);
	mat.transpose();
	btTransform out;
	out.setFromOpenGLMatrix(&mat(0, 0));
	return out;
}

ANKI_USE_RESULT inline Mat3x4 toAnki(const btMatrix3x3& m)
{
	Mat3x4 m3;
	m3.setRows(Vec4(toAnki(m[0]), 0.0f), Vec4(toAnki(m[1]), 0.0f), Vec4(toAnki(m[2]), 0.0f));
	return m3;
}

ANKI_USE_RESULT inline Transform toAnki(const btTransform& t)
{
	Transform out;
	out.setRotation(toAnki(t.getBasis()));
	out.setOrigin(Vec4(toAnki(t.getOrigin()), 0.0f));
	out.setScale(1.0f);
	return out;
}
/// @}

} // end namespace anki
