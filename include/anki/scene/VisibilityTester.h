#ifndef ANKI_SCENE_VISIBILITY_TESTER_H
#define ANKI_SCENE_VISIBILITY_TESTER_H

#include <vector>

namespace anki {

class Camera;
class Scene;
class SceneNode;
class Frustumable;
class Renderable;
class Light;

/// Its actually a container for visible entities
class VisibilityInfo
{
	friend class VisibilityTester;

public:
	typedef std::vector<Renderable*> Renderables;
	typedef std::vector<Light*> Lights;

	Renderables::iterator getRenderablesBegin()
	{
		return renderables.begin();
	}
	Renderables::iterator getRenderablesEnd()
	{
		return renderables.end();
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
	static void test(Frustumable& cam, Scene& scene, VisibilityInfo& vinfo);
};

} // end namespace

#endif
