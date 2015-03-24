// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_SPATIAL_COMPONENT_H
#define ANKI_SCENE_SPATIAL_COMPONENT_H

#include "anki/scene/Common.h"
#include "anki/scene/SceneComponent.h"
#include "anki/Collision.h"
#include "anki/util/Bitset.h"
#include "anki/util/Enum.h"
#include "anki/util/List.h"

namespace anki {

// Forward
class Sector;

/// @addtogroup scene
/// @{

/// Spatial flags
enum class SpatialComponentFlag: U8
{
	NONE = 0,
	VISIBLE_CAMERA = 1 << 1,
	VISIBLE_LIGHT = 1 << 2,

	/// Visible or not. The visibility tester sets it
	VISIBLE_ANY = VISIBLE_CAMERA | VISIBLE_LIGHT,

	/// This is used for example in lights. If the light does not collide 
	/// with any surface then it shouldn't be visible and be processed 
	/// further. This flag is being used to check if we should test agains
	/// near plane when using the tiler for visibility tests.
	FULLY_TRANSPARENT = 1 << 3,

	MARKED_FOR_UPDATE = 1 << 4
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(SpatialComponentFlag, inline)

/// Spatial component for scene nodes. It indicates scene nodes that need to 
/// be placed in the a sector and they participate in the visibility tests
class SpatialComponent: public SceneComponent, 
	public Bitset<SpatialComponentFlag>
{
public:
	using Flag = SpatialComponentFlag;

	static Bool classof(const SceneComponent& c)
	{
		return c.getType() == Type::SPATIAL;
	}

	SpatialComponent(
		SceneNode* node, 
		const CollisionShape* shape, 
		Flag flags = Flag::NONE);

	~SpatialComponent();

	const CollisionShape& getSpatialCollisionShape() const
	{
		return *m_shape;
	}

	const Aabb& getAabb() const
	{
		return m_aabb;
	}

	List<Sector*>& getSectorInfo()
	{
		return m_sectorInfo;
	}

	/// Get optimal collision shape for visibility tests
	const CollisionShape& getVisibilityCollisionShape()
	{
		const CollisionShape& cs = getSpatialCollisionShape();
		if(cs.getType() == CollisionShape::Type::SPHERE)
		{
			return cs;
		}
		else
		{
			return m_aabb;
		}
	}

	/// Used for sorting spatials. In most object the origin is the center of
	/// mess but for cameras the origin is the eye point
	const Vec4& getSpatialOrigin() const
	{
		ANKI_ASSERT(m_origin.x() != MAX_F32);
		return m_origin;
	}

	void setSpatialOrigin(const Vec4& origin)
	{
		m_origin = origin;
	}

	/// The derived class has to manually call this method when the collision 
	/// shape got updated
	void markForUpdate()
	{
		enableBits(Flag::MARKED_FOR_UPDATE);
	}

	/// @name SceneComponent overrides
	/// @{
	ANKI_USE_RESULT Error update(SceneNode&, F32, F32, Bool& updated) override;

	/// Disable some flags
	void reset() override;
	/// @}

private:
	const CollisionShape* m_shape;
	Aabb m_aabb; ///< A faster shape
	Vec4 m_origin = Vec4(MAX_F32, MAX_F32, MAX_F32, 0.0);
	List<Sector*> m_sectorInfo;
};
/// @}

} // end namespace anki

#endif
