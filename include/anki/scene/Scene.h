#ifndef ANKI_SCENE_SCENE_H
#define ANKI_SCENE_SCENE_H

#include "anki/scene/SceneNode.h"
#include "anki/scene/VisibilityTester.h"
#include "anki/math/Math.h"
#include "anki/util/Singleton.h"
#include "anki/scene/Sector.h"
#include "anki/util/Vector.h"
#include "anki/core/Timestamp.h"
#include "anki/physics/PhysWorld.h"

namespace anki {

class Renderer;

/// @addtogroup Scene
/// @{

/// The Scene contains all the dynamic entities
class Scene
{
	friend class SceneNode;

public:
	template<typename T>
	struct Types
	{
		typedef Vector<T*> Container;
		typedef typename Container::iterator Iterator;
		typedef typename Container::const_iterator ConstIterator;
		typedef typename ConstCharPtrHashMap<T*>::Type NameToItemMap;
	};

	/// @name Constructors/Destructor
	/// @{
	Scene();
	~Scene();
	/// @}

	/// @name Accessors
	/// @{
	const Vec3& getAmbientColor() const
	{
		return ambientCol;
	}
	void setAmbientColor(const Vec3& x)
	{
		ambientCol = x;
		ambiendColorUpdateTimestamp = Timestamp::getTimestamp();
	}
	U32 getAmbientColorUpdateTimestamp() const
	{
		return ambiendColorUpdateTimestamp;
	}

	Camera& getActiveCamera()
	{
		ANKI_ASSERT(mainCam != nullptr);
		return *mainCam;
	}
	const Camera& getActiveCamera() const
	{
		return *mainCam;
	}
	void setActiveCamera(Camera* cam)
	{
		mainCam = cam;
		activeCameraChangeTimestamp = Timestamp::getTimestamp();
	}
	U32 getActiveCameraChangeTimestamp() const
	{
		return activeCameraChangeTimestamp;
	}

	Types<SceneNode>::ConstIterator getSceneNodesBegin() const
	{
		return nodes.begin();
	}
	Types<SceneNode>::Iterator getSceneNodesBegin()
	{
		return nodes.begin();
	}
	Types<SceneNode>::ConstIterator getSceneNodesEnd() const
	{
		return nodes.end();
	}
	Types<SceneNode>::Iterator getSceneNodesEnd()
	{
		return nodes.end();
	}
	U32 getSceneNodesCount() const
	{
		return nodes.size();
	}

	PhysWorld& getPhysics()
	{
		return physics;
	}
	const PhysWorld& getPhysics() const
	{
		return physics;
	}
	/// @}

	void update(float prevUpdateTime, float crntTime, Renderer& renderer);

	SceneNode& findSceneNode(const char* name);
	SceneNode* tryFindSceneNode(const char* name);

	PtrVector<Sector> sectors;

private:
	Vec3 ambientCol = Vec3(1.0); ///< The global ambient color
	U32 ambiendColorUpdateTimestamp = Timestamp::getTimestamp();
	Camera* mainCam = nullptr;
	U32 activeCameraChangeTimestamp = Timestamp::getTimestamp();

	Types<SceneNode>::Container nodes;
	Types<SceneNode>::NameToItemMap nameToNode;

	VisibilityTester vtester;
	PhysWorld physics;

	void doVisibilityTests(Camera& cam, Renderer& r);

	/// Put a node in the appropriate containers
	void registerNode(SceneNode* node);
	void unregisterNode(SceneNode* node);

	/// Add to a container
	template<typename T>
	void addC(typename Types<T>::Container& c, T* ptr)
	{
		ANKI_ASSERT(std::find(c.begin(), c.end(), ptr) == c.end());
		c.push_back(ptr);
	}

	/// Add to a dictionary
	template<typename T>
	void addDict(typename Types<T>::NameToItemMap& d, T* ptr)
	{
		ANKI_ASSERT(d.find(ptr->getName().c_str()) == d.end()
			&& "Item with same name already exists");

		d[ptr->getName().c_str()] = ptr;
	}

	/// Remove from a container
	template<typename T>
	void removeC(typename Types<T>::Container& c, T* ptr)
	{
		typename Types<T>::Iterator it = c.begin();
		for(; it != c.end(); ++it)
		{
			if(*it == ptr)
			{
				break;
			}
		}

		ANKI_ASSERT(it != c.end());
		c.erase(it);
	}

	/// Remove from a dictionary
	template<typename T>
	void removeDict(typename Types<T>::NameToItemMap& d, T* ptr)
	{
		typename Types<T>::NameToItemMap::iterator it =
			d.find(ptr->getName().c_str());

		ANKI_ASSERT(it != d.end());
		d.erase(it);
	}
};

typedef Singleton<Scene> SceneSingleton;
/// @}

} // end namespace anki

#endif
