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

ANKI_CVAR(NumericCVar<F32>, Scene, ProbeEffectiveDistance, 256.0f, 1.0f, kMaxF32, "How far various probes can render")
ANKI_CVAR(NumericCVar<F32>, Scene, ProbeShadowEffectiveDistance, 32.0f, 1.0f, kMaxF32, "How far to render shadows for the various probes")

// Gpu scene arrays
ANKI_CVAR(NumericCVar<U32>, Scene, MinGpuSceneTransforms, 2 * 10 * 1024, 8, 100 * 1024, "The min number of transforms stored in the GPU scene")
ANKI_CVAR(NumericCVar<U32>, Scene, MinGpuSceneMeshes, 8 * 1024, 8, 100 * 1024, "The min number of meshes stored in the GPU scene")
ANKI_CVAR(NumericCVar<U32>, Scene, MinGpuSceneParticleEmitters, 1 * 1024, 8, 100 * 1024,
		  "The min number of particle emitters stored in the GPU scene")
ANKI_CVAR(NumericCVar<U32>, Scene, MinGpuSceneLights, 2 * 1024, 8, 100 * 1024, "The min number of lights stored in the GPU scene")
ANKI_CVAR(NumericCVar<U32>, Scene, MinGpuSceneReflectionProbes, 128, 8, 100 * 1024, "The min number of reflection probes stored in the GPU scene")
ANKI_CVAR(NumericCVar<U32>, Scene, MinGpuSceneGlobalIlluminationProbes, 128, 8, 100 * 1024, "The min number of GI probes stored in the GPU scene")
ANKI_CVAR(NumericCVar<U32>, Scene, MinGpuSceneDecals, 2 * 1024, 8, 100 * 1024, "The min number of decals stored in the GPU scene")
ANKI_CVAR(NumericCVar<U32>, Scene, MinGpuSceneFogDensityVolumes, 512, 8, 100 * 1024, "The min number fog density volumes stored in the GPU scene")
ANKI_CVAR(NumericCVar<U32>, Scene, MinGpuSceneRenderables, 10 * 1024, 8, 100 * 1024, "The min number of renderables stored in the GPU scene")

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

	void setActiveCameraNode(SceneNode* cam);

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

	void update(Second prevUpdateTime, Second crntTime);

	/// @note Thread-safe against itself. Can be called by SceneNode::update
	SceneNode* tryFindSceneNode(const CString& name);

	/// @note Thread-safe against itself. Can be called by SceneNode::update
	SceneNode& findSceneNode(const CString& name);

	/// Iterate the scene nodes using a lambda
	template<typename Func>
	void iterateSceneNodes(Func func)
	{
		for(SceneNode& psn : m_nodes)
		{
			const Bool continue_ = func(psn);
			if(!continue_)
			{
				break;
			}
		}
	}

	/// Create a new SceneNode
	/// @note Thread-safe against itself. Can be called by SceneNode::update
	template<typename TNode>
	TNode* newSceneNode(CString name)
	{
		TNode* node = newInstance<TNode>(SceneMemoryPool::getSingleton(), name);
		LockGuard lock(m_nodesForRegistrationMtx);
		m_nodesForRegistration.pushBack(node);
		return node;
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
	SceneNode* m_defaultMainCam = nullptr;

	EventManager m_events;

	Vec3 m_sceneMin = Vec3(kMaxF32);
	Vec3 m_sceneMax = Vec3(kMinF32);
	mutable SpinLock m_sceneBoundsMtx;

	IntrusiveList<SceneNode> m_nodesForRegistration;
	SpinLock m_nodesForRegistrationMtx;

	Atomic<U32> m_nodesUuid = {1};

	SceneComponentArrays m_componentArrays;

	SceneDynamicArray<LightComponent*> m_dirLights;
	SceneDynamicArray<SkyboxComponent*> m_skyboxes;

	SceneGraph();

	~SceneGraph();

	void updateNodes(UpdateSceneNodesCtx& ctx);
	void updateNode(Second prevTime, Second crntTime, SceneNode& node);
};
/// @}

} // end namespace anki
