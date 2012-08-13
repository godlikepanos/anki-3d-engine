#ifndef ANKI_SCENE_VISIBILITY_TESTER_H
#define ANKI_SCENE_VISIBILITY_TESTER_H

#include "anki/util/Vector.h"
#include <cstdint>

namespace anki {

class Camera;
class Scene;
class SceneNode;
class Frustumable;
class Renderable;
class Light;

/// @addtogroup Scene
/// @{

/// Its actually a container for visible entities
class VisibilityInfo
{
	friend class VisibilityTester;
	friend class Octree;

public:
	typedef Vector<Renderable*> Renderables;
	typedef Vector<Light*> Lights;

	Renderables::iterator getRenderablesBegin()
	{
		return renderables.begin();
	}
	Renderables::iterator getRenderablesEnd()
	{
		return renderables.end();
	}
	uint32_t getRenderablesCount() const
	{
		return renderables.size();
	}

	Lights::iterator getLightsBegin()
	{
		return lights.begin();
	}
	Lights::iterator getLightsEnd()
	{
		return lights.end();
	}

private:
	Renderables renderables;
	Lights lights;
};

/// Performs visibility determination tests and fills a few containers with the
/// visible scene nodes
class VisibilityTester
{
public:
	/// Constructor
	VisibilityTester()
	{}

	~VisibilityTester();

	/// This method:
	/// - Gets the visible renderables and frustumables
	/// - For every frustumable perform tests
	static void test(Frustumable& ref, Scene& scene);

private:
	static void testLight(Light& light, Scene& scene);
};
/// @}

} // end namespace anki

#endif
