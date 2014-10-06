// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/SceneGraph.h"
#include "anki/scene/Camera.h"
#include "anki/scene/ModelNode.h"
#include "anki/scene/InstanceNode.h"
#include "anki/util/Exception.h"
#include "anki/core/Counters.h"
#include "anki/renderer/Renderer.h"
#include "anki/misc/Xml.h"

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

namespace {

//==============================================================================
class UpdateSceneNodesTask: public Threadpool::Task
{
public:
	SceneGraph* m_scene = nullptr;
	F32 m_prevUpdateTime;
	F32 m_crntTime;

	void operator()(U32 taskId, PtrSize threadsCount)
	{
		PtrSize start, end;
		choseStartEnd(
			taskId, threadsCount, m_scene->getSceneNodesCount(), start, end);

		// Start with the root nodes
		m_scene->iterateSceneNodes(start, end, [&](SceneNode& node)
		{
			if(node.getParent() == nullptr)
			{
				updateInternal(node, m_prevUpdateTime, m_crntTime);
			}
		});
	}

	void updateInternal(SceneNode& node, F32 prevTime, F32 crntTime)
	{
		// Components update
		node.iterateComponents([&](SceneComponent& comp)
		{
			comp.updateReal(node, prevTime, crntTime);
		});

		// Update children
		node.visitChildren([&](SceneObject& obj)
		{
			if(obj.getType() == SceneObject::Type::SCENE_NODE)
			{
				SceneNode& child = obj.downCast<SceneNode>();
				updateInternal(child, prevTime, crntTime);
			}
		});

		// Frame update
		node.frameUpdate(prevTime, crntTime);
	}
};

} // end namespace anonymous

//==============================================================================
// Scene                                                                       =
//==============================================================================

//==============================================================================
SceneGraph::SceneGraph(AllocAlignedCallback allocCb, void* allocCbData, 
	Threadpool* threadpool, ResourceManager* resources)
:	m_threadpool(threadpool),
	m_resources(resources),
	m_gl(&m_resources->_getGlDevice()),
	m_alloc(allocCb, allocCbData, ANKI_SCENE_ALLOCATOR_SIZE),
	m_frameAlloc(allocCb, allocCbData, ANKI_SCENE_FRAME_ALLOCATOR_SIZE),
	m_nodes(m_alloc),
	m_dict(10, DictionaryHasher(), DictionaryEqual(), m_alloc),
	m_physics(),
	m_sectorGroup(this),
	m_events(this)
{
	m_nodes.reserve(ANKI_SCENE_OPTIMAL_SCENE_NODES_COUNT);

	m_objectsMarkedForDeletionCount.store(0);

	m_ambientCol = Vec3(0.0);
}

//==============================================================================
SceneGraph::~SceneGraph()
{}

//==============================================================================
void SceneGraph::registerNode(SceneNode* node)
{
	ANKI_ASSERT(node);

	// Add to dict if it has name
	if(node->getName())
	{
		if(m_dict.find(node->getName()) != m_dict.end())
		{
			throw ANKI_EXCEPTION("Node with the same name already exists");
		}

		m_dict[node->getName()] = node;
	}

	// Add to vector
	ANKI_ASSERT(
		std::find(m_nodes.begin(), m_nodes.end(), node) == m_nodes.end());
	m_nodes.push_back(node);
}

//==============================================================================
void SceneGraph::unregisterNode(SceneNode* node)
{
	// Remove from vector
	auto it = m_nodes.begin();
	for(; it != m_nodes.end(); it++)
	{
		if((*it) == node)
		{
			break;
		}
	}
	
	ANKI_ASSERT(it != m_nodes.end());
	m_nodes.erase(it);

	// Remove from dict
	if(node->getName())
	{
		auto it = m_dict.find(node->getName());

		ANKI_ASSERT(it != m_dict.end());
		m_dict.erase(it);
	}
}

//==============================================================================
SceneNode& SceneGraph::findSceneNode(const char* name)
{
	ANKI_ASSERT(m_dict.find(name) != m_dict.end());
	return *(m_dict.find(name))->second;
}

//==============================================================================
SceneNode* SceneGraph::tryFindSceneNode(const char* name)
{
	auto it = m_dict.find(name);
	return (it == m_dict.end()) ? nullptr : it->second;
}

//==============================================================================
void SceneGraph::deleteNodesMarkedForDeletion()
{
	SceneAllocator<SceneNode> al = m_alloc;

	/// Delete all nodes pending deletion. At this point all scene threads 
	/// should have finished their tasks
	while(m_objectsMarkedForDeletionCount > 0)
	{
		// First gather the nodes that will be deleted
		SceneFrameVector<decltype(m_nodes)::iterator> forDeletion;
		for(auto it = m_nodes.begin(); it != m_nodes.end(); it++)
		{
			if((*it)->isMarkedForDeletion())
			{
				forDeletion.push_back(it);
			}
		}

		// Now delete nodes
		for(auto& it : forDeletion)
		{
			// Remove it
			unregisterNode(*it);

			m_alloc.deleteInstance(*it);
		}

		// Do the same for events
		m_events.deleteEventsMarkedForDeletion();
	}
}

//==============================================================================
void SceneGraph::update(F32 prevUpdateTime, F32 crntTime, Renderer& renderer)
{
	ANKI_ASSERT(m_mainCam);

	ANKI_COUNTER_START_TIMER(SCENE_UPDATE_TIME);

	//
	// Sync point. Here we wait for all scene's threads
	//

	// Reset the framepool
	m_frameAlloc.getMemoryPool().reset();

	// Delete nodes
	deleteNodesMarkedForDeletion();

	Threadpool& threadPool = *m_threadpool;
	(void)threadPool;

	// XXX Do that in parallel
	//m_physics.update(prevUpdateTime, crntTime);
	renderer.getTiler().updateTiles(*m_mainCam);
	m_events.updateAllEvents(prevUpdateTime, crntTime);

	// Then the rest
	Array<UpdateSceneNodesTask, Threadpool::MAX_THREADS> jobs2;

	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		UpdateSceneNodesTask& job = jobs2[i];

		job.m_scene = this;
		job.m_prevUpdateTime = prevUpdateTime;
		job.m_crntTime = crntTime;

		threadPool.assignNewTask(i, &job);
	}

	threadPool.waitForAllThreadsToFinish();

	doVisibilityTests(*m_mainCam, *this, renderer);

	ANKI_COUNTER_STOP_TIMER_INC(SCENE_UPDATE_TIME);
}

} // end namespace anki
