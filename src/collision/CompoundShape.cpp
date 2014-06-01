#include "anki/collision/CompoundShape.h"
#include "anki/collision/Aabb.h"
#include <cstring>

namespace anki {

//==============================================================================
CompoundShape::CompoundShape()
	: CollisionShape(Type::COMPOUND)
{
	memset(&m_dflt, 0, sizeof(m_dflt));
}

//==============================================================================
F32 CompoundShape::testPlane(const Plane& p) const
{
	F32 min = MAX_F32;
	F32 max = MIN_F32;

	iterateShapes([&](const CollisionShape& cs)
	{
		F32 a = cs.testPlane(p);
		min = std::min(min, a);
		max = std::max(max, a);
	});

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
	iterateShapes([&](CollisionShape& cs)
	{
		cs.accept(v);
	});
}

//==============================================================================
void CompoundShape::accept(ConstVisitor& v) const
{
	iterateShapes([&](const CollisionShape& cs)
	{
		cs.accept(v);
	});
}

//==============================================================================
void CompoundShape::transform(const Transform& trf)
{
	iterateShapes([&](CollisionShape& cs)
	{
		cs.transform(trf);
	});
}

//==============================================================================
void CompoundShape::computeAabb(Aabb& out) const
{
	Vec3 min(MAX_F32), max(MIN_F32);

	iterateShapes([&](const CollisionShape& cs)
	{
		Aabb aabb;
		cs.computeAabb(aabb);

		for(U i = 0; i < 3; i++)
		{
			min[i] = std::min(min[i], aabb.getMin()[i]);
			max[i] = std::max(max[i], aabb.getMax()[i]);
		}
	});

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

