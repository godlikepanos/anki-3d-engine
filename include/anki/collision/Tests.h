// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_COLLISION_TESTS_H
#define ANKI_COLLISION_TESTS_H

#include "anki/collision/Common.h"

namespace anki {

/// @addtogroup collision
/// @{

/// Provides the collision algorithms that detect collision between various
/// shapes
/// @code
/// +------+------+------+------+------+------+------+
/// |      | PL   | COMP | AABB | SPH  | LINE | OBB  |
/// +------+------+------+------+------+------+------+
/// | PL   | N/A  | OK   | OK   | OK   | OK   | OK   |
/// +------+------+------+------+------+------+------+
/// | COMP |      | OK   | OK   | OK   | OK   | OK   |
/// +------+------+------+------+------+------+------+
/// | AABB |      |      | OK   | OK   | OK   | OK   |
/// +------+------+------+------+------+------+------+
/// | SPH  |      |      |      | OK   | OK   | OK   |
/// +------+------+------+------+------+------+------+
/// | LINE |      |      |      |      | OK   | OK   |
/// +------+------+------+------+------+------+------+
/// | OBB  |      |      |      |      |      | OK   |
/// +------+------+------+------+------+------+------+
/// @endcode
class CollisionTester
{
public:
	using TestCallback = U (*)(CollisionTester& self,
		const CollisionShape&, const CollisionShape&, 
		CollisionTempVector<ContactPoint>& points);

	CollisionTempAllocator<U8>& _getAllocator()
	{
		return m_alloc;
	}

	// Generic test function
	U test(const CollisionShape& a, const CollisionShape& b, 
		CollisionTempVector<ContactPoint>& points);

private:
	CollisionTempAllocator<U8> m_alloc;
};

/// @}

} // end namespace anki

#endif

