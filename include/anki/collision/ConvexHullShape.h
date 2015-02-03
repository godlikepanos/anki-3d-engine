// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_COLLISION_CONVEX_HULL_SHAPE_H
#define ANKI_COLLISION_CONVEX_HULL_SHAPE_H

#include "anki/collision/ConvexShape.h"
#include "anki/Math.h"

namespace anki {

/// @addtogroup collision
/// @{

/// Convex hull collision shape
class ConvexHullShape: public ConvexShape
{
public:
	using Base = ConvexShape;

	static Bool classof(const CollisionShape& c)
	{
		return c.getType() == Type::CONVEX_HULL;
	}

	ConvexHullShape()
	:	Base(Type::CONVEX_HULL)
	{}

	ConvexHullShape(ConvexHullShape&& b)
	:	Base(Type::CONVEX_HULL)
	{
		move(b);
	}

	ConvexHullShape(const ConvexHullShape& b) = delete;

	~ConvexHullShape();

	ConvexHullShape& operator=(const ConvexHullShape& b) = delete;

	ConvexHullShape& operator=(ConvexHullShape&& b)
	{
		move(b);
		return *this;
	}

	/// Calculate from a set of points. You have to call initStorage before
	/// calling this method.
	void setFromPointCloud(
		const Vec3* buff, U count, PtrSize stride, PtrSize buffSize);

	/// This function initializes the storage that holds the point cloud. This
	/// method allocates a storage and the owner is the convex hull. 
	/// @param alloc The allocator to use for the point cloud
	/// @param pointCount The number of points
	ANKI_USE_RESULT Error initStorage(
		CollisionAllocator<U8>& alloc, U pointCount);

	/// This function initializes the storage that holds the point cloud using
	/// a predefined buffer. The convex hull is not the owner of the storage.
	/// @param buffer The base of the storage buffer. Size should be 
	///               @a pointCount * sizeof(Vec4)
	/// @param pointCount The number of points
	void initStorage(void* buffer, U pointCount);

	/// Implements CollisionShape::accept
	void accept(MutableVisitor& v) override
	{
		v.visit(*this);
	}
	/// Implements CollisionShape::accept
	void accept(ConstVisitor& v) const override
	{
		v.visit(*this);
	}

	/// Implements CollisionShape::testPlane
	F32 testPlane(const Plane& p) const override;

	/// Implements CollisionShape::transform.
	void transform(const Transform& trf) override;

	/// Implements CollisionShape::computeAabb
	void computeAabb(Aabb& aabb) const override;

	/// Implements ConvexShape::computeSupport
	Vec4 computeSupport(const Vec4& dir) const override;

private:
	Transform m_trf = Transform::getIdentity();
	Vec4* m_points = nullptr;
	U32 m_pointsCount = 0;
	CollisionAllocator<Vec4> m_alloc;
	Bool8 m_ownsTheStorage = false;

	void move(ConvexHullShape& b);
};
/// @}

} // end namespace anki

#endif
