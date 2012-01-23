#ifndef ANKI_SCENE_SCENE_NODE_H
#define ANKI_SCENE_SCENE_NODE_H

#include "anki/scene/Property.h"
#include <string>


namespace anki {


class Scene; // Don't include

class Movable;
class Renderable;
class Frustumable;
class Spatial;


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
	const std::string& getName() const
	{
		return name;
	}
	/// @}

	/// @name Accessors of components
	/// @{
	virtual Movable* getMovable()
	{
		return NULL;
	}

	virtual Renderable* getRenderable()
	{
		return NULL;
	}

	virtual Frustumable* getFrustumable()
	{
		return NULL;
	}

	virtual Spatial* getSpatial()
	{
		return NULL;
	}
	/// @}

	/// This is called by the scene every frame. But default it does nothing
	virtual void frameUpdate(float prevUpdateTime, float crntTime)
	{
		(void)prevUpdateTime;
		(void)crntTime;
	}

private:
	std::string name; ///< A unique name
	Scene* scene; ///< Keep it here for unregistering
};
/// @}


} // end namespace


#endif
