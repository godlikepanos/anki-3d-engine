// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/collision/CompoundShape.h>
#include <anki/collision/Aabb.h>
#include <cstring>

namespace anki {

//==============================================================================
CompoundShape::CompoundShape()
:	CollisionShape(Type::COMPOUND)
{
	memset(&m_dflt, 0, sizeof(m_dflt));
}

//==============================================================================
F32 CompoundShape::testPlane(const Plane& p) const
{
	F32 min = MAX_F32;
	F32 max = MIN_F32;

	Error err = iterateShapes([&](const CollisionShape& cs) -> Error
	{
		F32 a = cs.testPlane(p);
		min = std::min(min, a);
		max = std::max(max, a);
		return ErrorCode::NONE;
	});
	(void)err;

	if(min > 0.0 && max > 0.0)
	{
		return min;
	}
	else if(min < 0.0 && max < 0.0)
	{
		return max;
	}
	
	return 0.0;
}

//==============================================================================
void CompoundShape::accept(MutableVisitor& v)
{
	Error err = iterateShapes([&](CollisionShape& cs) -> Error
	{
		cs.accept(v);
		return ErrorCode::NONE;
	});
	(void)err;
}

//==============================================================================
void CompoundShape::accept(ConstVisitor& v) const
{
	Error err = iterateShapes([&](const CollisionShape& cs) -> Error
	{
		cs.accept(v);
		return ErrorCode::NONE;
	});
	(void)err;
}

//==============================================================================
void CompoundShape::transform(const Transform& trf)
{
	Error err = iterateShapes([&](CollisionShape& cs) -> Error
	{
		cs.transform(trf);
		return ErrorCode::NONE;
	});
	(void)err;
}

//==============================================================================
void CompoundShape::computeAabb(Aabb& out) const
{
	Vec4 min(Vec3(MAX_F32), 0.0), max(Vec3(MIN_F32), 0.0);

	Error err = iterateShapes([&](const CollisionShape& cs) -> Error
	{
		Aabb aabb;
		cs.computeAabb(aabb);

		for(U i = 0; i < 3; i++)
		{
			min[i] = std::min(min[i], aabb.getMin()[i]);
			max[i] = std::max(max[i], aabb.getMax()[i]);
		}
		return ErrorCode::NONE;
	});
	(void)err;

	out.setMin(min);
	out.setMax(max);
}

//==============================================================================
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

