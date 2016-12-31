// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/collision/CompoundShape.h>
#include <anki/collision/Aabb.h>
#include <cstring>

namespace anki
{

CompoundShape::CompoundShape()
	: CollisionShape(CollisionShapeType::COMPOUND)
{
	memset(&m_dflt, 0, sizeof(m_dflt));
}

F32 CompoundShape::testPlane(const Plane& p) const
{
	F32 minv = MAX_F32;
	F32 maxv = MIN_F32;

	Error err = iterateShapes([&](const CollisionShape& cs) -> Error {
		F32 a = cs.testPlane(p);
		minv = min(minv, a);
		maxv = max(maxv, a);
		return ErrorCode::NONE;
	});
	(void)err;

	if(minv > 0.0 && maxv > 0.0)
	{
		return minv;
	}
	else if(minv < 0.0 && maxv < 0.0)
	{
		return maxv;
	}

	return 0.0;
}

void CompoundShape::accept(MutableVisitor& v)
{
	Error err = iterateShapes([&](CollisionShape& cs) -> Error {
		cs.accept(v);
		return ErrorCode::NONE;
	});
	(void)err;
}

void CompoundShape::accept(ConstVisitor& v) const
{
	Error err = iterateShapes([&](const CollisionShape& cs) -> Error {
		cs.accept(v);
		return ErrorCode::NONE;
	});
	(void)err;
}

void CompoundShape::transform(const Transform& trf)
{
	Error err = iterateShapes([&](CollisionShape& cs) -> Error {
		cs.transform(trf);
		return ErrorCode::NONE;
	});
	(void)err;
}

void CompoundShape::computeAabb(Aabb& out) const
{
	Vec4 minv(Vec3(MAX_F32), 0.0), maxv(Vec3(MIN_F32), 0.0);

	Error err = iterateShapes([&](const CollisionShape& cs) -> Error {
		Aabb aabb;
		cs.computeAabb(aabb);

		for(U i = 0; i < 3; i++)
		{
			minv[i] = min(minv[i], aabb.getMin()[i]);
			maxv[i] = max(maxv[i], aabb.getMax()[i]);
		}
		return ErrorCode::NONE;
	});
	(void)err;

	out.setMin(minv);
	out.setMax(maxv);
}

void CompoundShape::addShape(CollisionShape* shape)
{
	Chunk* chunk = &m_dflt;

	do
	{
		U idx = SHAPES_PER_CHUNK_COUNT;
		while(idx-- != 0)
		{
			if(chunk->m_shapes[idx] == nullptr)
			{
				chunk->m_shapes[idx] = shape;
				return;
			}
		}

		chunk = chunk->m_next;
	} while(chunk);

	ANKI_ASSERT(0 && "TODO: Add code for adding new chunks");
}

} // end namespace anki
