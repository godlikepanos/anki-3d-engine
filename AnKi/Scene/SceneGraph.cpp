// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Scene/RenderStateBucket.h>
#include <AnKi/Physics/PhysicsWorld.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Util/CVarSet.h>
#include <AnKi/Core/StatsSet.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Util/HighRezTimer.h>
#include <AnKi/Core/App.h>
#include <AnKi/Scene/StatsUiNode.h>
#include <AnKi/Scene/DeveloperConsoleUiNode.h>

#include <AnKi/Scene/Components/BodyComponent.h>
#include <AnKi/Scene/Components/CameraComponent.h>
#include <AnKi/Scene/Components/DecalComponent.h>
#include <AnKi/Scene/Components/FogDensityComponent.h>
#include <AnKi/Scene/Components/GlobalIlluminationProbeComponent.h>
#include <AnKi/Scene/Components/JointComponent.h>
#include <AnKi/Scene/Components/LensFlareComponent.h>
#include <AnKi/Scene/Components/LightComponent.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/Components/ParticleEmitterComponent.h>
#include <AnKi/Scene/Components/PlayerControllerComponent.h>
#include <AnKi/Scene/Components/ReflectionProbeComponent.h>
#include <AnKi/Scene/Components/ScriptComponent.h>
#include <AnKi/Scene/Components/SkinComponent.h>
#include <AnKi/Scene/Components/SkyboxComponent.h>
#include <AnKi/Scene/Components/TriggerComponent.h>
#include <AnKi/Scene/Components/UiComponent.h>
#include <AnKi/Scene/Components/MeshComponent.h>
#include <AnKi/Scene/Components/MaterialComponent.h>

namespace anki {

ANKI_SVAR(SceneUpdateTime, StatCategory::kTime, "All scene update", StatFlag::kMilisecond | StatFlag::kShowAverage | StatFlag::kMainThreadUpdates)
ANKI_SVAR(SceneComponentsUpdated, StatCategory::kScene, "Scene components updated per frame", StatFlag::kZeroEveryFrame)
ANKI_SVAR(SceneNodesUpdated, StatCategory::kScene, "Scene nodes updated per frame", StatFlag::kZeroEveryFrame)

constexpr U32 kUpdateNodeBatchSize = 10;

class SceneGraph::UpdateSceneNodesCtx
{
public:
	IntrusiveList<SceneNode>::Iterator m_crntNode;
	SpinLock m_crntNodeLock;

	Second m_prevUpdateTime;
	Second m_crntTime;

	DynamicArray<SceneNode*, MemoryPoolPtrWrapper<StackMemoryPool>> m_nodesForDeletion;

	Vec3 m_sceneMin = Vec3(kMaxF32);
	Vec3 m_sceneMax = Vec3(kMinF32);
	SpinLock m_sceneBoundsLock;

	UpdateSceneNodesCtx()
		: m_nodesForDeletion(&SceneGraph::getSingleton().m_framePool)
	{
	}
};

SceneGraph::SceneGraph()
{
}

SceneGraph::~SceneGraph()
{
	while(!m_nodesForRegistration.isEmpty())
	{
		deleteInstance(SceneMemoryPool::getSingleton(), m_nodesForRegistration.popBack());
	}

	while(!m_nodes.isEmpty())
	{
		deleteInstance(SceneMemoryPool::getSingleton(), m_nodes.popBack());
	}

#define ANKI_CAT_TYPE(arrayName, gpuSceneType, id, cvarName) GpuSceneArrays::arrayName::freeSingleton();
#include <AnKi/Scene/GpuSceneArrays.def.h>

	RenderStateBucketContainer::freeSingleton();
}

Error SceneGraph::init(AllocAlignedCallback allocCallback, void* allocCallbackData)
{
	SceneMemoryPool::allocateSingleton(allocCallback, allocCallbackData);

	m_framePool.init(allocCallback, allocCallbackData, 1_MB, 2.0, 0, true, "SceneGraphFramePool");

	// Init the default main camera
	m_defaultMainCam = newSceneNode<SceneNode>("mainCamera");
	CameraComponent* camc = m_defaultMainCam->newComponent<CameraComponent>();
	camc->setPerspective(0.1f, 1000.0f, toRad(60.0f), (1080.0f / 1920.0f) * toRad(60.0f));
	m_mainCam = m_defaultMainCam;

#define ANKI_CAT_TYPE(arrayName, gpuSceneType, id, cvarName) GpuSceneArrays::arrayName::allocateSingleton(U32(cvarName));
#include <AnKi/Scene/GpuSceneArrays.def.h>

	RenderStateBucketContainer::allocateSingleton();

	// Construct a few common nodex
	if(g_cvarCoreDisplayStats > 0)
	{
		StatsUiNode* statsNode = newSceneNode<StatsUiNode>("_StatsUi");
		statsNode->setFpsOnly(g_cvarCoreDisplayStats == 1);
	}

	newSceneNode<DeveloperConsoleUiNode>("_DevConsole");

	return Error::kNone;
}

SceneNode& SceneGraph::findSceneNode(const CString& name)
{
	SceneNode* node = tryFindSceneNode(name);
	ANKI_ASSERT(node);
	return *node;
}

SceneNode* SceneGraph::tryFindSceneNode(const CString& name)
{
	// Search registered nodes
	auto it = m_nodesDict.find(name);
	if(it != m_nodesDict.getEnd())
	{
		return *it;
	}

	// Didn't found it, search those up for registration
	LockGuard lock(m_nodesForRegistrationMtx);
	for(SceneNode& node : m_nodesForRegistration)
	{
		if(node.getName() == name)
		{
			return &node;
		}
	}

	return nullptr;
}

void SceneGraph::update(Second prevUpdateTime, Second crntTime)
{
	ANKI_ASSERT(m_mainCam);
	ANKI_TRACE_SCOPED_EVENT(SceneUpdate);

	const Second startUpdateTime = HighRezTimer::getCurrentTime();

	// Reset the framepool
	m_framePool.reset();

	// Register new nodes
	while(!m_nodesForRegistration.isEmpty())
	{
		SceneNode* node = m_nodesForRegistration.popFront();

		// Add to dict if it has a name
		if(node->getName() != "Unnamed")
		{
			if(tryFindSceneNode(node->getName()))
			{
				ANKI_SCENE_LOGE("Node with the same name already exists. New node will not be searchable: %s", node->getName().cstr());
			}
			else
			{
				m_nodesDict.emplace(node->getName(), node);
			}
		}

		// Add to list
		m_nodes.pushBack(node);
		++m_nodesCount;
	}

	// Update physics
	PhysicsWorld::getSingleton().update(crntTime - prevUpdateTime);

	// Before the update wake some threads with dummy work
	for(U i = 0; i < 2; i++)
	{
		CoreThreadJobManager::getSingleton().dispatchTask([]([[maybe_unused]] U32 tid) {});
	}

	// Update events and scene nodes
	UpdateSceneNodesCtx updateCtx;
	{
		ANKI_TRACE_SCOPED_EVENT(SceneNodesUpdate);
		m_events.updateAllEvents(prevUpdateTime, crntTime);

		updateCtx.m_crntNode = m_nodes.getBegin();
		updateCtx.m_prevUpdateTime = prevUpdateTime;
		updateCtx.m_crntTime = crntTime;

		for(U i = 0; i < CoreThreadJobManager::getSingleton().getThreadCount(); i++)
		{
			CoreThreadJobManager::getSingleton().dispatchTask([this, &updateCtx]([[maybe_unused]] U32 tid) {
				updateNodes(updateCtx);
			});
		}

		CoreThreadJobManager::getSingleton().waitForAllTasksToFinish();

		if(updateCtx.m_sceneMin != Vec3(kMaxF32))
		{
			const Bool forceUpdateSceneBounds = (m_frame % kForceSetSceneBoundsFrameCount) == 0;
			if(forceUpdateSceneBounds)
			{
				m_sceneMin = updateCtx.m_sceneMin;
				m_sceneMax = updateCtx.m_sceneMax;
			}
			else
			{
				m_sceneMin = m_sceneMin.min(updateCtx.m_sceneMin);
				m_sceneMax = m_sceneMax.max(updateCtx.m_sceneMax);
			}
		}
	}

	// Cleanup
	for(SceneNode* node : updateCtx.m_nodesForDeletion)
	{
		// Remove from the graph
		m_nodes.erase(node);
		ANKI_ASSERT(m_nodesCount > 0);
		--m_nodesCount;

		if(m_mainCam != m_defaultMainCam && m_mainCam == node)
		{
			m_mainCam = m_defaultMainCam;
		}

		// Remove from dict
		if(node->getName() != "Unnamed")
		{
			auto it = m_nodesDict.find(node->getName());
			ANKI_ASSERT(it != m_nodesDict.getEnd());
			if(*it == node)
			{
				m_nodesDict.erase(it);
			}
		}

		deleteInstance(SceneMemoryPool::getSingleton(), node);
	}
	updateCtx.m_nodesForDeletion.destroy();

#define ANKI_CAT_TYPE(arrayName, gpuSceneType, id, cvarName) GpuSceneArrays::arrayName::getSingleton().flush();
#include <AnKi/Scene/GpuSceneArrays.def.h>

	g_svarSceneUpdateTime.set((HighRezTimer::getCurrentTime() - startUpdateTime) * 1000.0);
	++m_frame;
}

void SceneGraph::updateNode(SceneNode& node, SceneComponentUpdateInfo& componentUpdateInfo)
{
	ANKI_TRACE_INC_COUNTER(SceneNodeUpdated, 1);

	// Components update
	U32 sceneComponentUpdatedCount = 0;
	node.iterateComponents([&](SceneComponent& comp) {
		componentUpdateInfo.m_node = &node;
		Bool updated = false;
		comp.update(componentUpdateInfo, updated);

		if(updated)
		{
			ANKI_TRACE_INC_COUNTER(SceneComponentUpdated, 1);
			comp.setTimestamp(GlobalFrameIndex::getSingleton().m_value);
			++sceneComponentUpdatedCount;
		}
	});

	// Frame update
	{
		if(sceneComponentUpdatedCount)
		{
			node.setComponentMaxTimestamp(GlobalFrameIndex::getSingleton().m_value);
			g_svarSceneComponentsUpdated.increment(sceneComponentUpdatedCount);
			g_svarSceneNodesUpdated.increment(1);
		}
		else
		{
			// No components or nothing updated, don't change the timestamp
		}

		node.frameUpdate(componentUpdateInfo.m_previousTime, componentUpdateInfo.m_currentTime);
	}

	// Update children
	node.visitChildrenMaxDepth(0, [&](SceneNode& child) {
		updateNode(child, componentUpdateInfo);
		return true;
	});
}

void SceneGraph::updateNodes(UpdateSceneNodesCtx& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(SceneNodeUpdate);

	IntrusiveList<SceneNode>::ConstIterator end = m_nodes.getEnd();

	const Bool forceUpdateSceneBounds = (m_frame % kForceSetSceneBoundsFrameCount) == 0;
	SceneComponentUpdateInfo componentUpdateInfo(ctx.m_prevUpdateTime, ctx.m_crntTime, forceUpdateSceneBounds);
	componentUpdateInfo.m_framePool = &m_framePool;

	Bool quit = false;
	while(!quit)
	{
		// Fetch a batch of scene nodes that don't have parent
		Array<SceneNode*, kUpdateNodeBatchSize> batch;
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

				if(node.isMarkedForDeletion())
				{
					ctx.m_nodesForDeletion.emplaceBack(&node);
				}
				else if(node.getParent() == nullptr)
				{
					batch[batchSize++] = &node;
				}
				else
				{
					// Ignore
				}

				++ctx.m_crntNode;
			}
		}

		// Process nodes
		for(U i = 0; i < batchSize; ++i)
		{
			updateNode(*batch[i], componentUpdateInfo);
		}
	}

	if(componentUpdateInfo.m_sceneMin != Vec3(kMaxF32))
	{
		LockGuard lock(ctx.m_sceneBoundsLock);
		ctx.m_sceneMin = ctx.m_sceneMin.min(componentUpdateInfo.m_sceneMin);
		ctx.m_sceneMax = ctx.m_sceneMax.max(componentUpdateInfo.m_sceneMax);
	}
}

LightComponent* SceneGraph::getDirectionalLight() const
{
	LightComponent* out = (m_dirLights.getSize()) ? m_dirLights[0] : nullptr;
	if(out)
	{
		ANKI_ASSERT(out->getLightComponentType() == LightComponentType::kDirectional);
	}
	return out;
}

const SceneNode& SceneGraph::getActiveCameraNode() const
{
	ANKI_ASSERT(m_mainCam);
	if(ANKI_EXPECT(m_mainCam->hasComponent<CameraComponent>()))
	{
		return *m_mainCam;
	}
	else
	{
		return *m_defaultMainCam;
	}
}

void SceneGraph::setActiveCameraNode(SceneNode* cam)
{
	if(cam)
	{
		m_mainCam = cam;
	}
	else
	{
		m_mainCam = m_defaultMainCam;
	}
}

} // end namespace anki
