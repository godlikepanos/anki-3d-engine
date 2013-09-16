#ifndef ANKI_SCENE_SCENE_GRAPH_H
#define ANKI_SCENE_SCENE_GRAPH_H

#include "anki/scene/Common.h"
#include "anki/scene/SceneNode.h"
#include "anki/scene/Visibility.h"
#include "anki/core/Timestamp.h"
#include "anki/Math.h"
#include "anki/util/Singleton.h"
#include "anki/util/HighRezTimer.h"

#include "anki/scene/Sector.h"
#include "anki/physics/PhysWorld.h"
#include "anki/event/EventManager.h"

namespace anki {

// Forward
class Renderer;
class Camera;

/// @addtogroup Scene
/// @{

/// The scene graph that  all the scene entities
class SceneGraph
{
	friend class SceneNode;

public:
	template<typename T>
	struct Types
	{
		typedef SceneVector<T*> Container;
		typedef typename Container::iterator Iterator;
		typedef typename Container::const_iterator ConstIterator;
		// XXX Change the map with map with allocator
		typedef typename ConstCharPtrHashMap<T*>::Type NameToItemMap;
	};

	/// @name Constructors/Destructor
	/// @{
	SceneGraph();
	~SceneGraph();
	/// @}

	void load(const char* filename);

	/// @name Accessors
	/// @{

	/// @note Return a copy
	SceneAllocator<U8> getAllocator() const
	{
		return SceneAllocator<U8>(alloc);
	}

	/// @note Return a copy
	SceneAllocator<U8> getFrameAllocator() const
	{
		return SceneAllocator<U8>(frameAlloc);
	}

	Vec4 getAmbientColor() const
	{
		return Vec4(ambientCol, 1.0);
	}
	void setAmbientColor(const Vec4& x)
	{
		ambientCol = x.xyz();
		ambiendColorUpdateTimestamp = getGlobTimestamp();
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
		activeCameraChangeTimestamp = getGlobTimestamp();
	}
	U32 getActiveCameraChangeTimestamp() const
	{
		return activeCameraChangeTimestamp;
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

	EventManager& getEventManager()
	{
		return events;
	}
	const EventManager& getEventManager() const
	{
		return events;
	}

	SectorGroup& getSectorGroup()
	{
		return sectorGroup;
	}
	const SectorGroup& getSectorGroup() const
	{
		return sectorGroup;
	}
	/// @}

	void update(F32 prevUpdateTime, F32 crntTime, Renderer& renderer);

	SceneNode& findSceneNode(const char* name);
	SceneNode* tryFindSceneNode(const char* name);

	/// Iterate the scene nodes using a lambda
	template<typename Func>
	void iterateSceneNodes(Func func)
	{
		for(SceneNode* psn : nodes)
		{
			func(*psn);
		}
	}

	/// Iterate the scene nodes using a lambda
	template<typename Func>
	void iterateSceneNodes(PtrSize begin, PtrSize count, Func func)
	{
		ANKI_ASSERT(begin < nodes.size() && count <= nodes.size());
		for(auto it = nodes.begin() + begin; it != nodes.begin() + count; it++)
		{
			func(*(*it));
		}
	}

	/// Create a new SceneNode
	template<typename Node, typename... Args>
	void newSceneNode(Node*& node, Args&&... args)
	{
		SceneAllocator<Node> al = alloc;
		node = al.allocate(1);
		al.construct(node, std::forward<Args>(args)...);
	}

private:
	SceneAllocator<U8> alloc;
	SceneAllocator<U8> frameAlloc;

	Types<SceneNode>::Container nodes;
	Types<SceneNode>::NameToItemMap nameToNode;

	Vec3 ambientCol = Vec3(1.0); ///< The global ambient color
	Timestamp ambiendColorUpdateTimestamp = getGlobTimestamp();
	Camera* mainCam = nullptr;
	Timestamp activeCameraChangeTimestamp = getGlobTimestamp();

	PhysWorld physics;

	SectorGroup sectorGroup;

	EventManager events;

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
		ANKI_ASSERT(ptr && ptr->getName());

		if(d.find(ptr->getName()) != d.end())
		{
			throw ANKI_EXCEPTION("Node with the same name already exists: "
				+ ptr->getName());
		}

		d[ptr->getName()] = ptr;
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
			d.find(ptr->getName());

		ANKI_ASSERT(it != d.end());
		d.erase(it);
	}
};

typedef Singleton<SceneGraph> SceneGraphSingleton;
/// @}

} // end namespace anki

#endif
