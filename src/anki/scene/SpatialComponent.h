// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/Common.h>
#include <anki/scene/SceneComponent.h>
#include <anki/Collision.h>
#include <anki/util/BitMask.h>
#include <anki/util/Enum.h>
#include <anki/util/List.h>

namespace anki
{

// Forward
class Sector;

/// @addtogroup scene
/// @{

/// Spatial component for scene nodes. It indicates scene nodes that need to be placed in the a sector and they
/// participate in the visibility tests.
class SpatialComponent : public SceneComponent
{
public:
	static const SceneComponentType CLASS_TYPE = SceneComponentType::SPATIAL;

	SpatialComponent(SceneNode* node, const CollisionShape* shape);

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

	const List<Sector*>& getSectorInfo() const
	{
		return m_sectorInfo;
	}

	/// Get optimal collision shape for visibility tests
	const CollisionShape& getVisibilityCollisionShape()
	{
		const CollisionShape& cs = getSpatialCollisionShape();
		if(cs.getType() == CollisionShapeType::SPHERE)
		{
			return cs;
		}
		else
		{
			return m_aabb;
		}
	}

	/// Check if it's confined in a single sector.
	Bool getSingleSector() const
	{
		return m_flags.get(Flag::SINGLE_SECTOR);
	}

	/// Confine it or not in a single sector.
	void setSingleSector(Bool yes)
	{
		m_flags.set(Flag::SINGLE_SECTOR, yes);
	}

	/// Used for sorting spatials. In most object the origin is the center of mass but for cameras the origin is the
	/// eye point.
	const Vec4& getSpatialOrigin() const
	{
		ANKI_ASSERT(m_origin.x() != MAX_F32);
		return m_origin;
	}

	void setSpatialOrigin(const Vec4& origin)
	{
		m_origin = origin;
	}

	/// The derived class has to manually call this method when the collision shape got updated.
	void markForUpdate()
	{
		m_flags.set(Flag::MARKED_FOR_UPDATE);
	}

	/// Set if visible by a camera
	void setVisibleByCamera(Bool visible)
	{
		m_flags.set(Flag::VISIBLE_CAMERA, visible);
	}

	/// Check if visible by camera
	Bool getVisibleByCamera() const
	{
		return m_flags.get(Flag::VISIBLE_CAMERA);
	}

	/// @name SceneComponent overrides
	/// @{
	ANKI_USE_RESULT Error update(SceneNode&, F32, F32, Bool& updated) override;
	/// @}

private:
	/// Spatial flags
	enum class Flag : U8
	{
		NONE = 0,
		VISIBLE_CAMERA = 1 << 1,
		VISIBLE_LIGHT = 1 << 2,
		VISIBLE_ANY = VISIBLE_CAMERA | VISIBLE_LIGHT,
		MARKED_FOR_UPDATE = 1 << 3,
		SINGLE_SECTOR = 1 << 4
	};
	ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(Flag, friend)

	const CollisionShape* m_shape;
	BitMask<Flag> m_flags;
	Aabb m_aabb; ///< A faster shape
	Vec4 m_origin = Vec4(MAX_F32, MAX_F32, MAX_F32, 0.0);
	List<Sector*> m_sectorInfo;
};
/// @}

} // end namespace anki
