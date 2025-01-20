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

namespace anki {

#define ANKI_PHYS_LOGI(...) ANKI_LOG("PHYS", kNormal, __VA_ARGS__)
#define ANKI_PHYS_LOGE(...) ANKI_LOG("PHYS", kError, __VA_ARGS__)
#define ANKI_PHYS_LOGW(...) ANKI_LOG("PHYS", kWarning, __VA_ARGS__)
#define ANKI_PHYS_LOGF(...) ANKI_LOG("PHYS", kFatal, __VA_ARGS__)

class Physics2MemoryPool : public HeapMemoryPool, public MakeSingleton<Physics2MemoryPool>
{
	template<typename>
	friend class MakeSingleton;

private:
	Physics2MemoryPool(AllocAlignedCallback allocCb, void* allocCbUserData)
		: HeapMemoryPool(allocCb, allocCbUserData, "PhysicsMemPool")
	{
	}

	~Physics2MemoryPool() = default;
};

ANKI_DEFINE_SUBMODULE_UTIL_CONTAINERS(Physics2, Physics2MemoryPool)

// Forward
class Physics2CollisionShape;

/// Custom deleter.
class Physics2CollisionShapePtrDeleter
{
public:
	void operator()(Physics2CollisionShape* ptr);
};

using Physics2CollisionShapePtr = IntrusivePtr<Physics2CollisionShape, Physics2CollisionShapePtrDeleter>;

inline JPH::RVec3 toJPH(Vec3 ak)
{
	return JPH::RVec3(ak.x(), ak.y(), ak.z());
}

} // end namespace anki
