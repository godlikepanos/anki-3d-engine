// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/collision/Forward.h>
#include <anki/collision/Common.h>
#include <anki/Math.h>
#include <anki/util/StdTypes.h>
#include <anki/util/Visitor.h>

namespace anki
{

/// @addtogroup collision
/// @{

/// Collision shape type
/// @note WARNING: Order is important
enum class CollisionShapeType : U8
{
	PLANE,
	LINE_SEG,
	COMPOUND,
	AABB,
	SPHERE,
	OBB,
	CONVEX_HULL,

	COUNT,
	LAST_CONVEX = CONVEX_HULL
};

/// Abstract class for collision shapes. It also features a visitor for implementing specific code outside the
/// collision codebase (eg rendering)
class CollisionShape
{
public:
	/// Generic mutable visitor
	class MutableVisitor
	{
	public:
		virtual ~MutableVisitor()
		{
		}

		virtual void visit(LineSegment&) = 0;
		virtual void visit(Obb&) = 0;
		virtual void visit(Plane&) = 0;
		virtual void visit(Sphere&) = 0;
		virtual void visit(Aabb&) = 0;
		virtual void visit(CompoundShape&) = 0;
		virtual void visit(ConvexHullShape&) = 0;
	};

	/// Generic const visitor
	class ConstVisitor
	{
	public:
		virtual ~ConstVisitor()
		{
		}

		virtual void visit(const LineSegment&) = 0;
		virtual void visit(const Obb&) = 0;
		virtual void visit(const Plane&) = 0;
		virtual void visit(const Sphere&) = 0;
		virtual void visit(const Aabb&) = 0;
		virtual void visit(const CompoundShape&) = 0;
		virtual void visit(const ConvexHullShape&) = 0;
	};

	CollisionShape(CollisionShapeType cid)
		: m_cid(cid)
	{
	}

	CollisionShape(const CollisionShape& b)
		: m_cid(b.m_cid)
	{
		operator=(b);
	}

	virtual ~CollisionShape()
	{
	}

	CollisionShape& operator=(const CollisionShape& b)
	{
		ANKI_ASSERT(b.m_cid == m_cid);
		(void)b;
		return *this;
	}

	CollisionShapeType getType() const
	{
		return m_cid;
	}

	/// If the collision shape intersects with the plane then the method returns 0.0, else it returns the distance. If
	/// the distance is < 0.0 then the collision shape lies behind the plane and if > 0.0 then in front of it
	virtual F32 testPlane(const Plane& p) const = 0;

	/// Transform
	virtual void transform(const Transform& trf) = 0;

	/// Get the AABB
	virtual void computeAabb(Aabb&) const = 0;

	/// Visitor accept
	virtual void accept(MutableVisitor&) = 0;
	/// Visitor accept
	virtual void accept(ConstVisitor&) const = 0;

protected:
	/// Function that iterates a point cloud
	template<typename TFunc>
	void iteratePointCloud(const void* buff, U count, PtrSize stride, PtrSize buffSize, TFunc func)
	{
		ANKI_ASSERT(buff);
		ANKI_ASSERT(count > 1);
		ANKI_ASSERT(stride >= sizeof(Vec3));
		ANKI_ASSERT(buffSize >= stride * count);

		const U8* ptr = (const U8*)buff;
		while(count-- != 0)
		{
			ANKI_ASSERT((((PtrSize)ptr + sizeof(Vec3)) - (PtrSize)buff) <= buffSize);
			const Vec3* pos = (const Vec3*)(ptr);

			func(*pos);

			ptr += stride;
		}
	}

private:
	/// Keep an ID to avoid (in some cases) the visitor and thus the cost of virtuals
	CollisionShapeType m_cid;
};
/// @}

} // end namespace anki
