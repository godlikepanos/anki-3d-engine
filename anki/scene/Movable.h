#ifndef ANKI_SCENE_MOVABLE_H
#define ANKI_SCENE_MOVABLE_H

#include "anki/util/Object.h"
#include "anki/math/Math.h"


namespace anki {


class PropertyMap;


/// @addtogroup Scene
/// @{

/// Interface for movable scene nodes
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
	Transform& getLocalTransform()
	{
		return lTrf;
	}
	void setLocalTransform(const Transform& x)
	{
		lTrf = x;
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
	}
	void rotateLocalY(float angDegrees)
	{
		lTrf.getRotation().rotateYAxis(angDegrees);
	}
	void rotateLocalZ(float angDegrees)
	{
		lTrf.getRotation().rotateZAxis(angDegrees);
	}
	void moveLocalX(float distance)
	{
		Vec3 x_axis = lTrf.getRotation().getColumn(0);
		lTrf.getOrigin() += x_axis * distance;
	}
	void moveLocalY(float distance)
	{
		Vec3 y_axis = lTrf.getRotation().getColumn(1);
		lTrf.getOrigin() += y_axis * distance;
	}
	void moveLocalZ(float distance)
	{
		Vec3 z_axis = lTrf.getRotation().getColumn(2);
		lTrf.getOrigin() += z_axis * distance;
	}
	/// @}

	/// This update happens always. It updates the MF_MOVED flag
	void updateWorldTransform();

	/// This is called after the updateWorldTransform() and if the MF_MOVED is
	/// true
	virtual void moveUpdate()
	{
		ANKI_ASSERT(isFlagEnabled(MF_MOVED));
	}

protected:
	Transform lTrf; ///< The transformation in local space

	/// The transformation in world space (local combined with parent's
	/// transformation)
	Transform wTrf;

	/// Keep the previous transformation for blurring calculations
	Transform prevWTrf;

	ulong flags; ///< The state flags
};
/// @}


} // end namespace


#endif
