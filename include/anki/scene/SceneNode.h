#ifndef ANKI_SCENE_SCENE_NODE_H
#define ANKI_SCENE_SCENE_NODE_H

#include "anki/scene/Property.h"
#include "anki/scene/Common.h"
#include <string>

namespace anki {

class Scene; // Don't include

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
		Scene* scene);

	/// Unregister node
	virtual ~SceneNode();
	/// @}

	/// @name Accessors
	/// @{
	const char* getName() const
	{
		return name.c_str();
	}

	Scene& getScene()
	{
		return *scene;
	}

	SceneAllocator<U8> getSceneAllocator() const;
	/// @}

	/// @name Accessors of components
	/// @{
	virtual Movable* getMovable()
	{
		return nullptr;
	}

	virtual Renderable* getRenderable()
	{
		return nullptr;
	}

	virtual Frustumable* getFrustumable()
	{
		return nullptr;
	}

	virtual Spatial* getSpatial()
	{
		return nullptr;
	}

	virtual Light* getLight()
	{
		return nullptr;
	}

	virtual RigidBody* getRigidBody()
	{
		return nullptr;
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

private:
	SceneString name; ///< A unique name
	Scene* scene = nullptr; ///< Keep it here for unregistering
};
/// @}

} // end namespace anki

#endif
