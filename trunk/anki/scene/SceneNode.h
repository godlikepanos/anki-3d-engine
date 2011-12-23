#ifndef ANKI_SCENE_SCENE_NODE_H
#define ANKI_SCENE_SCENE_NODE_H

#include "anki/math/Math.h"
#include "anki/collision/Obb.h"
#include "anki/util/Object.h"
#include <string>


namespace anki {


/// @addtogroup scene
/// @{

/// Interface class backbone of scene
class SceneNode: public Object<SceneNode>
{
public:
	typedef Object<SceneNode> Base;

	enum SceneNodeFlags
	{
		SNF_NONE = 0,
		SNF_INHERIT_PARENT_TRANSFORM = 1 ///< Ignore local transform
	};

	/// The one and only constructor
	/// @param flags The flags with the node properties
	/// @param parent The nods's parent. Its nulptr only for the root node
	explicit SceneNode(ulong flags, SceneNode* parent)
		: Base(parent), flags(flags_)
	{}

	virtual ~SceneNode();

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
	Transform& getWorldTransform()
	{
		return wTrf;
	}
	void setWorldTransform(const Transform& x)
	{
		wTrf = x;
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
	void enableFlag(SceneNodeFlags flag, bool enable = true)
	{
		if(enable)
		{
			flags |= flag;
		}
		else
		{
			flags &= ~flag;
		}
	}
	void disableFlag(SceneNodeFlags flag)
	{
		enableFlag(flag, false);
	}
	bool isFlagEnabled(SceneNodeFlags flag) const
	{
		return flags & flag;
	}
	/// @}

	/// @name Updates
	/// Two separate updates for the main loop. The update happens anyway
	/// and the updateTrf only when the node is being moved
	/// @{

	/// This is called every frame
	virtual void frameUpdate(float prevUpdateTime, float crntTime)
	{}

	/// This is called if the node moved
	virtual void moveUpdate()
	{}
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

	/// This update happens only when the object gets moved. Called only by
	/// the Scene
	void updateWorldTransform();

private:
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
