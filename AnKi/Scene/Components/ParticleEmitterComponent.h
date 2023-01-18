// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Resource/ParticleEmitterResource.h>
#include <AnKi/Collision/Aabb.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

// Forward
class RenderQueueDrawContext;
class RenderableQueueElement;

/// @addtogroup scene
/// @{

/// Interface for particle emitters.
class ParticleEmitterComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(ParticleEmitterComponent)

public:
	ParticleEmitterComponent(SceneNode* node);

	~ParticleEmitterComponent();

	Error loadParticleEmitterResource(CString filename);

	const Aabb& getAabbWorldSpace() const
	{
		return m_worldBoundingVolume;
	}

	Bool isEnabled() const
	{
		return m_particleEmitterResource.isCreated();
	}

	void setupRenderableQueueElements(RenderingTechnique technique, StackMemoryPool& tmpPool,
									  WeakArray<RenderableQueueElement>& outRenderables) const;

private:
	class ParticleBase;
	class SimpleParticle;
	class PhysicsParticle;

	enum class SimulationType : U8
	{
		kUndefined,
		kSimple,
		kPhysicsEngine
	};

	SceneNode* m_node = nullptr;

	MoveComponent* m_moveComponent = nullptr;

	ParticleEmitterProperties m_props;

	ParticleEmitterResourcePtr m_particleEmitterResource;
	DynamicArray<SimpleParticle> m_simpleParticles;
	DynamicArray<PhysicsParticle> m_physicsParticles;
	Second m_timeLeftForNextEmission = 0.0;
	U32 m_aliveParticleCount = 0;
	Bool m_resourceUpdated = true;

	Aabb m_worldBoundingVolume = Aabb(Vec3(-1.0f), Vec3(1.0f));

	SegregatedListsGpuMemoryPoolToken m_gpuScenePositions;
	SegregatedListsGpuMemoryPoolToken m_gpuSceneAlphas;
	SegregatedListsGpuMemoryPoolToken m_gpuSceneScales;
	SegregatedListsGpuMemoryPoolToken m_gpuSceneParticles;
	SegregatedListsGpuMemoryPoolToken m_gpuSceneUniforms;

	SimulationType m_simulationType = SimulationType::kUndefined;

	Error update(SceneComponentUpdateInfo& info, Bool& updated);

	template<typename TParticle>
	void simulate(Second prevUpdateTime, Second crntTime, WeakArray<TParticle> particles, Vec3*& positions,
				  F32*& scales, F32*& alphas);

	void onOtherComponentRemovedOrAdded(SceneComponent* other, Bool added);
};
/// @}

} // end namespace anki
