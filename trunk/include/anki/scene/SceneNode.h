#ifndef ANKI_SCENE_SCENE_NODE_H
#define ANKI_SCENE_SCENE_NODE_H

#include "anki/scene/Property.h"
#include "anki/scene/Common.h"
#include "anki/util/Object.h"

namespace anki {

class SceneGraph; // Don't include

// Components forward. Don't include
class Movable;
class Renderable;
class Frustumable;
class SpatialComponent;
class Light;
class RigidBody;
class Path;

/// @addtogroup Scene
/// @{

/// Interface class backbone of scene
class SceneNode: public Object<SceneNode, SceneAllocator<SceneNode>>,
	public PropertyMap
{
public:
	typedef Object<SceneNode, SceneAllocator<SceneNode>> Base;

	/// @name Constructors/Destructor
	/// @{

	/// The one and only constructor
	/// @param name The unique name of the node. If it's nullptr the the node
	///             is not searchable
	/// @param scene The scene that will register it
	/// @param parent The parent of tha node. Used mainly in movable nodes
	explicit SceneNode(
		const char* name,
		SceneGraph* scene,
		SceneNode* parent = nullptr);

	/// Unregister node
	virtual ~SceneNode();
	/// @}

	/// @name Accessors
	/// @{

	/// Return the name. It may be nullptr for nodes that we don't want to 
	/// track
	const char* getName() const
	{
		return name.length() > 0 ? name.c_str() : nullptr;
	}

	SceneAllocator<U8> getSceneAllocator() const;

	SceneAllocator<U8> getSceneFrameAllocator() const;

	SceneGraph& getSceneGraph()
	{
		return *scene;
	}
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

	SpatialComponent* getSpatial()
	{
		return sceneNodeProtected.spatial;
	}
	const SpatialComponent* getSpatial() const
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

	Path* getPath()
	{
		return sceneNodeProtected.path;
	}
	const Path* getPath() const
	{
		return sceneNodeProtected.path;
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
	U32 getLastUpdateFrame() const;

	/// Mark a node for deletion
	void markForDeletion()
	{
		markedForDeletion = true;
	}
	Bool isMarkedForDeletion()
	{
		return markedForDeletion;
	}

protected:
	struct
	{
		Movable* movable = nullptr;
		Renderable* renderable = nullptr;
		Frustumable* frustumable = nullptr;
		SpatialComponent* spatial = nullptr;
		Light* light = nullptr;
		RigidBody* rigidBody = nullptr;
		Path* path = nullptr;
	} sceneNodeProtected;

private:
	SceneGraph* scene = nullptr;
	SceneString name; ///< A unique name
	Bool8 markedForDeletion;
};
/// @}

} // end namespace anki

#endif
