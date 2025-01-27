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
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Constraints/PointConstraint.h>
#include <Jolt/Physics/Constraints/HingeConstraint.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
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

#define ANKI_PHYSICS_DEFINE_PTRS(className) \
	class className; \
	class className##PtrDeleter \
	{ \
	public: \
		void operator()(className* ptr); \
	}; \
	using className##Ptr = IntrusivePtr<className, className##PtrDeleter>;

ANKI_PHYSICS_DEFINE_PTRS(PhysicsCollisionShape)
ANKI_PHYSICS_DEFINE_PTRS(PhysicsBody)
ANKI_PHYSICS_DEFINE_PTRS(PhysicsJoint)
ANKI_PHYSICS_DEFINE_PTRS(PhysicsPlayerController)
#undef ANKI_PHYSICS_DEFINE_PTRS

template<U32 kSize>
class StaticTempAllocator final : public JPH::TempAllocator
{
public:
	alignas(JPH_RVECTOR_ALIGNMENT) Array<U8, kSize> m_memory;
	U32 m_base = 0;

	void* Allocate(U32 size) override
	{
		ANKI_ASSERT(size);
		size = getAlignedRoundUp(JPH_RVECTOR_ALIGNMENT, size);
		ANKI_ASSERT(m_base + size <= sizeof(m_memory));
		void* out = &m_memory[0] + size;
		m_base += size;
		return out;
	}

	void Free([[maybe_unused]] void* address, U32 size) override
	{
		ANKI_ASSERT(address && size);
		size = getAlignedRoundUp(JPH_RVECTOR_ALIGNMENT, size);
		ANKI_ASSERT(m_base >= size);
		m_base -= size;
	}
};

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

inline Vec3 toAnKi(JPH::Vec3 x)
{
	return Vec3(x.GetX(), x.GetY(), x.GetZ());
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
