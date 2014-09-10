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
struct UpdateSceneNodesJob: Threadpool::Task
{
	SceneGraph* scene = nullptr;
	F32 prevUpdateTime;
	F32 crntTime;
	Barrier* barrier = nullptr;

	void operator()(U32 taskId, PtrSize threadsCount)
	{
		ANKI_ASSERT(scene);
		PtrSize start, end;
		choseStartEnd(
			taskId, threadsCount, scene->getSceneNodesCount(), start, end);

		// First update the move components
		scene->iterateSceneNodes(start, end, [&](SceneNode& node)
		{
			MoveComponent* move = node.tryGetComponent<MoveComponent>();

			if(move)
			{
				move->updateReal(node, prevUpdateTime, crntTime,
					SceneComponent::ASYNC_UPDATE);
			}
		});

		barrier->wait();

		// Update the rest of the components
		auto moveComponentTypeId = MoveComponent::getClassType();
		scene->iterateSceneNodes(start, end, [&](SceneNode& node)
		{
			// Components update
			node.iterateComponents([&](SceneComponent& comp)
			{
				if(comp.getType() != moveComponentTypeId)
				{
					comp.updateReal(node, prevUpdateTime, crntTime, 
						SceneComponent::ASYNC_UPDATE);
				}
			});

			// Frame update
			node.frameUpdate(prevUpdateTime, crntTime, 
				SceneNode::ASYNC_UPDATE);
		});
	}
};

} // end namespace anonymous

//==============================================================================
// Scene                                                                       =
//==============================================================================

//==============================================================================
SceneGraph::SceneGraph(AllocAlignedCallback allocCb, void* allocCbData, 
	Threadpool* threadpool)
:	m_alloc(StackMemoryPool(allocCb, allocCbData, ANKI_SCENE_ALLOCATOR_SIZE)),
	m_frameAlloc(StackMemoryPool(allocCb, allocCbData, 
		ANKI_SCENE_FRAME_ALLOCATOR_SIZE)),
	m_nodes(m_alloc),
	m_dict(10, DictionaryHasher(), DictionaryEqual(), m_alloc),
	m_physics(),
	m_sectorGroup(this),
	m_events(this),
	m_threadpool(threadpool)
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

	// Sync updates
	iterateSceneNodes([&](SceneNode& node)
	{
		node.iterateComponents([&](SceneComponent& comp)
		{
			comp.reset();
			comp.updateReal(node, prevUpdateTime, crntTime, 
				SceneComponent::SYNC_UPDATE);
		});

		node.frameUpdate(prevUpdateTime, crntTime, SceneNode::SYNC_UPDATE);
	});

	Threadpool& threadPool = *m_threadpool;
	(void)threadPool;

	// XXX Do that in parallel
	//m_physics.update(prevUpdateTime, crntTime);
	renderer.getTiler().updateTiles(*m_mainCam);
	m_events.updateAllEvents(prevUpdateTime, crntTime);

	// Then the rest
	Array<UpdateSceneNodesJob, Threadpool::MAX_THREADS> jobs2;
	Barrier barrier(threadPool.getThreadsCount());

	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		UpdateSceneNodesJob& job = jobs2[i];

		job.scene = this;
		job.prevUpdateTime = prevUpdateTime;
		job.crntTime = crntTime;
		job.barrier = &barrier;

		threadPool.assignNewTask(i, &job);
	}

	threadPool.waitForAllThreadsToFinish();

	doVisibilityTests(*m_mainCam, *this, renderer);

	ANKI_COUNTER_STOP_TIMER_INC(SCENE_UPDATE_TIME);
}

//==============================================================================
void SceneGraph::load(const char* filename)
{}

//==============================================================================
GlDevice& SceneGraph::_getGlDevice()
{
	return m_resources->_getGlDevice();
}

} // end namespace anki
