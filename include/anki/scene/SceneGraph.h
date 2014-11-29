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

#include "anki/physics/PhysicsWorld.h"
#include "anki/event/EventManager.h"

namespace anki {

// Forward
class Renderer;
class ResourceManager;
class Camera;

/// @addtogroup scene
/// @{

/// The scene graph that  all the scene entities
class SceneGraph
{
	friend class SceneNode;

public:
	SceneGraph();

	~SceneGraph();

	ANKI_USE_RESULT Error create(
		AllocAlignedCallback allocCb, 
		void* allocCbData, 
		Threadpool* threadpool, 
		ResourceManager* resources);

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
		return m_nodesCount;
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

	Threadpool& _getThreadpool()
	{
		return *m_threadpool;
	}

	ANKI_USE_RESULT Error update(
		F32 prevUpdateTime, F32 crntTime, Renderer& renderer);

	SceneNode& findSceneNode(const CString& name);
	SceneNode* tryFindSceneNode(const CString& name);

	/// Iterate the scene nodes using a lambda
	template<typename Func>
	ANKI_USE_RESULT Error iterateSceneNodes(Func func)
	{
		for(SceneNode* psn : m_nodes)
		{
			Error err = func(*psn);
			if(err)
			{
				return err;
			}
		}

		return ErrorCode::NONE;
	}

	/// Iterate a range of scene nodes using a lambda
	template<typename Func>
	ANKI_USE_RESULT Error iterateSceneNodes(
		PtrSize begin, PtrSize end, Func func);

	/// Create a new SceneNode
	template<typename Node, typename... Args>
	ANKI_USE_RESULT Error newSceneNode(
		const CString& name, Node*& node, Args&&... args);

	/// Delete a scene node. It actualy marks it for deletion
	void deleteSceneNode(SceneNode* node)
	{
		node->setMarkedForDeletion();
	}

	void increaseObjectsMarkedForDeletion()
	{
		++m_objectsMarkedForDeletionCount;
	}

	/// @privatesection
	/// @{
	ResourceManager& _getResourceManager()
	{
		return *m_resources;
	}

	GlDevice& _getGlDevice()
	{
		return *m_gl;
	}
	/// @}

private:
	Threadpool* m_threadpool = nullptr;
	ResourceManager* m_resources = nullptr;
	GlDevice* m_gl = nullptr;

	SceneAllocator<U8> m_alloc;
	SceneFrameAllocator<U8> m_frameAlloc;

	List<SceneNode*, SceneAllocator<SceneNode*>> m_nodes;
	U32 m_nodesCount = 0;
	//SceneDictionary<SceneNode*> m_dict;

	Vec3 m_ambientCol = Vec3(1.0); ///< The global ambient color
	Timestamp m_ambiendColorUpdateTimestamp = getGlobTimestamp();
	Camera* m_mainCam = nullptr;
	Timestamp m_activeCameraChangeTimestamp = getGlobTimestamp();

	PhysicsWorld m_physics;

	EventManager m_events;

	AtomicU32 m_objectsMarkedForDeletionCount;

	/// Put a node in the appropriate containers
	ANKI_USE_RESULT Error registerNode(SceneNode* node);
	void unregisterNode(SceneNode* node);

	/// Delete the nodes that are marked for deletion
	void deleteNodesMarkedForDeletion();
};

//==============================================================================
template<typename Node, typename... Args>
inline Error SceneGraph::newSceneNode(
	const CString& name, Node*& node, Args&&... args)
{
	Error err = ErrorCode::NONE;
	SceneAllocator<Node> al = m_alloc;

	node = al.template newInstance<Node>(this);
	if(node)
	{
		err = node->create(name, std::forward<Args>(args)...);
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
		ANKI_LOGE("Failed to create scene node: %s", 
			(name.isEmpty()) ? "unnamed" : &name[0]);

		if(node)
		{
			al.deleteInstance(node);
			node = nullptr;
		}
	}

	return err;
}

//==============================================================================
template<typename Func>
Error SceneGraph::iterateSceneNodes(PtrSize begin, PtrSize end, Func func)
{
	ANKI_ASSERT(begin < m_nodesCount && end <= m_nodesCount);
	auto it = m_nodes.getBegin() + begin;
	
	PtrSize count = end - begin;
	while(count-- != 0)
	{
		ANKI_ASSERT(it != m_nodes.getEnd());
		Error err = func(*(*it));
		if(err)
		{
			return err;
		}

		++it;
	}

	return ErrorCode::NONE;
}
/// @}

} // end namespace anki

#endif
