#ifndef ANKI_COLLISION_COMPOUND_SHAPE_H
#define ANKI_COLLISION_COMPOUND_SHAPE_H

#include "anki/collision/CollisionShape.h"
#include "anki/util/NonCopyable.h"

namespace anki {

/// @addtogroup collision
/// @{

/// A shape that contains other shapes
class CompoundShape: public CollisionShape, public NonCopyable
{
public:
	/// @name Constructors
	/// @{
	CompoundShape();
	/// @}

	/// Implements CollisionShape::testPlane
	F32 testPlane(const Plane& p) const;

	/// Implements CollisionShape::transform
	void transform(const Transform& trf);

	/// Implements CollisionShape::computeAabb
	void computeAabb(Aabb& b) const;

	/// Implements CollisionShape::accept
	void accept(MutableVisitor& v);

	/// Implements CollisionShape::accept
	void accept(ConstVisitor& v) const;

	/// The compound shape will not take ownership of the object
	void addShape(CollisionShape* shape);

private:
	static const U SHAPES_PER_CHUNK = 8;

	Array<CollisionShape*, SHAPES_PER_CHUNK> m_dflt;

	template<typename TFunc>
	void iterateShapes(TFunc f)
	{
		U idx = 0;
		while(m_dflt[idx])
		{
			if(m_dflt[idx])
			{
				f(*m_dflt[idx]);
			}
			++idx;
		}
		ANKI_ASSERT(idx > 0 && "Empty CompoundShape");
	}

	template<typename TFunc>
	void iterateShapes(TFunc f) const
	{
		U idx = 0;
		while(m_dflt[idx])
		{
			if(m_dflt[idx])
			{
				f(*m_dflt[idx]);
			}
			++idx;
		}
		ANKI_ASSERT(idx > 0 && "Empty CompoundShape");
	}
};

/// @}

} // end namespace anki

#endif


