// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/Common.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/Visibility.h>
#include <anki/core/Timestamp.h>
#include <anki/Math.h>
#include <anki/util/Singleton.h>
#include <anki/util/HighRezTimer.h>
#include <anki/util/HashMap.h>
#include <anki/core/App.h>
#include <anki/event/EventManager.h>

namespace anki
{

// Forward
class MainRenderer;
class ResourceManager;
class Camera;
class Input;
class SectorGroup;
class ConfigSet;
class PerspectiveCamera;
class UpdateSceneNodesCtx;

/// @addtogroup scene
/// @{

/// The scene graph that  all the scene entities
class SceneGraph
{
	friend class SceneNode;
	friend class UpdateSceneNodesTask;

public:
	SceneGraph();

	~SceneGraph();

	ANKI_USE_RESULT Error init(AllocAlignedCallback allocCb,
		void* allocCbData,
		ThreadPool* threadpool,
		ThreadHive* thraedHive,
		ResourceManager* resources,
		Input* input,
		const Timestamp* globalTimestamp,
		const ConfigSet& config);

	Timestamp getGlobalTimestamp() const
	{
		return m_timestamp;
	}

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

	SceneNode& getActiveCamera()
	{
		ANKI_ASSERT(m_mainCam != nullptr);
		return *m_mainCam;
	}
	const SceneNode& getActiveCamera() const
	{
		return *m_mainCam;
	}
	void setActiveCamera(SceneNode* cam)
	{
		m_mainCam = cam;
		m_activeCameraChangeTimestamp = getGlobalTimestamp();
	}
	U32 getActiveCameraChangeTimestamp() const
	{
		return m_activeCameraChangeTimestamp;
	}

	U32 getSceneNodesCount() const
	{
		return m_nodesCount;
	}

	EventManager& getEventManager()
	{
		return m_events;
	}
	const EventManager& getEventManager() const
	{
		return m_events;
	}

	ThreadPool& _getThreadPool()
	{
		return *m_threadpool;
	}

	ThreadHive& getThreadHive()
	{
		return *m_threadHive;
	}

	ANKI_USE_RESULT Error update(F32 prevUpdateTime, F32 crntTime, MainRenderer& renderer);

	SceneNode& findSceneNode(const CString& name);
	SceneNode* tryFindSceneNode(const CString& name);

	/// Iterate the scene nodes using a lambda
	template<typename Func>
	ANKI_USE_RESULT Error iterateSceneNodes(Func func)
	{
		for(SceneNode& psn : m_nodes)
		{
			Error err = func(psn);
			if(err)
			{
				return err;
			}
		}

		return ErrorCode::NONE;
	}

	/// Iterate a range of scene nodes using a lambda
	template<typename Func>
	ANKI_USE_RESULT Error iterateSceneNodes(PtrSize begin, PtrSize end, Func func);

	/// Create a new SceneNode
	template<typename Node, typename... Args>
	ANKI_USE_RESULT Error newSceneNode(const CString& name, Node*& node, Args&&... args);

	/// Delete a scene node. It actualy marks it for deletion
	void deleteSceneNode(SceneNode* node)
	{
		node->setMarkedForDeletion();
	}

	void increaseObjectsMarkedForDeletion()
	{
		m_objectsMarkedForDeletionCount.fetchAdd(1);
	}

anki_internal:
	ResourceManager& getResourceManager()
	{
		return *m_resources;
	}

	GrManager& getGrManager()
	{
		return *m_gr;
	}

	PhysicsWorld& getPhysicsWorld()
	{
		return *m_physics;
	}

	const PhysicsWorld& getPhysicsWorld() const
	{
		return *m_physics;
	}

	const Input& getInput() const
	{
		ANKI_ASSERT(m_input);
		return *m_input;
	}

	SectorGroup& getSectorGroup()
	{
		ANKI_ASSERT(m_sectors);
		return *m_sectors;
	}

	F32 getMaxReflectionProxyDistance() const
	{
		ANKI_ASSERT(m_maxReflectionProxyDistance > 0.0);
		return m_maxReflectionProxyDistance;
	}

	U64 getNewSceneNodeUuid()
	{
		return m_nodesUuid++;
	}

	SceneComponentLists& getSceneComponentLists()
	{
		return m_componentLists;
	}

private:
	const Timestamp* m_globalTimestamp = nullptr;
	Timestamp m_timestamp = 0; ///< Cached timestamp

	// Sub-systems
	ThreadPool* m_threadpool = nullptr;
	ThreadHive* m_threadHive = nullptr;
	ResourceManager* m_resources = nullptr;
	GrManager* m_gr = nullptr;
	PhysicsWorld* m_physics = nullptr;
	Input* m_input = nullptr;

	SceneAllocator<U8> m_alloc;
	SceneFrameAllocator<U8> m_frameAlloc;

	IntrusiveList<SceneNode> m_nodes;
	U32 m_nodesCount = 0;
	HashMap<CString, SceneNode*, CStringHasher, CStringCompare> m_nodesDict;

	SceneNode* m_mainCam = nullptr;
	Timestamp m_activeCameraChangeTimestamp = getGlobalTimestamp();
	PerspectiveCamera* m_defaultMainCam = nullptr;

	EventManager m_events;
	SectorGroup* m_sectors;

	Atomic<U32> m_objectsMarkedForDeletionCount;

	F32 m_maxReflectionProxyDistance = 0.0;

	U64 m_nodesUuid = 0;

	SceneComponentLists m_componentLists;

	/// Put a node in the appropriate containers
	ANKI_USE_RESULT Error registerNode(SceneNode* node);
	void unregisterNode(SceneNode* node);

	/// Delete the nodes that are marked for deletion
	void deleteNodesMarkedForDeletion();

	ANKI_USE_RESULT Error updateNodes(UpdateSceneNodesCtx& ctx) const;
	ANKI_USE_RESULT static Error updateNode(F32 prevTime, F32 crntTime, SceneNode& node);
};

template<typename Node, typename... Args>
inline Error SceneGraph::newSceneNode(const CString& name, Node*& node, Args&&... args)
{
	Error err = ErrorCode::NONE;
	SceneAllocator<Node> al = m_alloc;

	node = al.template newInstance<Node>(this, name);
	if(node)
	{
		err = node->init(std::forward<Args>(args)...);
	}
	else
	{
		err = ErrorCode::OUT_OF_MEMORY;
	}

	if(!err)
	{
		err = registerNode(node);
	}

	if(err)
	{
		ANKI_SCENE_LOGE("Failed to create scene node: %s", (name.isEmpty()) ? "unnamed" : &name[0]);

		if(node)
		{
			al.deleteInstance(node);
			node = nullptr;
		}
	}

	return err;
}

template<typename Func>
Error SceneGraph::iterateSceneNodes(PtrSize begin, PtrSize end, Func func)
{
	ANKI_ASSERT(begin < m_nodesCount && end <= m_nodesCount);
	auto it = m_nodes.getBegin() + begin;

	PtrSize count = end - begin;
	Error err = ErrorCode::NONE;
	while(count-- != 0 && !err)
	{
		ANKI_ASSERT(it != m_nodes.getEnd());
		err = func(*it);

		++it;
	}

	return ErrorCode::NONE;
}
/// @}

} // end namespace anki
