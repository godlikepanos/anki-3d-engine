#ifndef ANKI_SCENE_MOVABLE_H
#define ANKI_SCENE_MOVABLE_H

#include "anki/util/Object.h"
#include "anki/util/Flags.h"
#include "anki/math/Math.h"
#include "anki/scene/Scene.h"

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
		lastUpdateFrame = SceneSingleton::get().getFramesCount();
	}
	void setLocalTranslation(const Vec3& x)
	{
		lTrf.setOrigin(x);
		lastUpdateFrame = SceneSingleton::get().getFramesCount();
	}
	void setLocalRotation(const Mat3& x)
	{
		lTrf.setRotation(x);
		lastUpdateFrame = SceneSingleton::get().getFramesCount();
	}
	void setLocalScale(float x)
	{
		lTrf.setScale(x);
		lastUpdateFrame = SceneSingleton::get().getFramesCount();
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
		lastUpdateFrame = SceneSingleton::get().getFramesCount();
	}
	void rotateLocalY(float angDegrees)
	{
		lTrf.getRotation().rotateYAxis(angDegrees);
		lastUpdateFrame = SceneSingleton::get().getFramesCount();
	}
	void rotateLocalZ(float angDegrees)
	{
		lTrf.getRotation().rotateZAxis(angDegrees);
		lastUpdateFrame = SceneSingleton::get().getFramesCount();
	}
	void moveLocalX(float distance)
	{
		Vec3 x_axis = lTrf.getRotation().getColumn(0);
		lTrf.getOrigin() += x_axis * distance;
		lastUpdateFrame = SceneSingleton::get().getFramesCount();
	}
	void moveLocalY(float distance)
	{
		Vec3 y_axis = lTrf.getRotation().getColumn(1);
		lTrf.getOrigin() += y_axis * distance;
		lastUpdateFrame = SceneSingleton::get().getFramesCount();
	}
	void moveLocalZ(float distance)
	{
		Vec3 z_axis = lTrf.getRotation().getColumn(2);
		lTrf.getOrigin() += z_axis * distance;
		lastUpdateFrame = SceneSingleton::get().getFramesCount();
	}
	void scale(float s)
	{
		lTrf.getScale() += s;
		lastUpdateFrame = SceneSingleton::get().getFramesCount();
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
	uint32_t lastUpdateFrame = SceneSingleton::get().getFramesCount();

	/// Called for every frame. It updates the @a wTrf if @a shouldUpdateWTrf
	/// is true. Then it moves to the children.
	void updateWorldTransform();
};
/// @}

} // end namespace anki

#endif
