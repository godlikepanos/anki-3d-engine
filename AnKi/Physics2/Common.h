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
#include <AnKi/Util/Enum.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/NarrowPhaseQuery.h>
#include <Jolt/Physics/Collision/CastResult.h>
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
	friend class anki::IntrusivePtr; \
	template<typename, typename, typename> \
	friend class anki::BlockArray;

class PhysicsMemoryPool : public HeapMemoryPool, public MakeSingleton<PhysicsMemoryPool>
{
	template<typename>
	friend class anki::MakeSingleton;

private:
	PhysicsMemoryPool(AllocAlignedCallback allocCb, void* allocCbUserData)
		: HeapMemoryPool(allocCb, allocCbUserData, "PhysicsMemPool")
	{
	}

	~PhysicsMemoryPool() = default;
};

ANKI_DEFINE_SUBMODULE_UTIL_CONTAINERS(Physics, PhysicsMemoryPool)

enum class PhysicsObjectType : U8
{
	kNone,
	kCollisionShape,
	kBody,
	kPlayerController,
	kJoint,

	kCount
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(PhysicsObjectType);

/// The base class of all physics objects.
class PhysicsObjectBase
{
	template<typename, typename>
	friend class IntrusivePtr;

public:
	PhysicsObjectBase(const PhysicsObjectBase&) = delete;

	PhysicsObjectBase& operator=(const PhysicsObjectBase&) = delete;

	PhysicsObjectType getType() const
	{
		return PhysicsObjectType(m_type);
	}

	void* getUserData() const
	{
		return m_userData;
	}

	void setUserData(void* userData)
	{
		m_userData = userData;
	}

protected:
	static constexpr U32 kTypeBits = 5u; ///< 5 is more than enough

	void* m_userData = nullptr;

	mutable Atomic<U32> m_refcount = {0};

	U32 m_type : kTypeBits;
	U32 m_blockArrayIndex : 32 - kTypeBits = kMaxU32 >> kTypeBits;

	PhysicsObjectBase(PhysicsObjectType type)
		: m_type(U32(type))
	{
		ANKI_ASSERT(type < PhysicsObjectType::kCount);
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

/// The physics layers a PhysicsBody can be.
enum class PhysicsLayer : U8
{
	kStatic,
	kMoving,
	kPlayerController,
	kTrigger,
	kDebris,

	kCount,
	kFirst = 0
};

enum class PhysicsLayerBit : U8
{
	kNone = 0,

	kStatic = 1u << 0u,
	kMoving = 1u << 1u,
	kPlayerController = 1u << 2u,
	kTrigger = 1u << 3u,
	kDebris = 1u << 4u,

	kAll = kStatic | kMoving | kPlayerController | kTrigger | kDebris
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(PhysicsLayerBit)

/// Table that shows which layer collides with whom.
inline constexpr Array<PhysicsLayerBit, U32(PhysicsLayer::kCount)> kPhysicsLayerCollisionTable = {
	{/* kStatic           */ PhysicsLayerBit::kAll & ~(PhysicsLayerBit::kStatic | PhysicsLayerBit::kTrigger),
	 /* kMoving           */ PhysicsLayerBit::kAll & ~(PhysicsLayerBit::kDebris),
	 /* kPlayerController */ PhysicsLayerBit::kAll & ~(PhysicsLayerBit::kDebris),
	 /* kTrigger          */ PhysicsLayerBit::kAll & ~(PhysicsLayerBit::kStatic | PhysicsLayerBit::kTrigger),
	 /* kDebris           */ PhysicsLayerBit::kStatic}};

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
