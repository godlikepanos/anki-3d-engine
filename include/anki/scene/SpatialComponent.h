#ifndef ANKI_SCENE_SPATIAL_COMPONENT_H
#define ANKI_SCENE_SPATIAL_COMPONENT_H

#include "anki/scene/Common.h"
#include "anki/scene/SceneComponent.h"
#include "anki/Collision.h"
#include "anki/util/Bitset.h"

namespace anki {

/// @addtogroup Scene
/// @{

/// Spatial component for scene nodes. It indicates scene nodes that need to 
/// be placed in the a sector and they participate in the visibility tests
class SpatialComponent: public SceneComponent, public Bitset<U8>
{
public:
	/// Spatial flags
	enum SpatialFlag
	{
		SF_NONE = 0,
		SF_VISIBLE_CAMERA = 1 << 1,
		SF_VISIBLE_LIGHT = 1 << 2,

		/// Visible or not. The visibility tester sets it
		SF_VISIBLE_ANY = SF_VISIBLE_CAMERA | SF_VISIBLE_LIGHT,

		/// This is used for example in lights. If the light does not collide 
		/// with any surface then it shouldn't be visible and be processed 
		/// further. This flag is being used to check if we should test agains
		/// near plane when using the tiler for visibility tests.
		SF_FULLY_TRANSPARENT = 1 << 3,

		SF_MARKED_FOR_UPDATE = 1 << 4
	};

	/// Pass the collision shape here so we can avoid the virtuals
	/// @param node The scene node. Used only to steal it's allocators
	/// @param flags A mask of SpatialFlag
	SpatialComponent(SceneNode* node, U32 flags = SF_NONE);

	// Remove from current OctreeNode
	~SpatialComponent();

	/// @name Accessors
	/// @{
	virtual const CollisionShape& getSpatialCollisionShape() = 0;

	const Aabb& getAabb() const
	{
		return aabb;
	}

	/// Get optimal collision shape for visibility tests
	const CollisionShape& getVisibilityCollisionShape()
	{
		const CollisionShape& cs = getSpatialCollisionShape();
		if(cs.getCollisionShapeType() == CollisionShape::CST_SPHERE)
		{
			return cs;
		}
		else
		{
			return aabb;
		}
	}

	/// Used for sorting spatials. In most object the origin is the center of
	/// mess but for cameras the origin is the eye point
	virtual Vec3 getSpatialOrigin() = 0;
	/// @}

	/// The derived class has to manually call this method when the collision 
	/// shape got updated
	void markForUpdate()
	{
		enableBits(SF_MARKED_FOR_UPDATE);
	}

	/// @name SceneComponent overrides
	/// @{
	Bool update(SceneNode&, F32, F32, UpdateType) override;

	/// Disable some flags
	void reset() override;
	/// @}

	static Type getGlobType()
	{
		return SPATIAL_COMPONENT;
	}

private:
	Aabb aabb; ///< A faster shape
};
/// @}

} // end namespace anki

#endif
