#ifndef ANKI_SCENE_MOVABLE_H
#define ANKI_SCENE_MOVABLE_H

#include "anki/util/Object.h"
#include "anki/math/Math.h"


namespace anki {


class PropertyMap;


/// @addtogroup Scene
/// @{

/// Interface for movable scene nodes
///
/// When the local tranform changes we calculate the @a dummyWTrf for this
/// movable and it's children. The @a dummyWTrf contains the final world
/// transformation. Why dont we set the @a wTrf at once? Because we the local
/// transformation may change in the logic part of the main loop and we dont
/// want
class Movable: public Object<Movable>
{
public:
	typedef Object<Movable> Base;

	enum MovableFlag
	{
		MF_NONE = 0,
		MF_IGNORE_LOCAL_TRANSFORM = 1, ///< Get the parent's world transform
		MF_MOVED = 2 ///< Moved in the previous frame
	};

	/// @name Constructors
	/// @{

	/// The one and only constructor
	/// @param flags_ The flags
	/// @param parent The parent. It can be nullptr
	/// @param pmap Property map to add a few variables
	Movable(uint flags_, Movable* parent, PropertyMap& pmap);
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
		shouldUpdateWTrf = true;
	}
	void setLocalTranslation(const Vec3& x)
	{
		lTrf.setOrigin(x);
		shouldUpdateWTrf = true;
	}
	void setLocalRotation(const Mat3& x)
	{
		lTrf.setRotation(x);
		shouldUpdateWTrf = true;
	}
	void setLocalScale(float x)
	{
		lTrf.setScale(x);
		shouldUpdateWTrf = true;
	}

	const Transform& getWorldTransform() const
	{
		return wTrf;
	}

	const Transform& getPrevWorldTransform() const
	{
		return prevWTrf;
	}

	ulong getFlags() const
	{
		return flags;
	}
	/// @}

	/// @name Flag manipulation
	/// @{
	void enableFlag(MovableFlag flag, bool enable = true)
	{
		flags = enable ? flags | flag : flags & ~flag;
	}
	void disableFlag(MovableFlag flag)
	{
		enableFlag(flag, false);
	}
	bool isFlagEnabled(MovableFlag flag) const
	{
		return flags & flag;
	}
	/// @}

	/// @name Mess with the local transform
	/// @{
	void rotateLocalX(float angDegrees)
	{
		lTrf.getRotation().rotateXAxis(angDegrees);
		shouldUpdateWTrf = true;
	}
	void rotateLocalY(float angDegrees)
	{
		lTrf.getRotation().rotateYAxis(angDegrees);
		shouldUpdateWTrf = true;
	}
	void rotateLocalZ(float angDegrees)
	{
		lTrf.getRotation().rotateZAxis(angDegrees);
		shouldUpdateWTrf = true;
	}
	void moveLocalX(float distance)
	{
		Vec3 x_axis = lTrf.getRotation().getColumn(0);
		lTrf.getOrigin() += x_axis * distance;
		shouldUpdateWTrf = true;
	}
	void moveLocalY(float distance)
	{
		Vec3 y_axis = lTrf.getRotation().getColumn(1);
		lTrf.getOrigin() += y_axis * distance;
		shouldUpdateWTrf = true;
	}
	void moveLocalZ(float distance)
	{
		Vec3 z_axis = lTrf.getRotation().getColumn(2);
		lTrf.getOrigin() += z_axis * distance;
		shouldUpdateWTrf = true;
	}
	/// @}

	/// This is called by the scene's main update method only when the object
	/// had actually moved
	virtual void moveUpdate()
	{}

	/// Update self and children world transform, if root node. Called every
	/// frame.
	/// @note Don't update if child because we start from roots and go to
	///       children and we don't want a child to be updated before the
	///       parent
	void update();

protected:
	bool shouldUpdateWTrf; ///< Its true when we change the local transform

	Transform lTrf; ///< The transformation in local space

	/// The transformation in world space (local combined with parent's
	/// transformation)
	Transform wTrf;

	/// Keep the previous transformation for checking if it moved
	Transform prevWTrf;

	ulong flags; ///< The state flags

	/// Called for every frame. It updates the @a wTrf if @a shouldUpdateWTrf
	/// is true. Then it moves to the children.
	void updateWorldTransform();
};
/// @}


} // end namespace


#endif
