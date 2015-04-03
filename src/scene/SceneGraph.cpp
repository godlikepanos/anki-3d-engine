// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/SceneGraph.h"
#include "anki/scene/Camera.h"
#include "anki/scene/ModelNode.h"
#include "anki/scene/InstanceNode.h"
#include "anki/core/Counters.h"
#include "anki/renderer/Renderer.h"
#include "anki/physics/PhysicsWorld.h"

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

	Error operator()(U32 taskId, PtrSize threadsCount)
	{
		PtrSize start, end;
		choseStartEnd(
			taskId, threadsCount, m_scene->getSceneNodesCount(), start, end);

		// Start with the root nodes
		Error err = m_scene->iterateSceneNodes(
			start, end, [&](SceneNode& node) -> Error
		{
			Error err = ErrorCode::NONE;
			if(node.getParent() == nullptr)
			{
				err = updateInternal(node, m_prevUpdateTime, m_crntTime);
			}

			return err;
		});

		return err;
	}

	ANKI_USE_RESULT Error updateInternal(
		SceneNode& node, F32 prevTime, F32 crntTime)
	{
		Error err = ErrorCode::NONE;

		// Components update
		err = node.iterateComponents([&](SceneComponent& comp) -> Error
		{
			Bool updated = false;
			return comp.updateReal(node, prevTime, crntTime, updated);
		});

		if(err)
		{
			return err;
		}

		// Update children
		err = node.visitChildren([&](SceneNode& child) -> Error
		{
			return updateInternal(child, prevTime, crntTime);
		});

		// Frame update
		if(!err)
		{
			err = node.frameUpdate(prevTime, crntTime);
		}

		return err;
	}
};

} // end namespace anonymous

//==============================================================================
// Scene                                                                       =
//==============================================================================

//==============================================================================
SceneGraph::SceneGraph()
{}

//==============================================================================
SceneGraph::~SceneGraph()
{}

//==============================================================================
Error SceneGraph::create(
	AllocAlignedCallback allocCb, 
	void* allocCbData, 
	U32 frameAllocatorSize,
	Threadpool* threadpool, 
	ResourceManager* resources,
	Input* input,
	const Timestamp* globalTimestamp)
{
	Error err = ErrorCode::NONE;

	m_globalTimestamp = globalTimestamp;
	m_threadpool = threadpool;
	m_resources = resources;
	m_objectsMarkedForDeletionCount.store(0);
	m_ambientCol = Vec3(0.0);
	m_gr = &m_resources->getGrManager();
	m_physics = &m_resources->_getPhysicsWorld();
	m_input = input;

	m_alloc = SceneAllocator<U8>(
		allocCb, allocCbData, 
		1024 * 10,
		1024 * 10,
		ChainMemoryPool::ChunkGrowMethod::FIXED,
		0);
	m_frameAlloc = SceneFrameAllocator<U8>(
		allocCb, allocCbData, frameAllocatorSize);

	err = m_events.create(this);

	if(err)
	{
		ANKI_LOGE("Scene creation failed");
	}

	return err;
}

//==============================================================================
Error SceneGraph::registerNode(SceneNode* node)
{
	ANKI_ASSERT(node);

	// Add to dict if it has name
	if(node->getName())
	{
		if(tryFindSceneNode(node->getName()))
		{
			ANKI_LOGE("Node with the same name already exists");
			return ErrorCode::FUNCTION_FAILED;
		}

		//m_dict[node->getName()] = node;
	}

	// Add to vector
	m_nodes.pushBack(m_alloc, node);
	++m_nodesCount;

	return ErrorCode::NONE;
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
	m_nodes.erase(m_alloc, it);

	--m_nodesCount;

	// Remove from dict
#if 0
	if(node->getName())
	{
		auto it = m_dict.find(node->getName());

		ANKI_ASSERT(it != m_dict.end());
		m_dict.erase(it);
	}
#endif
}

//==============================================================================
SceneNode& SceneGraph::findSceneNode(const CString& name)
{
	SceneNode* node = tryFindSceneNode(name);
	ANKI_ASSERT(node);
	return *node;
}

//==============================================================================
SceneNode* SceneGraph::tryFindSceneNode(const CString& name)
{
	auto it = m_nodes.getBegin();
	for(; it != m_nodes.getEnd(); ++it)
	{
		if((*it)->getName() == name)
		{
			break;
		}
	}

	return (it == m_nodes.getEnd()) ? nullptr : (*it);
}

//==============================================================================
void SceneGraph::deleteNodesMarkedForDeletion()
{
	/// Delete all nodes pending deletion. At this point all scene threads 
	/// should have finished their tasks
	while(m_objectsMarkedForDeletionCount.load() > 0)
	{
		Bool found = false;
		auto it = m_nodes.begin();
		auto end = m_nodes.end();
		for(; it != end; it++)
		{
			if((*it)->getMarkedForDeletion())
			{
				// Delete node
				unregisterNode(*it);
				m_alloc.deleteInstance(*it);
				found = true;
				break;
			}
		}

		(void)found;
		ANKI_ASSERT(found && "Something is wrong with marked for deletion");
	}
}

//==============================================================================
Error SceneGraph::update(F32 prevUpdateTime, F32 crntTime, Renderer& renderer)
{
	ANKI_ASSERT(m_mainCam);

	Error err = ErrorCode::NONE;

	m_timestamp = *m_globalTimestamp;

	ANKI_COUNTER_START_TIMER(SCENE_UPDATE_TIME);

	//
	// Sync point. Here we wait for all scene's threads
	//

	// Reset the framepool
	m_frameAlloc.getMemoryPool().reset();

	// Delete stuff
	m_events.deleteEventsMarkedForDeletion();
	deleteNodesMarkedForDeletion();

	Threadpool& threadPool = *m_threadpool;
	(void)threadPool;

	// Update
	m_physics->updateAsync(crntTime - prevUpdateTime);
	renderer.getTiler().updateTiles(*m_mainCam);
	m_physics->waitUpdate();
	err = m_events.updateAllEvents(prevUpdateTime, crntTime);
	if(err) return err;

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

	err = threadPool.waitForAllThreadsToFinish();

	if(!err)
	{
		err = doVisibilityTests(*m_mainCam, *this, renderer);
	}

	ANKI_COUNTER_STOP_TIMER_INC(SCENE_UPDATE_TIME);
	return err;
}

} // end namespace anki
