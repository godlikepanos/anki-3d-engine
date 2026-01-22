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
#define ANKI_DEFINE_SCENE_COMPONENT(name, weight, sceneNodeCanHaveMany, icon, serializable) \
	SceneBlockArray<name##Component>& get##name##s() \
	{ \
		return m_##name##Array; \
	} \
	const SceneBlockArray<name##Component>& get##name##s() const \
	{ \
		return m_##name##Array; \
	}
#include <AnKi/Scene/Components/SceneComponentClasses.def.h>

private:
#define ANKI_DEFINE_SCENE_COMPONENT(name, weight, sceneNodeCanHaveMany, icon, serializable) SceneBlockArray<name##Component> m_##name##Array;
#include <AnKi/Scene/Components/SceneComponentClasses.def.h>
};

// The scenegraph consists of multiple scenes. Scenes are containers of nodes.
class Scene
{
	friend class SceneGraph;

public:
	Scene()
		: m_sceneUuid(m_scenesUuid.fetchAdd(1))
	{
	}

	CString getName() const
	{
		return m_name;
	}

	template<typename TFunc>
	FunctorContinue visitNodes(TFunc func)
	{
		FunctorContinue cont = FunctorContinue::kContinue;
		for(SceneNode* node : m_nodes)
		{
			cont = func(*node);
			if(cont == FunctorContinue::kStop)
			{
				break;
			}
		}
		return cont;
	}

	template<typename TFunc>
	FunctorContinue visitNodes(TFunc func) const
	{
		FunctorContinue cont = FunctorContinue::kContinue;
		for(const SceneNode* node : m_nodes)
		{
			cont = func(*node);
			if(cont == FunctorContinue::kStop)
			{
				break;
			}
		}
		return cont;
	}

	Bool isEmpty() const
	{
		return m_nodes.getSize() == 0;
	}

	ANKI_INTERNAL U32 getNewNodeUuid() const
	{
		return m_nodesUuid.fetchAdd(1);
	}

	ANKI_INTERNAL U32 getSceneUuid() const
	{
		ANKI_ASSERT(m_sceneUuid);
		return m_sceneUuid;
	}

private:
	SceneBlockArray<SceneNode*> m_nodes;

	mutable Atomic<U32> m_nodesUuid = {1};

	SceneString m_name;

	static inline Atomic<U32> m_scenesUuid = {1};
	U32 m_sceneUuid = 0;

	U8 m_arrayIndex = kMaxU8; // Index in SceneGraph::m_scenes

	Bool m_immutable = false; // Can't add or remove nodes from it
};

// The scene graph that  all the scene entities
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
		forbidCallOnUpdate();
		const SceneNode& cam = static_cast<const SceneGraph*>(this)->getActiveCameraNode();
		return const_cast<SceneNode&>(cam);
	}

	const SceneNode& getActiveCameraNode() const;

	void setActiveCameraNode(SceneNode* cam);

	EventManager& getEventManager()
	{
		return m_events;
	}
	const EventManager& getEventManager() const
	{
		return m_events;
	}

	void update(Second prevUpdateTime, Second crntTime);

	// Scene manipulation //

	template<typename TFunc>
	void visitScenes(TFunc func)
	{
		for(Scene& scene : m_scenes)
		{
			if(func(scene) == FunctorContinue::kStop)
			{
				break;
			}
		}
	}

	Scene* newEmptyScene(CString name);

	Scene* tryFindScene(CString name);

	void setActiveScene(Scene* scene);

	Scene& getActiveScene()
	{
		return m_scenes[m_activeSceneIndex];
	}

	const Scene& getActiveScene() const
	{
		return m_scenes[m_activeSceneIndex];
	}

	Error saveScene(CString filename, Scene& scene);

	Error loadScene(CString filename, Scene*& scene);

	void deleteScene(Scene* scene);

	// End scene manipulation //

	// Note: Thread-safe against itself. Can be called by SceneNode::update
	SceneNode* tryFindSceneNode(const CString& name);

	// Note: Thread-safe against itself. Can be called by SceneNode::update
	SceneNode& findSceneNode(const CString& name);

	// Iterate the scene nodes using a lambda
	template<typename Func>
	FunctorContinue visitNodes(Func func)
	{
		FunctorContinue cont = FunctorContinue::kContinue;
		for(Scene& scene : m_scenes)
		{
			cont = scene.visitNodes(func);
			if(cont == FunctorContinue::kStop)
			{
				break;
			}
		}
		return cont;
	}

	// Create a new SceneNode
	// Note: Thread-safe against itself. Can be called by SceneNode::update
	// name: The unique name of the node. If it's empty the the node is not searchable.
	template<typename TNode>
	TNode* newSceneNode(CString name)
	{
		TNode* node = nullptr;
		if(!m_scenes[m_activeSceneIndex].m_immutable)
		{
			SceneNodeInitInfo inf;
			inf.m_name = name;
			ANKI_SILENCE_INTERNAL_BEGIN
			inf.m_nodeUuid = m_scenes[m_activeSceneIndex].getNewNodeUuid();
			ANKI_SILENCE_INTERNAL_END
			inf.m_sceneIndex = m_activeSceneIndex;
			inf.m_sceneUuid = m_scenes[m_activeSceneIndex].m_sceneUuid;
			node = newInstance<TNode>(SceneMemoryPool::getSingleton(), inf);
			LockGuard lock(m_deferredOps.m_mtx);
			m_deferredOps.m_nodesForRegistration.emplaceBack(node);
		}
		else
		{
			ANKI_SCENE_LOGE("Can't create new nodes to the immutable scene: %s", m_scenes[m_activeSceneIndex].m_name.cstr());
		}
		return node;
	}

	const Vec3& getSceneMin() const
	{
		forbidCallOnUpdate();
		return m_sceneMin;
	}

	const Vec3& getSceneMax() const
	{
		forbidCallOnUpdate();
		return m_sceneMax;
	}

	SceneComponentArrays& getComponentArrays()
	{
		forbidCallOnUpdate();
		return m_componentArrays;
	}

	LightComponent* getDirectionalLight() const
	{
		forbidCallOnUpdate();
		return m_activeDirLight;
	}

	SkyboxComponent* getSkybox() const
	{
		forbidCallOnUpdate();
		return m_activeSkybox;
	}

	Array<Vec3, 2> getSceneBounds() const
	{
		forbidCallOnUpdate();
		ANKI_ASSERT(m_sceneMin < m_sceneMax);
		return {m_sceneMin, m_sceneMax};
	}

	// Pause the game loop
	void pause(Bool pause_)
	{
		forbidCallOnUpdate();
		m_paused = pause_;
	}

	Bool isPaused() const
	{
		forbidCallOnUpdate();
		return m_paused;
	}

	// If enable is true the components will be checking for updates of resources. Useful for the editor resource updates. It has a perf hit so it
	// should be enabled only by the editor
	void setCheckForResourceUpdates(Bool enable)
	{
		forbidCallOnUpdate();
		m_checkForResourceUpdates = enable;
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

	static constexpr U32 kForceSetSceneBoundsFrameCount = 60 * 2; // Re-set the scene bounds after 2".

	mutable StackMemoryPool m_framePool;

	BlockArray<Scene, SingletonMemoryPoolWrapper<SceneMemoryPool>, BlockArrayConfig<4>> m_scenes;
	U8 m_activeSceneIndex = 0;

	SceneBlockArray<SceneNode*> m_updatableNodes;

	GrHashMap<CString, SceneNode*> m_nodesDict;

	SceneNode* m_mainCamNode = nullptr;
	SceneNode* m_defaultMainCamNode = nullptr;
	SceneNode* m_editorUiNode = nullptr;

	EventManager m_events;

	U64 m_frame = 0;

	Vec3 m_sceneMin = Vec3(-0.1f);
	Vec3 m_sceneMax = Vec3(+0.1f);

	// These are operations that might happen in threads and they need to be processed when there are no other threads running
	class
	{
	public:
		SceneDynamicArray<SceneNode*> m_nodesForRegistration;
		SceneDynamicArray<std::pair<SceneNode*, SceneString>> m_nodesRenamed;
		SceneDynamicArray<std::pair<SceneNode*, SceneNode*>> m_nodesParentChanged;
		SpinLock m_mtx;
	} m_deferredOps;

	Bool m_checkForResourceUpdates = false; // If true the components will have to re-check their resources for updates

	Bool m_paused = true; // If true the game-loop is paused

	SceneComponentArrays m_componentArrays;

	LightComponent* m_activeDirLight = nullptr;
	SkyboxComponent* m_activeSkybox = nullptr;

#if ANKI_ASSERTIONS_ENABLED
	volatile Bool m_inUpdate = false;
#endif

	SceneGraph();

	~SceneGraph();

	void updateNodes(U32 tid, UpdateSceneNodesCtx& ctx);
	void updateNode(U32 tid, SceneNode& node, UpdateSceneNodesCtx& ctx);

	// Begin deferred operations //
	void sceneNodeChangedNameDeferred(SceneNode& node, CString oldName)
	{
		LockGuard lock(m_deferredOps.m_mtx);
		m_deferredOps.m_nodesRenamed.emplaceBack(std::pair(&node, oldName));
	}

	void setSceneNodeParentDeferred(SceneNode* node, SceneNode* parent)
	{
		LockGuard lock(m_deferredOps.m_mtx);
		m_deferredOps.m_nodesParentChanged.emplaceBack(std::pair(node, parent));
	}

	void doDeferredOperations();
	// End deferred operations //

	static void countSerializableNodes(SceneNode& root, U32& serializableNodeCount);

	void forbidCallOnUpdate() const
	{
		ANKI_ASSERT(!m_inUpdate && "Did a illegal function call from update()");
	}
};

} // end namespace anki
