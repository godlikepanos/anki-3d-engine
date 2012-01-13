#ifndef ANKI_SCENE_SCENE_NODE_H
#define ANKI_SCENE_SCENE_NODE_H

#include <string>


namespace anki {


class Scene; // Don't include

class Movable;
class Renderable;
class Frustumable;
class Spartial;


/// @addtogroup scene
/// @{

/// Interface class backbone of scene
class SceneNode
{
public:
	/// The one and only constructor
	/// @param name The unique name of the node
	/// @param scene The scene that will register it
	explicit SceneNode(
		const char* name,
		Scene* scene);

	/// Unregister node
	virtual ~SceneNode();

	/// @name Accessors of properties
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

	virtual Spartial* getSpartial()
	{
		return NULL;
	}
	/// @}

	/// This is called by the scene every frame
	virtual void frameUpdate(float prevUpdateTime, float crntTime)
	{
		(void)prevUpdateTime;
		(void)crntTime;
	}

private:
	std::string name; ///< A unique name
	Scene* scene; ///< For registering and unregistering
};
/// @}


} // end namespace


#endif
