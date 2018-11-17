// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/SceneGraph.h>
#include <anki/scene/CameraNode.h>
#include <anki/scene/PhysicsDebugNode.h>
#include <anki/scene/ModelNode.h>
#include <anki/scene/Octree.h>
#include <anki/core/Trace.h>
#include <anki/physics/PhysicsWorld.h>
#include <anki/resource/ResourceManager.h>
#include <anki/renderer/MainRenderer.h>
#include <anki/misc/ConfigSet.h>
#include <anki/util/ThreadHive.h>

namespace anki
{

const U NODE_UPDATE_BATCH = 10;

class SceneGraph::UpdateSceneNodesCtx
{
public:
	SceneGraph* m_scene = nullptr;

	IntrusiveList<SceneNode>::Iterator m_crntNode;
	SpinLock m_crntNodeLock;

	Second m_prevUpdateTime;
	Second m_crntTime;
};

SceneGraph::SceneGraph()
{
}

SceneGraph::~SceneGraph()
{
	Error err = iterateSceneNodes([&](SceneNode& s) -> Error {
		s.setMarkedForDeletion();
		return Error::NONE;
	});
	(void)err;

	deleteNodesMarkedForDeletion();

	if(m_octree)
	{
		m_alloc.deleteInstance(m_octree);
	}
}

Error SceneGraph::init(AllocAlignedCallback allocCb,
	void* allocCbData,
	ThreadHive* threadHive,
	ResourceManager* resources,
	Input* input,
	ScriptManager* scriptManager,
	const Timestamp* globalTimestamp,
	const ConfigSet& config)
{
	m_globalTimestamp = globalTimestamp;
	m_threadHive = threadHive;
	m_resources = resources;
	m_objectsMarkedForDeletionCount.store(0);
	m_gr = &m_resources->getGrManager();
	m_physics = &m_resources->getPhysicsWorld();
	m_input = input;
	m_scriptManager = scriptManager;

	m_alloc = SceneAllocator<U8>(allocCb, allocCbData);
	m_frameAlloc = SceneFrameAllocator<U8>(allocCb, allocCbData, 1 * 1024 * 1024);

	m_earlyZDist = config.getNumber("scene.earlyZDistance");

	ANKI_CHECK(m_events.init(this));

	m_maxReflectionProxyDistance = config.getNumber("scene.imageReflectionMaxDistance");

	m_octree = m_alloc.newInstance<Octree>(m_alloc);
	m_octree->init(m_sceneMin, m_sceneMax, 5); // TODO

	// Init the default main camera
	ANKI_CHECK(newSceneNode<PerspectiveCameraNode>("mainCamera", m_defaultMainCam));
	m_defaultMainCam->setAll(toRad(60.0f), toRad(60.0f), 0.1f, 1000.0f);
	m_mainCam = m_defaultMainCam;

	// Create a special node for debugging the physics world
	PhysicsDebugNode* pnode;
	ANKI_CHECK(newSceneNode<PhysicsDebugNode>("_physicsDebugNode", pnode));

	return Error::NONE;
}

Error SceneGraph::registerNode(SceneNode* node)
{
	ANKI_ASSERT(node);

	// Add to dict if it has a name
	if(node->getName())
	{
		if(tryFindSceneNode(node->getName()))
		{
			ANKI_SCENE_LOGE("Node with the same name already exists");
			return Error::USER_DATA;
		}

		m_nodesDict.emplace(m_alloc, node->getName(), node);
	}

	// Add to vector
	m_nodes.pushBack(node);
	++m_nodesCount;

	return Error::NONE;
}

void SceneGraph::unregisterNode(SceneNode* node)
{
	// Remove from the graph
	m_nodes.erase(node);
	--m_nodesCount;

	if(m_mainCam != m_defaultMainCam && m_mainCam == node)
	{
		m_mainCam = m_defaultMainCam;
	}

	// Remove from dict
	if(node->getName())
	{
		auto it = m_nodesDict.find(node->getName());
		ANKI_ASSERT(it != m_nodesDict.getEnd());
		m_nodesDict.erase(m_alloc, it);
	}
}

SceneNode& SceneGraph::findSceneNode(const CString& name)
{
	SceneNode* node = tryFindSceneNode(name);
	ANKI_ASSERT(node);
	return *node;
}

SceneNode* SceneGraph::tryFindSceneNode(const CString& name)
{
	auto it = m_nodesDict.find(name);
	return (it == m_nodesDict.getEnd()) ? nullptr : (*it);
}

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

Error SceneGraph::update(Second prevUpdateTime, Second crntTime)
{
	ANKI_ASSERT(m_mainCam);
	ANKI_TRACE_SCOPED_EVENT(SCENE_UPDATE);

	m_stats.m_updateTime = HighRezTimer::getCurrentTime();

	m_timestamp = *m_globalTimestamp;
	ANKI_ASSERT(m_timestamp > 0);

	// Reset the framepool
	m_frameAlloc.getMemoryPool().reset();

	// Delete stuff
	{
		ANKI_TRACE_SCOPED_EVENT(SCENE_MARKED_FOR_DELETION);
		m_events.deleteEventsMarkedForDeletion();
		deleteNodesMarkedForDeletion();
	}

	// Update
	{
		ANKI_TRACE_SCOPED_EVENT(SCENE_PHYSICS_UPDATE);
		m_stats.m_physicsUpdate = HighRezTimer::getCurrentTime();
		m_physics->update(crntTime - prevUpdateTime);
		m_stats.m_physicsUpdate = HighRezTimer::getCurrentTime() - m_stats.m_physicsUpdate;
	}

	{
		ANKI_TRACE_SCOPED_EVENT(SCENE_NODES_UPDATE);
		ANKI_CHECK(m_events.updateAllEvents(prevUpdateTime, crntTime));

		// Then the rest
		Array<ThreadHiveTask, ThreadHive::MAX_THREADS> tasks;
		UpdateSceneNodesCtx updateCtx;
		updateCtx.m_scene = this;
		updateCtx.m_crntNode = m_nodes.getBegin();
		updateCtx.m_prevUpdateTime = prevUpdateTime;
		updateCtx.m_crntTime = crntTime;

		for(U i = 0; i < m_threadHive->getThreadCount(); i++)
		{
			tasks[i] = ANKI_THREAD_HIVE_TASK(
				{
					if(self->m_scene->updateNodes(*self))
					{
						ANKI_SCENE_LOGF("Will not recover");
					}
				},
				&updateCtx,
				nullptr,
				nullptr);
		}

		m_threadHive->submitTasks(&tasks[0], m_threadHive->getThreadCount());
		m_threadHive->waitAllTasks();
	}

	m_stats.m_updateTime = HighRezTimer::getCurrentTime() - m_stats.m_updateTime;
	return Error::NONE;
}

void SceneGraph::doVisibilityTests(RenderQueue& rqueue)
{
	m_stats.m_visibilityTestsTime = HighRezTimer::getCurrentTime();
	doVisibilityTests(*m_mainCam, *this, rqueue);
	m_stats.m_visibilityTestsTime = HighRezTimer::getCurrentTime() - m_stats.m_visibilityTestsTime;
}

Error SceneGraph::updateNode(Second prevTime, Second crntTime, SceneNode& node)
{
	ANKI_TRACE_INC_COUNTER(SCENE_NODES_UPDATED, 1);

	Error err = Error::NONE;

	// Components update
	Timestamp componentTimestamp = 0;
	err = node.iterateComponents([&](SceneComponent& comp) -> Error {
		Bool updated = false;
		Error e = comp.update(node, prevTime, crntTime, updated);

		if(updated)
		{
			comp.setTimestamp(node.getSceneGraph().m_timestamp);
			componentTimestamp = max(componentTimestamp, node.getSceneGraph().m_timestamp);
			ANKI_ASSERT(componentTimestamp > 0);
		}

		return e;
	});

	// Update children
	if(!err)
	{
		err = node.visitChildrenMaxDepth(
			0, [&](SceneNode& child) -> Error { return updateNode(prevTime, crntTime, child); });
	}

	// Frame update
	if(!err)
	{
		if(componentTimestamp != 0)
		{
			node.setComponentMaxTimestamp(componentTimestamp);
		}
		else
		{
			// No components or nothing updated, don't change the timestamp
		}

		err = node.frameUpdate(prevTime, crntTime);
	}

	return err;
}

Error SceneGraph::updateNodes(UpdateSceneNodesCtx& ctx) const
{
	ANKI_TRACE_SCOPED_EVENT(SCENE_NODES_UPDATE);

	IntrusiveList<SceneNode>::ConstIterator end = m_nodes.getEnd();

	Bool quit = false;
	Error err = Error::NONE;
	while(!quit && !err)
	{
		// Fetch a batch of scene nodes that don't have parent
		Array<SceneNode*, NODE_UPDATE_BATCH> batch;
		U batchSize = 0;

		{
			LockGuard<SpinLock> lock(ctx.m_crntNodeLock);

			while(1)
			{
				if(batchSize == batch.getSize())
				{
					break;
				}

				if(ctx.m_crntNode == end)
				{
					quit = true;
					break;
				}

				SceneNode& node = *ctx.m_crntNode;
				if(node.getParent() == nullptr)
				{
					batch[batchSize++] = &node;
				}

				++ctx.m_crntNode;
			}
		}

		// Process nodes
		for(U i = 0; i < batchSize && !err; ++i)
		{
			err = updateNode(ctx.m_prevUpdateTime, ctx.m_crntTime, *batch[i]);
		}
	}

	return err;
}

} // end namespace anki
