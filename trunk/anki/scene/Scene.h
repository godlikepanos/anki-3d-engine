#ifndef ANKI_SCENE_SCENE_H
#define ANKI_SCENE_SCENE_H

#include "anki/scene/SceneNode.h"
#include <boost/range/iterator_range.hpp>
#include <vector>


namespace anki {


/// The Scene contains all the dynamic entities
///
/// XXX Add physics
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

	typedef ConstCharPtrHashMap<SceneNode*>::Type NameToSceneNodeMap;

	/// @name Constructors/Destructor
	/// @{
	Scene();
	~Scene();
	/// @}

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

	Types<SceneNode>::ConstRange getAllNodes() const
	{
		return Types<SceneNode>::ConstRange(nodes.begin(), nodes.end());
	}
	Types<SceneNode>::MutableRange getAllNodes()
	{
		return Types<SceneNode>::MutableRange(nodes.begin(), nodes.end());
	}
	/// @}

	bool nodeExists(const char* name) const
	{
		return nameToNode.find(name) != nameToNode.end();
	}

private:
	/// @name Containers of scene's data
	/// @{
	Types<SceneNode>::Container nodes;
	Types<Movable>::Container movables;
	Types<Renderable>::Container renderables;
	Types<Spatial>::Container spatials;
	Types<Frustumable>::Container frustumables;
	/// @}

	NameToSceneNodeMap nameToNode;

	Vec3 ambientCol; ///< The global ambient color
};


} // end namespace


#endif
