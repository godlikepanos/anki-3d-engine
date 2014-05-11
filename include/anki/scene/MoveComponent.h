#ifndef ANKI_SCENE_MOVE_COMPONENT_H
#define ANKI_SCENE_MOVE_COMPONENT_H

#include "anki/scene/Common.h"
#include "anki/scene/SceneComponent.h"
#include "anki/util/Bitset.h"
#include "anki/Math.h"

namespace anki {

/// @addtogroup Scene
/// @{

/// Interface for movable scene nodes
class MoveComponent: 
	public SceneComponent,
	public SceneHierarchicalObject<MoveComponent>, 
	public Bitset<U8>
{
public:
	typedef SceneHierarchicalObject<MoveComponent> Base;

	enum MoveComponentFlag
	{
		MF_NONE = 0,

		/// Get the parent's world transform
		MF_IGNORE_LOCAL_TRANSFORM = 1 << 1,

		/// If dirty then is marked for update
		MF_MARKED_FOR_UPDATE = 1 << 3,
	};

	/// @name Constructors & destructor
	/// @{

	/// The one and only constructor
	/// @param node The scene node to steal it's allocators
	/// @param flags The flags
	MoveComponent(SceneNode* node, U32 flags = MF_NONE);

	~MoveComponent();
	/// @}

	/// @name Accessors
	/// @{
	const Transform& getLocalTransform() const
	{
		return lTrf;
	}
	void setLocalTransform(const Transform& x)
	{
		lTrf = x;
		markForUpdate();
	}
	void setLocalOrigin(const Vec3& x)
	{
		lTrf.setOrigin(x);
		markForUpdate();
	}
	const Vec3& getLocalOrigin() const
	{
		return lTrf.getOrigin();
	}
	void setLocalRotation(const Mat3& x)
	{
		lTrf.setRotation(x);
		markForUpdate();
	}
	const Mat3& getLocalRotation() const
	{
		return lTrf.getRotation();
	}
	void setLocalScale(F32 x)
	{
		lTrf.setScale(x);
		markForUpdate();
	}
	F32 getLocalScale() const
	{
		return lTrf.getScale();
	}

	const Transform& getWorldTransform() const
	{
		return wTrf;
	}

	const Transform& getPrevWorldTransform() const
	{
		return prevWTrf;
	}
	/// @}

	/// @name SceneComponent overrides
	/// @{

	/// Update self and children world transform recursively, if root node.
	/// Need to call this at every frame.
	/// @note Don't update if child because we start from roots and go to
	///       children and we don't want a child to be updated before the
	///       parent
	Bool update(SceneNode&, F32, F32, UpdateType uptype) override;
	/// @}

	/// @name Mess with the local transform
	/// @{
	void rotateLocalX(F32 angDegrees)
	{
		lTrf.getRotation().rotateXAxis(angDegrees);
		markForUpdate();
	}
	void rotateLocalY(F32 angDegrees)
	{
		lTrf.getRotation().rotateYAxis(angDegrees);
		markForUpdate();
	}
	void rotateLocalZ(F32 angDegrees)
	{
		lTrf.getRotation().rotateZAxis(angDegrees);
		markForUpdate();
	}
	void moveLocalX(F32 distance)
	{
		Vec3 x_axis = lTrf.getRotation().getColumn(0);
		lTrf.getOrigin() += x_axis * distance;
		markForUpdate();
	}
	void moveLocalY(F32 distance)
	{
		Vec3 y_axis = lTrf.getRotation().getColumn(1);
		lTrf.getOrigin() += y_axis * distance;
		markForUpdate();
	}
	void moveLocalZ(F32 distance)
	{
		Vec3 z_axis = lTrf.getRotation().getColumn(2);
		lTrf.getOrigin() += z_axis * distance;
		markForUpdate();
	}
	void scale(F32 s)
	{
		lTrf.getScale() *= s;
		markForUpdate();
	}
	/// @}

	static constexpr Type getClassType()
	{
		return MOVE_COMPONENT;
	}

private:
	SceneNode* node;

	/// The transformation in local space
	Transform lTrf = Transform::getIdentity();

	/// The transformation in world space (local combined with parent's
	/// transformation)
	Transform wTrf = Transform::getIdentity();

	/// Keep the previous transformation for checking if it moved
	Transform prevWTrf = Transform::getIdentity();

	void markForUpdate()
	{
		enableBits(MF_MARKED_FOR_UPDATE);
	}

	/// Called every frame. It updates the @a wTrf if @a shouldUpdateWTrf
	/// is true. Then it moves to the children.
	void updateWorldTransform();
};
/// @}

} // end namespace anki

#endif
