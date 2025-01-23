// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/StdTypes.h>
#include <AnKi/Util/Enum.h>
#include <AnKi/Util/Ptr.h>
#include <AnKi/Util/MemoryPool.h>
#include <AnKi/Math.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/JobSystemThreadPool.h>

namespace anki {
namespace v2 {

#define ANKI_PHYS_LOGI(...) ANKI_LOG("PHYS", kNormal, __VA_ARGS__)
#define ANKI_PHYS_LOGE(...) ANKI_LOG("PHYS", kError, __VA_ARGS__)
#define ANKI_PHYS_LOGW(...) ANKI_LOG("PHYS", kWarning, __VA_ARGS__)
#define ANKI_PHYS_LOGF(...) ANKI_LOG("PHYS", kFatal, __VA_ARGS__)

#define ANKI_PHYSICS_COMMON_FRIENDS \
	friend class PhysicsWorld; \
	template<typename, typename> \
	friend class IntrusivePtr; \
	template<typename, typename, typename> \
	friend class BlockArray;

class PhysicsMemoryPool : public HeapMemoryPool, public MakeSingleton<PhysicsMemoryPool>
{
	template<typename>
	friend class MakeSingleton;

private:
	PhysicsMemoryPool(AllocAlignedCallback allocCb, void* allocCbUserData)
		: HeapMemoryPool(allocCb, allocCbUserData, "PhysicsMemPool")
	{
	}

	~PhysicsMemoryPool() = default;
};

ANKI_DEFINE_SUBMODULE_UTIL_CONTAINERS(Physics, PhysicsMemoryPool)

enum class PhysicsLayer : U8
{
	kStatic,
	kMoving,
	kCharacter,
	kCount
};

// Forward
class PhysicsCollisionShape;
class PhysicsBody;

/// Custom deleter.
class PhysicsCollisionShapePtrDeleter
{
public:
	void operator()(PhysicsCollisionShape* ptr);
};

using PhysicsCollisionShapePtr = IntrusivePtr<PhysicsCollisionShape, PhysicsCollisionShapePtrDeleter>;

/// Custom deleter.
class PhysicsBodyPtrDeleter
{
public:
	void operator()(PhysicsBody* ptr);
};

using PhysicsBodyPtr = IntrusivePtr<PhysicsBody, PhysicsBodyPtrDeleter>;

inline JPH::RVec3 toJPH(Vec3 ak)
{
	return JPH::RVec3(ak.x(), ak.y(), ak.z());
}

inline JPH::Quat toJPH(Quat ak)
{
	return JPH::Quat(ak.x(), ak.y(), ak.z(), ak.w());
}

inline Vec4 toAnKi(const JPH::Vec4& jph)
{
	return Vec4(jph.GetX(), jph.GetY(), jph.GetZ(), jph.GetW());
}

inline Transform toAnKi(const JPH::RMat44& jph)
{
	Mat4 m;
	for(U32 i = 0; i < 4; ++i)
	{
		m.setColumn(i, toAnKi(jph.GetColumn4(i)));
	}
	return Transform(m);
}

} // namespace v2
} // end namespace anki
