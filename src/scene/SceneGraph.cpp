// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/SceneGraph.h>
#include <anki/scene/Camera.h>
#include <anki/scene/ModelNode.h>
#include <anki/scene/Sector.h>
#include <anki/core/Trace.h>
#include <anki/physics/PhysicsWorld.h>
#include <anki/resource/ResourceManager.h>
#include <anki/renderer/MainRenderer.h>
#include <anki/renderer/Renderer.h>
#include <anki/misc/ConfigSet.h>

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

namespace {

//==============================================================================
class UpdateSceneNodesTask: public ThreadPool::Task
{
public:
	SceneGraph* m_scene = nullptr;
	F32 m_prevUpdateTime;
	F32 m_crntTime;

	IntrusiveList<SceneNode>::Iterator* m_crntNode;
	SpinLock* m_crntNodeLock;
	IntrusiveList<SceneNode>::Iterator m_nodesEnd;

	Error operator()(U32 taskId, PtrSize threadsCount)
	{
		ANKI_TRACE_START_EVENT(SCENE_NODES_UPDATE);

		IntrusiveList<SceneNode>::Iterator& it = *m_crntNode;
		SpinLock& lock = *m_crntNodeLock;

		Bool quit = false;
		Error err = ErrorCode::NONE;
		while(!quit && !err)
		{
			// Fetch a few scene nodes
			Array<SceneNode*, 5> nodes = {{nullptr, }};
			lock.lock();
			for(SceneNode*& node : nodes)
			{
				if(it != m_nodesEnd)
				{
					node = &(*it);
					++it;
				}
			}
			lock.unlock();

			// Process nodes
			U count = 0;
			for(U i = 0; i < nodes.getSize(); ++i)
			{
				if(nodes[i])
				{
					if(nodes[i]->getParent() == nullptr)
					{
						err = updateInternal(
							*nodes[i], m_prevUpdateTime, m_crntTime);
						ANKI_TRACE_INC_COUNTER(SCENE_NODES_UPDATED, 1);
					}
					++count;
				}
			}

			if(ANKI_UNLIKELY(count == 0))
			{
				quit = true;
			}
		}

		ANKI_TRACE_STOP_EVENT(SCENE_NODES_UPDATE);
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
{
	Error err = iterateSceneNodes([&](SceneNode& s) -> Error
	{
		s.setMarkedForDeletion();
		return ErrorCode::NONE;
	});
	(void)err;

	deleteNodesMarkedForDeletion();

	if(m_sectors)
	{
		m_alloc.deleteInstance(m_sectors);
		m_sectors = nullptr;
	}
}

//==============================================================================
Error SceneGraph::init(
	AllocAlignedCallback allocCb,
	void* allocCbData,
	ThreadPool* threadpool,
	ResourceManager* resources,
	Input* input,
	const Timestamp* globalTimestamp,
	const ConfigSet& config)
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
		1.0,
		0);
	m_frameAlloc = SceneFrameAllocator<U8>(
		allocCb, allocCbData, 1 * 1024 * 1024);

	err = m_events.create(this);

	if(err)
	{
		ANKI_LOGE("Scene creation failed");
	}

	m_sectors = m_alloc.newInstance<SectorGroup>(this);

	m_maxReflectionProxyDistance = config.getNumber(
		"imageReflectionMaxDistance");

	return err;
}

//==============================================================================
Error SceneGraph::registerNode(SceneNode* node)
{
	ANKI_ASSERT(node);

	// Add to dict if it has a name
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
	m_nodes.pushBack(node);
	++m_nodesCount;

	return ErrorCode::NONE;
}

//==============================================================================
void SceneGraph::unregisterNode(SceneNode* node)
{
	// Remove from the graph
	m_nodes.erase(node);
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
		if((*it).getName() == name)
		{
			break;
		}
	}

	return (it == m_nodes.getEnd()) ? nullptr : &(*it);
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
		for(; it != end; ++it)
		{
			SceneNode& node = *it;

			if(node.getMarkedForDeletion())
			{
				// Delete node
				unregisterNode(&node);
				m_alloc.deleteInstance(&node);
				m_objectsMarkedForDeletionCount.fetchSub(1);
				found = true;
				break;
			}
		}

		(void)found;
		ANKI_ASSERT(found && "Something is wrong with marked for deletion");
	}
}

//==============================================================================
Error SceneGraph::update(F32 prevUpdateTime, F32 crntTime,
	MainRenderer& renderer)
{
	ANKI_ASSERT(m_mainCam);
	ANKI_TRACE_START_EVENT(SCENE_UPDATE);

	m_timestamp = *m_globalTimestamp;

	// Reset the framepool
	m_frameAlloc.getMemoryPool().reset();

	// Delete stuff
	ANKI_TRACE_START_EVENT(SCENE_DELETE_STUFF);
	m_events.deleteEventsMarkedForDeletion();
	deleteNodesMarkedForDeletion();
	ANKI_TRACE_STOP_EVENT(SCENE_DELETE_STUFF);

	ThreadPool& threadPool = *m_threadpool;
	(void)threadPool;

	// Update
	ANKI_TRACE_START_EVENT(SCENE_PHYSICS_UPDATE);
	m_physics->updateAsync(crntTime - prevUpdateTime);
	m_physics->waitUpdate();
	ANKI_TRACE_STOP_EVENT(SCENE_PHYSICS_UPDATE);

	ANKI_TRACE_START_EVENT(SCENE_NODES_UPDATE);
	ANKI_CHECK(m_events.updateAllEvents(prevUpdateTime, crntTime));

	// Then the rest
	Array<UpdateSceneNodesTask, ThreadPool::MAX_THREADS> jobs2;
	IntrusiveList<SceneNode>::Iterator nodeIt = m_nodes.getBegin();
	SpinLock nodeItLock;

	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		UpdateSceneNodesTask& job = jobs2[i];

		job.m_scene = this;
		job.m_prevUpdateTime = prevUpdateTime;
		job.m_crntTime = crntTime;
		job.m_crntNode = &nodeIt;
		job.m_crntNodeLock = &nodeItLock;
		job.m_nodesEnd = m_nodes.getEnd();

		threadPool.assignNewTask(i, &job);
	}

	ANKI_CHECK(threadPool.waitForAllThreadsToFinish());
	ANKI_TRACE_STOP_EVENT(SCENE_NODES_UPDATE);

	renderer.getOffscreenRenderer().prepareForVisibilityTests(*m_mainCam);

	ANKI_TRACE_START_EVENT(SCENE_VISIBILITY_TESTS);
	ANKI_CHECK(doVisibilityTests(*m_mainCam, *this,
		renderer.getOffscreenRenderer()));
	ANKI_TRACE_STOP_EVENT(SCENE_VISIBILITY_TESTS);

	ANKI_TRACE_STOP_EVENT(SCENE_UPDATE);
	return ErrorCode::NONE;
}

} // end namespace anki
