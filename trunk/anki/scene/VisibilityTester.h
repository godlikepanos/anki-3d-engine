#ifndef ANKI_SCENE_VISIBILITY_TESTER_H
#define ANKI_SCENE_VISIBILITY_TESTER_H

#include <vector>
#include <boost/range/iterator_range.hpp>


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

	boost::iterator_range<Renderables::iterator> getRenderables()
	{
		return boost::iterator_range<Renderables::iterator>(
			renderables.begin(), renderables.end());
	}

	boost::iterator_range<Lights::iterator> getLights()
	{
		return boost::iterator_range<Lights::iterator>(
			lights.begin(), lights.end());
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
