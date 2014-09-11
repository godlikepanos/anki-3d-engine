// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_SCENE_GRAPH_H
#define ANKI_SCENE_SCENE_GRAPH_H

#include "anki/scene/Common.h"
#include "anki/scene/SceneNode.h"
#include "anki/scene/Visibility.h"
#include "anki/core/Timestamp.h"
#include "anki/Math.h"
#include "anki/util/Singleton.h"
#include "anki/util/HighRezTimer.h"
#include "anki/core/App.h"

#include "anki/scene/Sector.h"
#include "anki/physics/PhysicsWorld.h"
#include "anki/event/EventManager.h"

namespace anki {

// Forward
class Renderer;
class ResourceManager;
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
	SceneGraph(AllocAlignedCallback allocCb, void* allocCbData, 
		Threadpool* threadpool);

	~SceneGraph();
	/// @}

	void load(const char* filename);

	/// @note Return a copy
	SceneAllocator<U8> getAllocator() const
	{
		return m_alloc;
	}

	/// @note Return a copy
	SceneFrameAllocator<U8> getFrameAllocator() const
	{
		return m_frameAlloc;
	}

	Vec4 getAmbientColor() const
	{
		return Vec4(m_ambientCol, 1.0);
	}
	void setAmbientColor(const Vec4& x)
	{
		m_ambientCol = x.xyz();
		m_ambiendColorUpdateTimestamp = getGlobTimestamp();
	}
	U32 getAmbientColorUpdateTimestamp() const
	{
		return m_ambiendColorUpdateTimestamp;
	}

	Camera& getActiveCamera()
	{
		ANKI_ASSERT(m_mainCam != nullptr);
		return *m_mainCam;
	}
	const Camera& getActiveCamera() const
	{
		return *m_mainCam;
	}
	void setActiveCamera(Camera* cam)
	{
		m_mainCam = cam;
		m_activeCameraChangeTimestamp = getGlobTimestamp();
	}
	U32 getActiveCameraChangeTimestamp() const
	{
		return m_activeCameraChangeTimestamp;
	}

	U32 getSceneNodesCount() const
	{
		return m_nodes.size();
	}

	PhysicsWorld& getPhysics()
	{
		return m_physics;
	}
	const PhysicsWorld& getPhysics() const
	{
		return m_physics;
	}

	EventManager& getEventManager()
	{
		return m_events;
	}
	const EventManager& getEventManager() const
	{
		return m_events;
	}

	SectorGroup& getSectorGroup()
	{
		return m_sectorGroup;
	}
	const SectorGroup& getSectorGroup() const
	{
		return m_sectorGroup;
	}

	Threadpool& _getThreadpool()
	{
		return *m_threadpool;
	}

	void update(F32 prevUpdateTime, F32 crntTime, Renderer& renderer);

	SceneNode& findSceneNode(const char* name);
	SceneNode* tryFindSceneNode(const char* name);

	/// Iterate the scene nodes using a lambda
	template<typename Func>
	void iterateSceneNodes(Func func)
	{
		for(SceneNode* psn : m_nodes)
		{
			func(*psn);
		}
	}

	/// Iterate a range of scene nodes using a lambda
	template<typename Func>
	void iterateSceneNodes(PtrSize begin, PtrSize count, Func func)
	{
		ANKI_ASSERT(begin < m_nodes.size() && count <= m_nodes.size());
		for(auto it = m_nodes.begin() + begin; 
			it != m_nodes.begin() + count; 
			it++)
		{
			func(*(*it));
		}
	}

	/// Create a new SceneNode
	template<typename Node, typename... Args>
	Node* newSceneNode(const char* name, Args&&... args)
	{
		Node* node;
		SceneAllocator<Node> al = m_alloc;
		node = al.template newInstance<Node>(
			name, this, std::forward<Args>(args)...);
		registerNode(node);
		return node;
	}

	/// Delete a scene node. It actualy marks it for deletion
	void deleteSceneNode(SceneNode* node)
	{
		node->markForDeletion();
	}
	void increaseObjectsMarkedForDeletion()
	{
		++m_objectsMarkedForDeletionCount;
	}
	void decreaseObjectsMarkedForDeletion()
	{
		++m_objectsMarkedForDeletionCount;
	}

	/// @privatesection
	/// @{
	ResourceManager& _getResourceManager()
	{
		return *m_resources;
	}

	GlDevice& _getGlDevice();
	/// @}

private:
	Threadpool* m_threadpool = nullptr;
	ResourceManager* m_resources = nullptr;

	SceneAllocator<U8> m_alloc;
	SceneAllocator<U8> m_frameAlloc;

	SceneVector<SceneNode*> m_nodes;
	SceneDictionary<SceneNode*> m_dict;

	Vec3 m_ambientCol = Vec3(1.0); ///< The global ambient color
	Timestamp m_ambiendColorUpdateTimestamp = getGlobTimestamp();
	Camera* m_mainCam = nullptr;
	Timestamp m_activeCameraChangeTimestamp = getGlobTimestamp();

	PhysicsWorld m_physics;

	SectorGroup m_sectorGroup;

	EventManager m_events;

	AtomicU32 m_objectsMarkedForDeletionCount;

	/// Put a node in the appropriate containers
	void registerNode(SceneNode* node);
	void unregisterNode(SceneNode* node);

	/// Delete the nodes that are marked for deletion
	void deleteNodesMarkedForDeletion();
};

/// @}

} // end namespace anki

#endif
