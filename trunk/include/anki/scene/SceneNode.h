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
class Light;


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
	/// @}

	/// This is called by the scene every frame after logic and before
	/// rendering. By default it does nothing
	/// @param[in] prevUpdateTime Timestamp of the previous update
	/// @param[in] crntTime Timestamp of this update
	/// @param[in] frame Frame number
	virtual void frameUpdate(float prevUpdateTime, float crntTime, int frame)
	{
		(void)prevUpdateTime;
		(void)crntTime;
		(void)frame;
	}

	uint32_t getLastUpdateFrame();

private:
	std::string name; ///< A unique name
	Scene* scene; ///< Keep it here for unregistering
};
/// @}


} // end namespace anki


#endif
