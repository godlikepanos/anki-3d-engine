#ifndef ANKI_SCENE_VISIBILITY_TESTER_H
#define ANKI_SCENE_VISIBILITY_TESTER_H

#include <vector>
#include <cstdint>

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
	enum Test
	{
		T_NONE_BIT = 0
		T_RENDERABLES_BIT = 1,
		T_LIGHTS_BIT = 2,
		T_CAMERAS_BIT = 4,
		T_ALL_BIT = T_RENDERABLES_BIT | T_LIGHTS_BIT | T_CAMERAS_BIT
	};

	/// Constructor
	VisibilityTester()
	{}

	~VisibilityTester();

	/// This method:
	/// - Gets the visible renderables and frustumables
	/// - For every frustumable perform tests
	static void test(Frustumable& cam, Scene& scene, VisibilityInfo& vinfo,
		uint32_t testMask = T_ALL_BIT);
};

} // end namespace anki

#endif
