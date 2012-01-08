#ifndef ANKI_SCENE_SCENE_H
#define ANKI_SCENE_SCENE_H

#include "anki/scene/SceneNode.h"
#include <boost/range/iterator_range.hpp>


namespace anki {


/// The Scene contains all the dynamic entities
class Scene
{
public:
	template<typename T>
	struct Types
	{
		typedef std::vector<T*> Container;
		typedef typename Container::iterator Iterator;
		typedef typename Container::const_iterator ConstIterator;
		typedef boost::iterator_range<Iterator> MutableRange;
		typedef boost::iterator_range<ConstIterator> ConstRange;
	};

	Scene();
	~Scene();

	/// Put a node in the appropriate containers
	void registerNode(SceneNode* node);
	void unregisterNode(SceneNode* node);

	void update(float prevUpdateTime, float crntTime);

	void doVisibilityTests(Camera& cam)
	{
		//XXX visibilityTester->test(cam);
	}

	/// @name Accessors
	/// @{
	const Vec3& getAmbientColor() const
	{
		return ambientCol;
	}
	Vec3& getAmbientColor()
	{
		return ambientCol;
	}
	void setAmbientColor(const Vec3& x)
	{
		ambientCol = x;
	}

	/*PhysWorld& getPhysWorld()
	{
		return *physPhysWorld;
	}
	const PhysWorld& getPhysWorld() const
	{
		return *physPhysWorld;
	}

	const VisibilityTester& getVisibilityTester() const
	{
		return *visibilityTester;
	}*/

	Types<SceneNode>::ConstRange getAllNodes() const
	{
		return Types<SceneNode>::ConstRange(nodes.begin(), nodes.end());
	}
	Types<SceneNode>::MutableRange getAllNodes()
	{
		return Types<SceneNode>::MutableRange(nodes.begin(), nodes.end());
	}
	/// @}

private:
	/// @name Containers of scene's data
	/// @{
	Types<SceneNode>::Container nodes;
	/// @}

	Vec3 ambientCol; ///< The global ambient color
	/// Connection with Bullet wrapper
	/*boost::scoped_ptr<PhysWorld> physPhysWorld;
	boost::scoped_ptr<VisibilityTester> visibilityTester;*/
};


} // end namespace


#endif
