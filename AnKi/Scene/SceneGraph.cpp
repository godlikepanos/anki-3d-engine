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
#include <AnKi/Util/Filesystem.h>
#include <AnKi/Core/App.h>
#include <AnKi/Resource/ScriptResource.h>
#include <AnKi/Script/ScriptManager.h>
#include <AnKi/Scene/StatsUiNode.h>
#include <AnKi/Scene/DeveloperConsoleUiNode.h>
#include <AnKi/Scene/EditorUiNode.h>

#include <AnKi/Scene/Components/BodyComponent.h>
#include <AnKi/Scene/Components/CameraComponent.h>
#include <AnKi/Scene/Components/DecalComponent.h>
#include <AnKi/Scene/Components/FogDensityComponent.h>
#include <AnKi/Scene/Components/GlobalIlluminationProbeComponent.h>
#include <AnKi/Scene/Components/JointComponent.h>
#include <AnKi/Scene/Components/LensFlareComponent.h>
#include <AnKi/Scene/Components/LightComponent.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/Components/ParticleEmitter2Component.h>
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

class SceneGraph::UpdateSceneNodesCtx
{
public:
	class PerThread
	{
	public:
		DynamicArray<SceneNode*, MemoryPoolPtrWrapper<StackMemoryPool>> m_nodesForDeletion;

		LightComponent* m_dirLightComponent = nullptr;
		SkyboxComponent* m_skyboxComponent = nullptr;

		Vec3 m_sceneMin = Vec3(kMaxF32);
		Vec3 m_sceneMax = Vec3(kMinF32);

		Bool m_multipleDirLights : 1 = false;
		Bool m_multipleSkyboxes : 1 = false;

		PerThread()
			: m_nodesForDeletion(&SceneGraph::getSingleton().m_framePool)
		{
		}
	};

	Atomic<U32> m_crntNodeIndex = {0};
	U32 m_lastNodeIndex = 0;

	Second m_prevUpdateTime = 0.0;
	Second m_crntTime = 0.0;

	DynamicArray<PerThread, MemoryPoolPtrWrapper<StackMemoryPool>> m_perThread;

	Bool m_forceUpdateSceneBounds = false;

	UpdateSceneNodesCtx(U32 threadCount)
		: m_perThread(&SceneGraph::getSingleton().m_framePool)
	{
		m_perThread.resize(threadCount);
	}
};

SceneGraph::SceneGraph()
{
}

SceneGraph::~SceneGraph()
{
	for(SceneNode* node : m_deferredOps.m_nodesForRegistration)
	{
		deleteInstance(SceneMemoryPool::getSingleton(), node);
	}

	for(Scene& scene : m_scenes)
	{
		for(SceneNode* node : scene.m_nodes)
		{
			deleteInstance(SceneMemoryPool::getSingleton(), node);
		}
	}

#define ANKI_CAT_TYPE(arrayName, gpuSceneType, id, cvarName) GpuSceneArrays::arrayName::freeSingleton();
#include <AnKi/Scene/GpuSceneArrays.def.h>

	RenderStateBucketContainer::freeSingleton();
}

Error SceneGraph::init(AllocAlignedCallback allocCallback, void* allocCallbackData)
{
	SceneMemoryPool::allocateSingleton(allocCallback, allocCallbackData);

	m_framePool.init(allocCallback, allocCallbackData, 1_MB, 2.0, 0, true, "SceneGraphFramePool");

#define ANKI_CAT_TYPE(arrayName, gpuSceneType, id, cvarName) GpuSceneArrays::arrayName::allocateSingleton(U32(cvarName));
#include <AnKi/Scene/GpuSceneArrays.def.h>

	// Setup the default scene
	{
		Scene* scene = newEmptyScene("_DefaultScene");
		setActiveScene(scene);

		// Init the default main camera
		m_defaultMainCam = newSceneNode<SceneNode>("_MainCamera");
		m_defaultMainCam->setSerialization(false);
		CameraComponent* camc = m_defaultMainCam->newComponent<CameraComponent>();
		camc->setPerspective(0.1f, 1000.0f, toRad(100.0f), toRad(100.0f));
		m_mainCam = m_defaultMainCam;

		RenderStateBucketContainer::allocateSingleton();

		// Construct a few common nodes
		if(g_cvarCoreDisplayStats > 0)
		{
			StatsUiNode* statsNode = newSceneNode<StatsUiNode>("_StatsUi");
			statsNode->setFpsOnly(g_cvarCoreDisplayStats == 1);
		}

		newSceneNode<DeveloperConsoleUiNode>("_DevConsole");

		EditorUiNode* editorNode = newSceneNode<EditorUiNode>("_Editor");
		editorNode->getFirstComponentOfType<UiComponent>().setEnabled(false);
		m_editorUi = editorNode;

		scene->m_immutable = true;
	}

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
	LockGuard lock(m_deferredOps.m_mtx);
	for(SceneNode* node : m_deferredOps.m_nodesForRegistration)
	{
		if(node->getName() == name)
		{
			return node;
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

	// Deferred ops at the beginning
	doDeferredOperations();

	// Update physics
	if(!m_paused) [[likely]]
	{
		PhysicsWorld::getSingleton().update(crntTime - prevUpdateTime);
	}

#if ANKI_ASSERTIONS_ENABLED
	m_inUpdate = true;
#endif

	// Update events
	{
		ANKI_TRACE_SCOPED_EVENT(EventsUpdate);
		m_events.updateAllEvents(prevUpdateTime, crntTime);
	}

	// Update scene nodes
	UpdateSceneNodesCtx updateCtx(CoreThreadJobManager::getSingleton().getThreadCount());
	{
		// Before the update wake some threads with dummy work
		for(U32 i = 0; i < 2; i++)
		{
			CoreThreadJobManager::getSingleton().dispatchTask([]([[maybe_unused]] U32 tid) {});
		}

		ANKI_TRACE_SCOPED_EVENT(SceneNodesUpdate);
		updateCtx.m_crntNodeIndex.setNonAtomically((m_updatableNodes.isEmpty()) ? 0 : m_updatableNodes.getFront().getArrayIndex());
		updateCtx.m_lastNodeIndex = (m_updatableNodes.isEmpty()) ? 0 : m_updatableNodes.getBack().getArrayIndex();
		updateCtx.m_prevUpdateTime = prevUpdateTime;
		updateCtx.m_crntTime = crntTime;
		updateCtx.m_forceUpdateSceneBounds = (m_frame % kForceSetSceneBoundsFrameCount) == 0;

		for(U32 i = 0; i < CoreThreadJobManager::getSingleton().getThreadCount(); i++)
		{
			CoreThreadJobManager::getSingleton().dispatchTask([this, &updateCtx](U32 tid) {
				updateNodes(tid, updateCtx);
			});
		}

		CoreThreadJobManager::getSingleton().waitForAllTasksToFinish();
	}

#if ANKI_ASSERTIONS_ENABLED
	m_inUpdate = false;
#endif

	// Update scene bounds
	{
		Vec3 sceneMin = Vec3(kMaxF32);
		Vec3 sceneMax = Vec3(kMinF32);
		for(U32 tid = 0; tid < updateCtx.m_perThread.getSize(); ++tid)
		{
			const UpdateSceneNodesCtx::PerThread& thread = updateCtx.m_perThread[tid];
			sceneMin = sceneMin.min(thread.m_sceneMin);
			sceneMax = sceneMax.max(thread.m_sceneMax);
		}

		if(sceneMin.x != kMaxF32)
		{
			if(updateCtx.m_forceUpdateSceneBounds)
			{
				m_sceneMin = sceneMin;
				m_sceneMax = sceneMax;
			}
			else
			{
				m_sceneMin = m_sceneMin.min(sceneMin);
				m_sceneMax = m_sceneMax.max(sceneMax);
			}
		}
	}

	// Set some unique components
	{
		m_activeDirLight = nullptr;
		m_activeSkybox = nullptr;
		Bool bMultipleDirLights = false;
		Bool bMultipleSkyboxes = false;
		for(U32 tid = 0; tid < updateCtx.m_perThread.getSize(); ++tid)
		{
			const UpdateSceneNodesCtx::PerThread& thread = updateCtx.m_perThread[tid];
			bMultipleDirLights = bMultipleDirLights || thread.m_multipleDirLights;
			bMultipleSkyboxes = bMultipleSkyboxes || thread.m_multipleSkyboxes;

			if(thread.m_dirLightComponent)
			{
				if(m_activeDirLight)
				{
					bMultipleDirLights = true;
					if(thread.m_dirLightComponent->getUuid() < m_activeDirLight->getUuid())
					{
						m_activeDirLight = thread.m_dirLightComponent;
					}
				}
				else
				{
					m_activeDirLight = thread.m_dirLightComponent;
				}
			}

			if(thread.m_skyboxComponent)
			{
				if(m_activeSkybox)
				{
					bMultipleSkyboxes = true;
					if(thread.m_skyboxComponent->getUuid() < m_activeSkybox->getUuid())
					{
						m_activeSkybox = thread.m_skyboxComponent;
					}
				}
				else
				{
					m_activeSkybox = thread.m_skyboxComponent;
				}
			}
		}

		if(bMultipleDirLights)
		{
			ANKI_SCENE_LOGW("There are multiple dir lights in the scene. Choosing one to be active");
		}

		if(bMultipleSkyboxes)
		{
			ANKI_SCENE_LOGW("There are multiple skyboxes in the scene. Choosing one to be active");
		}
	}

	// Cleanup
	for(U32 tid = 0; tid < updateCtx.m_perThread.getSize(); ++tid)
	{
		const auto& thread = updateCtx.m_perThread[tid];

		for(SceneNode* node : thread.m_nodesForDeletion)
		{
			Scene& scene = m_scenes[node->m_sceneIndex];

			// Remove from the scene
			scene.m_nodes.erase(node->m_nodeArrayIndex);

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

			// Remove from the updates array
			if(node->m_updatableNodesArrayIndex != kMaxU32)
			{
				m_updatableNodes.erase(node->m_updatableNodesArrayIndex);
			}

			// ...and now can delete
			deleteInstance(SceneMemoryPool::getSingleton(), node);
		}
	}

	// Misc
#define ANKI_CAT_TYPE(arrayName, gpuSceneType, id, cvarName) GpuSceneArrays::arrayName::getSingleton().flush();
#include <AnKi/Scene/GpuSceneArrays.def.h>

	g_svarSceneUpdateTime.set((HighRezTimer::getCurrentTime() - startUpdateTime) * 1000.0);
	++m_frame;
}

void SceneGraph::updateNode(U32 tid, SceneNode& node, UpdateSceneNodesCtx& ctx)
{
	ANKI_TRACE_FUNCTION();
	ANKI_TRACE_INC_COUNTER(SceneNodeUpdated, 1);

	UpdateSceneNodesCtx::PerThread& thread = ctx.m_perThread[tid];

	// Components update
	SceneComponentUpdateInfo componentUpdateInfo(ctx.m_prevUpdateTime, ctx.m_crntTime, ctx.m_forceUpdateSceneBounds, m_checkForResourceUpdates,
												 m_paused);
	componentUpdateInfo.m_framePool = &m_framePool;
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

		if(comp.getType() == SceneComponentType::kLight)
		{
			LightComponent& lc = static_cast<LightComponent&>(comp);
			if(lc.getLightComponentType() == LightComponentType::kDirectional)
			{
				if(thread.m_dirLightComponent)
				{
					thread.m_multipleDirLights = true;
					// Try to choose the same dir light in a deterministic way
					if(lc.getUuid() < thread.m_dirLightComponent->getUuid())
					{
						ctx.m_perThread[tid].m_dirLightComponent = &lc;
					}
				}
				else
				{
					ctx.m_perThread[tid].m_dirLightComponent = &lc;
				}
			}
		}
		else if(comp.getType() == SceneComponentType::kSkybox)
		{
			SkyboxComponent& skyc = static_cast<SkyboxComponent&>(comp);
			if(thread.m_skyboxComponent)
			{
				thread.m_multipleSkyboxes = true;
				// Try to choose the same skybox in a deterministic way
				if(skyc.getUuid() < thread.m_skyboxComponent->getUuid())
				{
					ctx.m_perThread[tid].m_skyboxComponent = &skyc;
				}
			}
			else
			{
				ctx.m_perThread[tid].m_skyboxComponent = &skyc;
			}
		}

		return FunctorContinue::kContinue;
	});

	// Node update
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

		if(!m_paused || node.getUpdateOnPause()) [[likely]]
		{
			SceneNodeUpdateInfo info(ctx.m_prevUpdateTime, ctx.m_crntTime, m_paused);
			node.update(info);
		}
	}

	// Update children
	node.visitChildrenMaxDepth(0, [&](SceneNode& child) {
		updateNode(tid, child, ctx);
		return FunctorContinue::kContinue;
	});

	ctx.m_perThread[tid].m_sceneMin = ctx.m_perThread[tid].m_sceneMin.min(componentUpdateInfo.m_sceneMin);
	ctx.m_perThread[tid].m_sceneMax = ctx.m_perThread[tid].m_sceneMax.max(componentUpdateInfo.m_sceneMax);
}

void SceneGraph::updateNodes(U32 tid, UpdateSceneNodesCtx& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(SceneNodeUpdate);

	while(1)
	{
		// Fetch a batch of root scene nodes
		constexpr U32 batchMaxSize = ANKI_CACHE_LINE_SIZE / sizeof(void*); // Read a cacheline worth of SceneNodeEntry
		Array<SceneNode*, batchMaxSize> batch;
		U32 batchSize = 0;

		const U32 entryIndex = ctx.m_crntNodeIndex.fetchAdd(batchMaxSize);
		if(entryIndex > ctx.m_lastNodeIndex)
		{
			break;
		}

		for(U32 i = entryIndex; i < min(entryIndex + batchMaxSize, ctx.m_lastNodeIndex + 1); ++i)
		{
			if(m_updatableNodes.indexExists(i))
			{
				batch[batchSize++] = m_updatableNodes[i];
			}
		}

		// Process nodes
		for(U32 i = 0; i < batchSize; ++i)
		{
			SceneNode& node = *batch[i];
			ANKI_ASSERT(node.getParent() == nullptr);
			if(node.isMarkedForDeletion()) [[unlikely]]
			{
				ctx.m_perThread[tid].m_nodesForDeletion.emplaceBack(&node);
			}
			else
			{
				updateNode(tid, *batch[i], ctx);
			}
		}
	}
}

const SceneNode& SceneGraph::getActiveCameraNode() const
{
	forbidCallOnUpdate();
	ANKI_ASSERT(m_mainCam);
	if(ANKI_EXPECT(m_mainCam->hasComponent<CameraComponent>()) && !m_paused)
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
	forbidCallOnUpdate();
	if(cam)
	{
		m_mainCam = cam;
	}
	else
	{
		m_mainCam = m_defaultMainCam;
	}
}

void SceneGraph::countSerializableNodes(SceneNode& root, U32& serializableNodeCount)
{
	if(root.getSerialization())
	{
		++serializableNodeCount;

		root.visitChildrenMaxDepth(0, [&](SceneNode& child) {
			countSerializableNodes(child, serializableNodeCount);
			return FunctorContinue::kContinue;
		});
	}
}

Error SceneGraph::saveScene(CString filename, Scene& scene)
{
	ANKI_TRACE_FUNCTION();
	forbidCallOnUpdate();

	const U64 begin = HighRezTimer::getCurrentTimeUs();

	ANKI_LOGI("Saving scene: %s", filename.cstr());

	File file;
	ANKI_CHECK(file.open(filename, FileOpenFlag::kWrite));

	TextSceneSerializer serializer(&file);

	// Header
	SceneString magic = "ANKISCEN";
	ANKI_SERIALIZE(magic, 1);
	U32 version = kSceneBinaryVersion;
	ANKI_SERIALIZE(version, 1);

	SceneString sceneName = scene.m_name;
	ANKI_SERIALIZE(sceneName, 1);

	// Count the serializable nodes
	U32 serializableNodeCount = 0;
	scene.visitNodes([&](SceneNode& node) {
		if(node.getParent() == nullptr)
		{
			// Only root nodes, rest will be visited later
			countSerializableNodes(node, serializableNodeCount);
		}

		return FunctorContinue::kContinue;
	});

	// Scene nodes
	SceneNode::SerializeCommonArgs serializationArgs;
	ANKI_ASSERT(serializableNodeCount <= scene.m_nodes.getSize());
	U32 nodeCount = serializableNodeCount;
	ANKI_SERIALIZE(nodeCount, 1);

	Error err = Error::kNone;
	auto serializeNode = [&](SceneNode& node) -> Error {
		SceneString className = (node.getSceneNodeRegistryRecord()) ? node.getSceneNodeRegistryRecord()->m_name : "SceneNode";
		ANKI_SERIALIZE(className, 1);

		SceneString name = (node.getName() != "Unnamed") ? node.getName() : "";
		ANKI_SERIALIZE(name, 1);

		U32 uuid = node.getNodeUuid();
		ANKI_SERIALIZE(uuid, 1);

		ANKI_CHECK(node.serializeCommon(serializer, serializationArgs));
		ANKI_CHECK(node.serialize(serializer));

		--nodeCount;

		return Error::kNone;
	};

	scene.visitNodes([&](SceneNode& node) {
		if(node.getParent() != nullptr || !node.getSerialization())
		{
			// Skip non-root nodes (they will be visited later) and non-serializable nodes
			return FunctorContinue::kContinue;
		}

		node.visitThisAndChildren([&](SceneNode& node) {
			if(!node.getSerialization() || (err = serializeNode(node)))
			{
				return FunctorContinue::kStop;
			}

			return FunctorContinue::kContinue;
		});

		if(err)
		{
			return FunctorContinue::kStop;
		}

		return FunctorContinue::kContinue;
	});

	if(err)
	{
		return err;
	}

	ANKI_ASSERT(nodeCount == 0);

	// Components
	auto serializeComponent = [&](auto& comp) -> Error {
		if(!serializationArgs.m_write.m_serializableComponentMask[comp.getType()].getBit(comp.getArrayIndex()))
		{
			return Error::kNone;
		}

		ANKI_ASSERT(comp.getSerialization());

		U32 uuid = comp.getComponentUuid();
		ANKI_SERIALIZE(uuid, 1);

		ANKI_CHECK(comp.serialize(serializer));

		--serializationArgs.m_write.m_componentsToBeSerializedCount[comp.getType()];

		return Error::kNone;
	};

#define ANKI_DEFINE_SCENE_COMPONENT(name, weight, sceneNodeCanHaveMany, icon, serializable) \
	if(serializable) \
	{ \
		U32 name##Count = serializationArgs.m_write.m_componentsToBeSerializedCount[SceneComponentType::k##name]; \
		ANKI_SERIALIZE(name##Count, 1); \
		for(SceneComponent & comp : m_componentArrays.get##name##s()) \
		{ \
			ANKI_CHECK(serializeComponent(comp)); \
		} \
		ANKI_ASSERT(serializationArgs.m_write.m_componentsToBeSerializedCount[SceneComponentType::k##name] == 0); \
	}
#include <AnKi/Scene/Components/SceneComponentClasses.def.h>

	const F64 timeDiffMs = F64(HighRezTimer::getCurrentTimeUs() - begin) / 1000.0;
	ANKI_SCENE_LOGI("Saving scene finished. %fms", timeDiffMs);

	return Error::kNone;
}

Error SceneGraph::loadScene(CString filename, Scene*& scene)
{
	ANKI_ASSERT(scene == nullptr);

	ANKI_TRACE_FUNCTION();
	forbidCallOnUpdate();

	const U64 begin = HighRezTimer::getCurrentTimeUs();

	ANKI_LOGI("Loading scene: %s", filename.cstr());

	String extension;
	getFilepathExtension(filename, extension);

	if(extension == "lua")
	{
		scene = newEmptyScene("Level");
		const U32 oldActiveScene = m_activeSceneIndex;
		setActiveScene(scene);

		ScriptResourcePtr script;
		ANKI_CHECK(ResourceManager::getSingleton().loadResource(filename, script));
		ANKI_CHECK(ScriptManager::getSingleton().evalString(script->getSource()));

		ANKI_SCENE_LOGI("Loading scene finished. %fms", F64(HighRezTimer::getCurrentTimeUs() - begin) / 1000.0);
		setActiveScene(&m_scenes[oldActiveScene]);

		return Error::kNone;
	}

	ResourceFilePtr file;
	ANKI_CHECK(ResourceFilesystem::getSingleton().openFile(filename, file));

	TextSceneSerializer serializer(file.get());

	// Header
	SceneString magic;
	ANKI_SERIALIZE(magic, 1);
	if(magic != "ANKISCEN")
	{
		ANKI_LOGE("Wrong magic value");
		return Error::kUserData;
	}

	U32 version = 0;
	ANKI_SERIALIZE(version, 1);
	if(version > kSceneBinaryVersion)
	{
		ANKI_LOGE("Wrong version number");
		return Error::kUserData;
	}

	SceneString sceneName;
	ANKI_SERIALIZE(sceneName, 1);
	scene = newEmptyScene(sceneName);

	// Scene nodes
	SceneNode::SerializeCommonArgs serializationArgs;
	U32 nodeCount = 0;
	ANKI_SERIALIZE(nodeCount, 1);

	for(U32 i = 0; i < nodeCount; ++i)
	{
		SceneString className;
		ANKI_SERIALIZE(className, 1);

		SceneNodeInitInfo initInf;

		SceneString name;
		ANKI_SERIALIZE(name, 1);
		initInf.m_name = name;

		U32 uuid;
		ANKI_SERIALIZE(uuid, 1);
		initInf.m_nodeUuid = uuid;
		initInf.m_sceneUuid = scene->m_sceneUuid;
		initInf.m_sceneIndex = scene->m_arrayIndex;

		SceneNode* node;
		if(className != "SceneNode")
		{
			// Derived SceneNode class
			const SceneNodeRegistryRecord& record = *static_cast<SceneNodeRegistryRecord*>(GlobalRegistry::getSingleton().findRecord(className));

			void* mem = SceneMemoryPool::getSingleton().allocate(record.m_getClassSizeCallback(), ANKI_SAFE_ALIGNMENT);
			record.m_constructCallback(mem, initInf);

			node = static_cast<SceneNode*>(mem);
		}
		else
		{
			node = newInstance<SceneNode>(SceneMemoryPool::getSingleton(), initInf);
		}

		ANKI_CHECK(node->serializeCommon(serializer, serializationArgs));
		ANKI_CHECK(node->serialize(serializer));

		node->m_sceneIndex = scene->m_arrayIndex;
		node->m_sceneUuid = scene->m_sceneUuid;

		m_deferredOps.m_nodesForRegistration.emplaceBack(node);
	}

	// Components
	auto serializeComponent = [&](auto& compArray) -> Error {
		U32 uuid;
		ANKI_SERIALIZE(uuid, 1);

		auto it2 = serializationArgs.m_read.m_componentUuidToNode.find(uuid);
		if(it2 == serializationArgs.m_read.m_componentUuidToNode.getEnd())
		{
			ANKI_SCENE_LOGE("Incorrect UUID");
			return Error::kUserData;
		}

		SceneComponentInitInfo initInf;
		initInf.m_node = *it2;
		initInf.m_componentUuid = uuid;
		initInf.m_sceneUuid = scene->m_sceneUuid;

		auto it = compArray.emplace(initInf);
		SceneComponent& comp = *it;
		comp.setArrayIndex(it.getArrayIndex());
		ANKI_CHECK(comp.serialize(serializer));

		initInf.m_node->addComponent(&comp);

		return Error::kNone;
	};

#define ANKI_DEFINE_SCENE_COMPONENT(name, weight, sceneNodeCanHaveMany, icon, serializable) \
	if(serializable) \
	{ \
		U32 name##Count = 0; \
		ANKI_SERIALIZE(name##Count, 1); \
		for(U32 i = 0; i < name##Count; ++i) \
		{ \
			ANKI_CHECK(serializeComponent(SceneGraph::getSingleton().getComponentArrays().get##name##s())); \
		} \
	}
#include <AnKi/Scene/Components/SceneComponentClasses.def.h>

	ANKI_SCENE_LOGI("Loading scene finished. %fms", F64(HighRezTimer::getCurrentTimeUs() - begin) / 1000.0);
	return Error::kNone;
}

void SceneGraph::doDeferredOperations()
{
	// Register new nodes
	for(SceneNode* node : m_deferredOps.m_nodesForRegistration)
	{
		// Add to dict if it has a name
		if(node->getName() != "Unnamed")
		{
			if(m_nodesDict.find(node->getName()) != m_nodesDict.getEnd())
			{
				ANKI_SCENE_LOGE("Node with the same name already exists. New node will not be searchable: %s", node->getName().cstr());
			}
			else
			{
				m_nodesDict.emplace(node->getName(), node);
			}
		}

		// Add to scene
		ANKI_ASSERT(node->getParent() == nullptr && "New nodes can't have a parent yet");
		auto it = m_scenes[node->m_sceneIndex].m_nodes.emplace(node);
		node->m_nodeArrayIndex = it.getArrayIndex();

		// Add to updatable
		if(node->getParent() == nullptr)
		{
			auto it2 = m_updatableNodes.emplace(node);
			node->m_updatableNodesArrayIndex = it2.getArrayIndex();
		}
		else
		{
			ANKI_ASSERT(node->m_updatableNodesArrayIndex == kMaxU32);
		}
	}
	m_deferredOps.m_nodesForRegistration.destroy();

	// Renaming
	for(auto& pair : m_deferredOps.m_nodesRenamed)
	{
		SceneNode& node = *pair.first;
		CString oldName = pair.second;

		auto it = m_nodesDict.find(oldName);
		if(it != m_nodesDict.getEnd())
		{
			m_nodesDict.erase(it);
		}
		else
		{
			ANKI_ASSERT(oldName == "Unnamed" && "Didn't found the SceneNode and its name wasn't unnamed");
		}

		if(node.getName() != "Unnamed")
		{
			if(m_nodesDict.find(node.getName()) != m_nodesDict.getEnd())
			{
				ANKI_SCENE_LOGE("Node with the same name already exists. New node will not be searchable: %s", node.getName().cstr());
			}
			else
			{
				m_nodesDict.emplace(node.getName(), &node);
			}
		}
	}
	m_deferredOps.m_nodesRenamed.destroy();

	// Hierarchy changes
	for(auto& pair : m_deferredOps.m_nodesParentChanged)
	{
		SceneNode* child = pair.first;
		SceneNode* parent = pair.second;

		if(child->getParent() == nullptr)
		{
			ANKI_ASSERT(m_updatableNodes[child->m_updatableNodesArrayIndex] == child);
			m_updatableNodes.erase(child->m_updatableNodesArrayIndex);
			child->m_updatableNodesArrayIndex = kMaxU32;
		}
		else
		{
			child->removeParent();
		}

		if(parent)
		{
			// Add new parent
			static_cast<SceneNode::Base*>(child)->setParent(parent);
		}
		else
		{
			auto it = m_updatableNodes.emplace(child);
			child->m_updatableNodesArrayIndex = it.getArrayIndex();
		}
	}
	m_deferredOps.m_nodesParentChanged.destroy();
}

Scene* SceneGraph::newEmptyScene(CString name)
{
	forbidCallOnUpdate();

	if(!name)
	{
		name = "Unnamed";
	}

	if(tryFindScene(name))
	{
		ANKI_SCENE_LOGE("Scene found with the same name %s", name.cstr());
		return nullptr;
	}

	auto it = m_scenes.emplace();
	it->m_name = name;
	it->m_arrayIndex = U8(it.getArrayIndex());

	return &(*it);
}

Scene* SceneGraph::tryFindScene(CString name)
{
	for(Scene& other : m_scenes)
	{
		if(other.m_name == name)
		{
			return &other;
		}
	}

	return nullptr;
}

void SceneGraph::setActiveScene(Scene* scene)
{
	ANKI_ASSERT(scene);
	forbidCallOnUpdate();
	m_activeSceneIndex = scene->m_arrayIndex;
}

void SceneGraph::deleteScene(Scene* scene)
{
	ANKI_ASSERT(scene);
	forbidCallOnUpdate();

	const U64 begin = HighRezTimer::getCurrentTimeUs();

	if(scene->m_arrayIndex == m_activeSceneIndex)
	{
		m_activeSceneIndex = tryFindScene("_DefaultScene")->m_arrayIndex;
	}

	for(SceneNode* node : scene->m_nodes)
	{
		if(node->m_updatableNodesArrayIndex != kMaxU32)
		{
			m_updatableNodes.erase(node->m_updatableNodesArrayIndex);
		}

		if(node->getName() != "Unnamed")
		{
			auto it = m_nodesDict.find(node->getName());
			ANKI_ASSERT(it != m_nodesDict.getEnd());
			m_nodesDict.erase(it);
		}

		deleteInstance(SceneMemoryPool::getSingleton(), node);
	}

	m_scenes.erase(scene->m_arrayIndex);

	ANKI_SCENE_LOGI("Unloaded scene %s in %fms", scene->m_name.cstr(), F64(HighRezTimer::getCurrentTimeUs() - begin) / 1000.0);
}

} // end namespace anki
