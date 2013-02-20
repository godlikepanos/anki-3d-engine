#ifndef ANKI_SCENE_SCENE_NODE_H
#define ANKI_SCENE_SCENE_NODE_H

#include "anki/scene/Property.h"
#include "anki/scene/Common.h"
#include <string>

namespace anki {

class SceneGraph; // Don't include

// Components forward. Don't include
class Movable;
class Renderable;
class Frustumable;
class Spatial;
class Light;
class RigidBody;

/// @addtogroup Scene
/// @{

/// Interface class backbone of scene
class SceneNode: public PropertyMap
{
public:
	/// @name Constructors/Destructor
	/// @{

	/// The one and only constructor
	/// @param name The unique name of the node
	/// @param scene The scene that will register it
	explicit SceneNode(
		const char* name,
		SceneGraph* scene);

	/// Unregister node
	virtual ~SceneNode();
	/// @}

	/// @name Accessors
	/// @{
	const char* getName() const
	{
		return name.c_str();
	}

	SceneGraph& getSceneGraph()
	{
		return *scene;
	}

	SceneAllocator<U8> getSceneAllocator() const;

	SceneAllocator<U8> getSceneFrameAllocator() const;
	/// @}

	/// @name Accessors of components
	/// @{
	Movable* getMovable()
	{
		return sceneNodeProtected.movable;
	}
	const Movable* getMovable() const
	{
		return sceneNodeProtected.movable;
	}

	Renderable* getRenderable()
	{
		return sceneNodeProtected.renderable;
	}
	const Renderable* getRenderable() const
	{
		return sceneNodeProtected.renderable;
	}

	Frustumable* getFrustumable()
	{
		return sceneNodeProtected.frustumable;
	}
	const Frustumable* getFrustumable() const
	{
		return sceneNodeProtected.frustumable;
	}

	Spatial* getSpatial()
	{
		return sceneNodeProtected.spatial;
	}
	const Spatial* getSpatial() const
	{
		return sceneNodeProtected.spatial;
	}

	Light* getLight()
	{
		return sceneNodeProtected.light;
	}
	const Light* getLight() const 
	{
		return sceneNodeProtected.light;
	}

	RigidBody* getRigidBody()
	{
		return sceneNodeProtected.rigidBody;
	}
	const RigidBody* getRigidBody() const
	{
		return sceneNodeProtected.rigidBody;
	}
	/// @}

	/// This is called by the scene every frame after logic and before
	/// rendering. By default it does nothing
	/// @param[in] prevUpdateTime Timestamp of the previous update
	/// @param[in] crntTime Timestamp of this update
	/// @param[in] frame Frame number
	virtual void frameUpdate(F32 prevUpdateTime, F32 crntTime, I frame)
	{
		(void)prevUpdateTime;
		(void)crntTime;
		(void)frame;
	}

	/// Return the last frame the node was updated. It checks all components
	U32 getLastUpdateFrame();

protected:
	struct
	{
		Movable* movable = nullptr;
		Renderable* renderable = nullptr;
		Frustumable* frustumable = nullptr;
		Spatial* spatial = nullptr;
		Light* light = nullptr;
		RigidBody* rigidBody = nullptr;
	} sceneNodeProtected;

private:
	SceneString name; ///< A unique name
	SceneGraph* scene = nullptr; ///< Keep it here for unregistering
};
/// @}

} // end namespace anki

#endif
