// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Common.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Math.h>
#include <AnKi/Util/HashMap.h>
#include <AnKi/Util/BlockArray.h>
#include <AnKi/Scene/Events/EventManager.h>
#include <AnKi/Resource/Common.h>
#include <AnKi/Util/CVarSet.h>
#include <AnKi/Core/Common.h>

namespace anki {

/// @addtogroup scene
/// @{

inline NumericCVar<F32> g_probeEffectiveDistanceCVar("Scene", "ProbeEffectiveDistance", 256.0f, 1.0f, kMaxF32, "How far various probes can render");
inline NumericCVar<F32> g_probeShadowEffectiveDistanceCVar("Scene", "ProbeShadowEffectiveDistance", 32.0f, 1.0f, kMaxF32,
														   "How far to render shadows for the various probes");

// Gpu scene arrays
inline NumericCVar<U32> g_minGpuSceneTransformsCVar("Scene", "MinGpuSceneTransforms", 2 * 10 * 1024, 8, 100 * 1024,
													"The min number of transforms stored in the GPU scene");
inline NumericCVar<U32> g_minGpuSceneMeshesCVar("Scene", "MinGpuSceneMeshes", 8 * 1024, 8, 100 * 1024,
												"The min number of meshes stored in the GPU scene");
inline NumericCVar<U32> g_minGpuSceneParticleEmittersCVar("Scene", "MinGpuSceneParticleEmitters", 1 * 1024, 8, 100 * 1024,
														  "The min number of particle emitters stored in the GPU scene");
inline NumericCVar<U32> g_minGpuSceneLightsCVar("Scene", "MinGpuSceneLights", 2 * 1024, 8, 100 * 1024,
												"The min number of lights stored in the GPU scene");
inline NumericCVar<U32> g_minGpuSceneReflectionProbesCVar("Scene", "MinGpuSceneReflectionProbes", 128, 8, 100 * 1024,
														  "The min number of reflection probes stored in the GPU scene");
inline NumericCVar<U32> g_minGpuSceneGlobalIlluminationProbesCVar("Scene", "MinGpuSceneGlobalIlluminationProbes", 128, 8, 100 * 1024,
																  "The min number of GI probes stored in the GPU scene");
inline NumericCVar<U32> g_minGpuSceneDecalsCVar("Scene", "MinGpuSceneDecals", 2 * 1024, 8, 100 * 1024,
												"The min number of decals stored in the GPU scene");
inline NumericCVar<U32> g_minGpuSceneFogDensityVolumesCVar("Scene", "MinGpuSceneFogDensityVolumes", 512, 8, 100 * 1024,
														   "The min number fog density volumes stored in the GPU scene");
inline NumericCVar<U32> g_minGpuSceneRenderablesCVar("Scene", "MinGpuSceneRenderables", 10 * 1024, 8, 100 * 1024,
													 "The min number of renderables stored in the GPU scene");

class SceneComponentArrays
{
public:
#define ANKI_DEFINE_SCENE_COMPONENT(name, weight) \
	SceneBlockArray<name##Component>& get##name##s() \
	{ \
		return m_##name##Array; \
	}
#include <AnKi/Scene/Components/SceneComponentClasses.def.h>

private:
#define ANKI_DEFINE_SCENE_COMPONENT(name, weight) SceneBlockArray<name##Component> m_##name##Array;
#include <AnKi/Scene/Components/SceneComponentClasses.def.h>
};

/// The scene graph that  all the scene entities
class SceneGraph : public MakeSingleton<SceneGraph>
{
	template<typename>
	friend class MakeSingleton;

	friend class SceneNode;
	friend class UpdateSceneNodesTask;
	friend class Event;

public:
	Error init(AllocAlignedCallback allocCallback, void* allocCallbackData);

	StackMemoryPool& getFrameMemoryPool() const
	{
		return m_framePool;
	}

	SceneNode& getActiveCameraNode()
	{
		ANKI_ASSERT(m_mainCam != nullptr);
		return *m_mainCam;
	}
	const SceneNode& getActiveCameraNode() const
	{
		return *m_mainCam;
	}
	void setActiveCameraNode(SceneNode* cam)
	{
		m_mainCam = cam;
		m_activeCameraChangeTimestamp = GlobalFrameIndex::getSingleton().m_value;
	}
	Timestamp getActiveCameraNodeChangeTimestamp() const
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

	Error update(Second prevUpdateTime, Second crntTime);

	SceneNode& findSceneNode(const CString& name);
	SceneNode* tryFindSceneNode(const CString& name);

	/// Iterate the scene nodes using a lambda
	template<typename Func>
	Error iterateSceneNodes(Func func)
	{
		for(SceneNode& psn : m_nodes)
		{
			Error err = func(psn);
			if(err)
			{
				return err;
			}
		}

		return Error::kNone;
	}

	/// Iterate a range of scene nodes using a lambda
	template<typename Func>
	Error iterateSceneNodes(PtrSize begin, PtrSize end, Func func);

	/// Create a new SceneNode
	template<typename Node, typename... Args>
	Error newSceneNode(const CString& name, Node*& node, Args&&... args);

	/// Delete a scene node. It actualy marks it for deletion
	void deleteSceneNode(SceneNode* node)
	{
		node->setMarkedForDeletion();
	}

	void increaseObjectsMarkedForDeletion()
	{
		m_objectsMarkedForDeletionCount.fetchAdd(1);
	}

	const Vec3& getSceneMin() const
	{
		return m_sceneMin;
	}

	const Vec3& getSceneMax() const
	{
		return m_sceneMax;
	}

	/// Get a unique UUID.
	/// @note It's thread-safe.
	U32 getNewUuid()
	{
		return m_nodesUuid.fetchAdd(1);
	}

	SceneComponentArrays& getComponentArrays()
	{
		return m_componentArrays;
	}

	void addDirectionalLight(LightComponent* comp)
	{
		ANKI_ASSERT(m_dirLights.find(comp) == m_dirLights.getEnd());
		m_dirLights.emplaceBack(comp);
		if(m_dirLights.getSize() > 1)
		{
			ANKI_SCENE_LOGW("More than one directional lights detected");
		}
	}

	void removeDirectionalLight(LightComponent* comp)
	{
		auto it = m_dirLights.find(comp);
		ANKI_ASSERT(it != m_dirLights.getEnd());
		m_dirLights.erase(it);
	}

	LightComponent* getDirectionalLight() const;

	void addSkybox(SkyboxComponent* comp)
	{
		ANKI_ASSERT(m_skyboxes.find(comp) == m_skyboxes.getEnd());
		m_skyboxes.emplaceBack(comp);
		if(m_skyboxes.getSize() > 1)
		{
			ANKI_SCENE_LOGW("More than one skyboxes detected");
		}
	}

	void removeSkybox(SkyboxComponent* comp)
	{
		auto it = m_skyboxes.find(comp);
		ANKI_ASSERT(it != m_skyboxes.getEnd());
		m_skyboxes.erase(it);
	}

	SkyboxComponent* getSkybox() const
	{
		return (m_skyboxes.getSize()) ? m_skyboxes[0] : nullptr;
	}

	/// @note It's thread-safe.
	void updateSceneBounds(const Vec3& min, const Vec3& max)
	{
		LockGuard lock(m_sceneBoundsMtx);
		m_sceneMin = m_sceneMin.min(min);
		m_sceneMax = m_sceneMax.max(max);
	}

	/// @note It's thread-safe.
	Array<Vec3, 2> getSceneBounds() const
	{
		LockGuard lock(m_sceneBoundsMtx);
		ANKI_ASSERT(m_sceneMin < m_sceneMax);
		return {m_sceneMin, m_sceneMax};
	}

private:
	class UpdateSceneNodesCtx;

	class InitMemPoolDummy
	{
	public:
		~InitMemPoolDummy()
		{
			SceneMemoryPool::freeSingleton();
		}
	} m_initMemPoolDummy;

	mutable StackMemoryPool m_framePool;

	IntrusiveList<SceneNode> m_nodes;
	U32 m_nodesCount = 0;
	GrHashMap<CString, SceneNode*> m_nodesDict;

	SceneNode* m_mainCam = nullptr;
	Timestamp m_activeCameraChangeTimestamp = 0;
	SceneNode* m_defaultMainCam = nullptr;

	EventManager m_events;

	Vec3 m_sceneMin = Vec3(kMaxF32);
	Vec3 m_sceneMax = Vec3(kMinF32);
	mutable SpinLock m_sceneBoundsMtx;

	Atomic<U32> m_objectsMarkedForDeletionCount = {0};

	Atomic<U32> m_nodesUuid = {1};

	SceneComponentArrays m_componentArrays;

	SceneDynamicArray<LightComponent*> m_dirLights;
	SceneDynamicArray<SkyboxComponent*> m_skyboxes;

	SceneGraph();

	~SceneGraph();

	/// Put a node in the appropriate containers
	Error registerNode(SceneNode* node);
	void unregisterNode(SceneNode* node);

	/// Delete the nodes that are marked for deletion
	void deleteNodesMarkedForDeletion();

	Error updateNodes(UpdateSceneNodesCtx& ctx);
	Error updateNode(Second prevTime, Second crntTime, SceneNode& node);
};

template<typename Node, typename... Args>
inline Error SceneGraph::newSceneNode(const CString& name, Node*& node, Args&&... args)
{
	Error err = Error::kNone;

	node = newInstance<Node>(SceneMemoryPool::getSingleton(), name);
	if(node)
	{
		err = node->init(std::forward<Args>(args)...);
	}
	else
	{
		err = Error::kOutOfMemory;
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
			deleteInstance(SceneMemoryPool::getSingleton(), node);
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
	Error err = Error::kNone;
	while(count-- != 0 && !err)
	{
		ANKI_ASSERT(it != m_nodes.getEnd());
		err = func(*it);

		++it;
	}

	return Error::kNone;
}
/// @}

} // end namespace anki
