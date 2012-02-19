#ifndef ANKI_SCENE_VISIBILITY_TESTER_H
#define ANKI_SCENE_VISIBILITY_TESTER_H

#include <vector>
#include <boost/range/iterator_range.hpp>


namespace anki {


class Camera;
class Scene;
class Renderable;
class Frustumable;


/// XXX
class VisibilityInfo
{
	friend class VisibilityTester;

public:
	typedef std::vector<Renderable*> Renderables;
	typedef std::vector<Frustumable*> Frustumables;

	boost::iterator_range<Renderables::iterator> getRenderables()
	{
		return boost::iterator_range<Renderables::iterator>(
			renderables.begin(), renderables.end());
	}

	boost::iterator_range<Frustumables::iterator> getFrustumable()
	{
		return boost::iterator_range<Frustumables::iterator>(
			frustumables.begin(), frustumables.end());
	}

private:
	Renderables renderables;
	std::vector<Frustumable*> frustumables;
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
	void test(Frustumable& cam, Scene& scene, VisibilityInfo& vinfo);
};


} // end namespace


#endif
