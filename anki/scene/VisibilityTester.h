#ifndef ANKI_SCENE_VISIBILITY_TESTER_H
#define ANKI_SCENE_VISIBILITY_TESTER_H

#include <vector>
#include <boost/range/iterator_range.hpp>


namespace anki {


class Camera;
class Scene;
class SceneNode;
class Frustumable;


/// XXX
class VisibilityInfo
{
	friend class VisibilityTester;

public:
	typedef std::vector<SceneNode*> SceneNodes;

	boost::iterator_range<SceneNodes::iterator> getNodes()
	{
		return boost::iterator_range<SceneNodes::iterator>(
			nodes.begin(), nodes.end());
	}

private:
	SceneNodes nodes;
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
