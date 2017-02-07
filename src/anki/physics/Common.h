// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/StdTypes.h>
#include <anki/util/Enum.h>
#include <anki/util/Ptr.h>
#include <anki/Math.h>

// Common newton libs
#include <Newton.h>
#include <dLinearAlgebra.h>

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

ANKI_USE_RESULT inline Vec4 toAnki(const dVector& v)
{
	return Vec4(v.m_x, v.m_y, v.m_z, v.m_w);
}

ANKI_USE_RESULT inline dVector toNewton(const Vec4& v)
{
	return dVector(v.x(), v.y(), v.z(), v.w());
}

ANKI_USE_RESULT inline Mat4 toAnki(const dMatrix& m)
{
	Mat4 ak(*reinterpret_cast<const Mat4*>(&m));
	return ak.getTransposed();
}

ANKI_USE_RESULT inline dMatrix toNewton(const Mat4& m)
{
	Mat4 transp = m.getTransposed();
	return dMatrix(&transp(0, 0));
}
/// @}

} // end namespace anki
