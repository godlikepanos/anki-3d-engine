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
#include "anki/physics/PhysicsWorld.h"
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

	PhysicsWorld& getPhysics()
	{
		return physics;
	}
	const PhysicsWorld& getPhysics() const
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

	/// Iterate a range of scene nodes using a lambda
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
	void newSceneNode(Node*& node, const char* name, Args&&... args)
	{
		SceneAllocator<Node> al = alloc;
		node = al.allocate(1);
		al.construct(node, name, this, std::forward<Args>(args)...);

		registerNode(node);
	}

	/// Delete a scene node. It actualy marks it for deletion
	void deleteSceneNode(SceneNode* node)
	{
		node->markForDeletion();
	}
	void increaseObjectsMarkedForDeletion()
	{
		++objectsMarkedForDeletionCount;
	}
	void decreaseObjectsMarkedForDeletion()
	{
		++objectsMarkedForDeletionCount;
	}

private:
	SceneAllocator<U8> alloc;
	SceneAllocator<U8> frameAlloc;

	SceneVector<SceneNode*> nodes;
	SceneDictionary<SceneNode*> dict;

	Vec3 ambientCol = Vec3(1.0); ///< The global ambient color
	Timestamp ambiendColorUpdateTimestamp = getGlobTimestamp();
	Camera* mainCam = nullptr;
	Timestamp activeCameraChangeTimestamp = getGlobTimestamp();

	PhysicsWorld physics;

	SectorGroup sectorGroup;

	EventManager events;

	std::atomic<U32> objectsMarkedForDeletionCount;

	/// Put a node in the appropriate containers
	void registerNode(SceneNode* node);
	void unregisterNode(SceneNode* node);

	/// Delete the nodes that are marked for deletion
	void deleteNodesMarkedForDeletion();
};

typedef Singleton<SceneGraph> SceneGraphSingleton;
/// @}

} // end namespace anki

#endif
