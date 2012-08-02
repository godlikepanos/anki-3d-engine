#ifndef ANKI_SCENE_MOVABLE_H
#define ANKI_SCENE_MOVABLE_H

#include "anki/util/Object.h"
#include "anki/util/Flags.h"
#include "anki/math/Math.h"

namespace anki {

class PropertyMap;

/// @addtogroup Scene
/// @{

/// Interface for movable scene nodes
class Movable: public Object<Movable>, public Flags<uint32_t>
{
public:
	typedef Object<Movable> Base;

	enum MovableFlag
	{
		MF_NONE = 0,
		MF_IGNORE_LOCAL_TRANSFORM = 1, ///< Get the parent's world transform
		MF_MOVED = 2 ///< Moved in the previous frame
	};

	/// @name Constructors & destructor
	/// @{

	/// The one and only constructor
	/// @param flags The flags
	/// @param parent The parent. It can be nullptr
	/// @param pmap Property map to add a few variables
	Movable(uint32_t flags, Movable* parent, PropertyMap& pmap);

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
		enableFlag(MF_MOVED);
	}
	void setLocalTranslation(const Vec3& x)
	{
		lTrf.setOrigin(x);
		enableFlag(MF_MOVED);
	}
	void setLocalRotation(const Mat3& x)
	{
		lTrf.setRotation(x);
		enableFlag(MF_MOVED);
	}
	void setLocalScale(float x)
	{
		lTrf.setScale(x);
		enableFlag(MF_MOVED);
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

	/// @name Mess with the local transform
	/// @{
	void rotateLocalX(float angDegrees)
	{
		lTrf.getRotation().rotateXAxis(angDegrees);
		enableFlag(MF_MOVED);
	}
	void rotateLocalY(float angDegrees)
	{
		lTrf.getRotation().rotateYAxis(angDegrees);
		enableFlag(MF_MOVED);
	}
	void rotateLocalZ(float angDegrees)
	{
		lTrf.getRotation().rotateZAxis(angDegrees);
		enableFlag(MF_MOVED);
	}
	void moveLocalX(float distance)
	{
		Vec3 x_axis = lTrf.getRotation().getColumn(0);
		lTrf.getOrigin() += x_axis * distance;
		enableFlag(MF_MOVED);
	}
	void moveLocalY(float distance)
	{
		Vec3 y_axis = lTrf.getRotation().getColumn(1);
		lTrf.getOrigin() += y_axis * distance;
		enableFlag(MF_MOVED);
	}
	void moveLocalZ(float distance)
	{
		Vec3 z_axis = lTrf.getRotation().getColumn(2);
		lTrf.getOrigin() += z_axis * distance;
		enableFlag(MF_MOVED);
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
	Transform lTrf; ///< The transformation in local space

	/// The transformation in world space (local combined with parent's
	/// transformation)
	Transform wTrf;

	/// Keep the previous transformation for checking if it moved
	Transform prevWTrf;

	/// Called for every frame. It updates the @a wTrf if @a shouldUpdateWTrf
	/// is true. Then it moves to the children.
	void updateWorldTransform();
};
/// @}

} // end namespace anki

#endif
