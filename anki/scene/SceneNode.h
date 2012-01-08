#ifndef ANKI_SCENE_SCENE_NODE_H
#define ANKI_SCENE_SCENE_NODE_H

#include "anki/math/Math.h"
#include "anki/collision/Obb.h"
#include "anki/util/Object.h"
#include <string>


namespace anki {


class Scene;
class Renderable;
class Frustum;


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
		SNF_IGNORE_LOCAL_TRANSFORM = 1, ///< Get the parent's world transform
		SNF_MOVED = 2 ///< Moved in the previous frame. The scene update sets it
	};

	/// The one and only constructor
	/// @param flags The flags with the node properties
	/// @param parent The nods's parent. It can be nullptr
	explicit SceneNode(
		const char* name,
		long flags,
		SceneNode* parent,
		Scene* scene);

	/// Unregister node
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
	{
		(void)prevUpdateTime;
		(void)crntTime;
	}

	/// This is called if the node moved
	virtual void moveUpdate()
	{}
	/// @}

	virtual Renderable* getRenderable()
	{
		return NULL;
	}

	virtual Frustum* getFrustum()
	{
		return NULL;
	}

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
	std::string name; ///< A unique name

	Transform lTrf; ///< The transformation in local space

	/// The transformation in world space (local combined with parent's
	/// transformation)
	Transform wTrf;

	/// Keep the previous transformation for blurring calculations
	Transform prevWTrf;

	ulong flags; ///< The state flags

	Scene* scene; ///< For registering and unregistering
};
/// @}


} // end namespace


#endif
