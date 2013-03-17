#ifndef ANKI_SCENE_MOVABLE_H
#define ANKI_SCENE_MOVABLE_H

#include "anki/util/Object.h"
#include "anki/util/Bitset.h"
#include "anki/math/Math.h"
#include "anki/core/Timestamp.h"
#include "anki/scene/Common.h"
#include <algorithm> // For std::find

namespace anki {

class PropertyMap;

/// @addtogroup Scene
/// @{

/// Interface for movable scene nodes
class Movable: public Object<Movable, SceneAllocator<Movable>>,
	public Bitset<U8>
{
public:
	typedef Object<Movable, SceneAllocator<Movable>> Base;

	enum MovableFlag
	{
		MF_NONE = 0,
		/// Get the parent's world transform
		MF_IGNORE_LOCAL_TRANSFORM = 1 << 1,
		/// If dirty then is marked for update
		MF_TRANSFORM_DIRTY = 1 << 2
	};

	/// @name Constructors & destructor
	/// @{

	/// The one and only constructor
	/// @param flags The flags
	/// @param parent The parent. It can be nullptr
	/// @param pmap Property map to add a few variables
	Movable(U32 flags, Movable* parent, PropertyMap& pmap,
		const SceneAllocator<Movable>& alloc);

	~Movable();
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
		movableMarkForUpdate();
	}
	void setLocalTranslation(const Vec3& x)
	{
		lTrf.setOrigin(x);
		movableMarkForUpdate();
	}
	void setLocalRotation(const Mat3& x)
	{
		lTrf.setRotation(x);
		movableMarkForUpdate();
	}
	void setLocalScale(F32 x)
	{
		lTrf.setScale(x);
		movableMarkForUpdate();
	}

	const Transform& getWorldTransform() const
	{
		return wTrf;
	}

	const Transform& getPrevWorldTransform() const
	{
		return prevWTrf;
	}

	U32 getMovableTimestamp() const
	{
		return timestamp;
	}
	/// @}

	/// @name Mess with the local transform
	/// @{
	void rotateLocalX(F32 angDegrees)
	{
		lTrf.getRotation().rotateXAxis(angDegrees);
		movableMarkForUpdate();
	}
	void rotateLocalY(F32 angDegrees)
	{
		lTrf.getRotation().rotateYAxis(angDegrees);
		movableMarkForUpdate();
	}
	void rotateLocalZ(F32 angDegrees)
	{
		lTrf.getRotation().rotateZAxis(angDegrees);
		movableMarkForUpdate();
	}
	void moveLocalX(F32 distance)
	{
		Vec3 x_axis = lTrf.getRotation().getColumn(0);
		lTrf.getOrigin() += x_axis * distance;
		movableMarkForUpdate();
	}
	void moveLocalY(F32 distance)
	{
		Vec3 y_axis = lTrf.getRotation().getColumn(1);
		lTrf.getOrigin() += y_axis * distance;
		movableMarkForUpdate();
	}
	void moveLocalZ(F32 distance)
	{
		Vec3 z_axis = lTrf.getRotation().getColumn(2);
		lTrf.getOrigin() += z_axis * distance;
		movableMarkForUpdate();
	}
	void scale(F32 s)
	{
		lTrf.getScale() *= s;
		movableMarkForUpdate();
	}
	/// @}

	/// This is called by the @a update() method only when the object had
	/// actually moved. It's overridable
	virtual void movableUpdate()
	{}

	/// Update self and children world transform recursively, if root node.
	/// Need to call this at every frame.
	/// @note Don't update if child because we start from roots and go to
	///       children and we don't want a child to be updated before the
	///       parent
	void update();

protected:
	/// The transformation in local space
	Transform lTrf = Transform::getIdentity();

	/// The transformation in world space (local combined with parent's
	/// transformation)
	Transform wTrf = Transform::getIdentity();

	/// Keep the previous transformation for checking if it moved
	Transform prevWTrf = Transform::getIdentity();

	/// The frame where it was last moved
	U32 timestamp = Timestamp::getTimestamp();

	/// Called for every frame. It updates the @a wTrf if @a shouldUpdateWTrf
	/// is true. Then it moves to the children.
	void updateWorldTransform();

	void movableMarkForUpdate()
	{
		timestamp = Timestamp::getTimestamp();
		enableBits(MF_TRANSFORM_DIRTY);
	}
};
/// @}

} // end namespace anki

#endif
