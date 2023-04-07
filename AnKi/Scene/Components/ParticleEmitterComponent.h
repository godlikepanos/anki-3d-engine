// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Scene/Spatial.h>
#include <AnKi/Resource/ParticleEmitterResource.h>
#include <AnKi/Collision/Aabb.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

// Forward
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

	void loadParticleEmitterResource(CString filename);

	Bool isEnabled() const
	{
		return m_particleEmitterResource.isCreated();
	}

	void setupRenderableQueueElements(RenderingTechnique technique,
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

	ParticleEmitterProperties m_props;

	Spatial m_spatial;

	ParticleEmitterResourcePtr m_particleEmitterResource;
	SceneDynamicArray<SimpleParticle> m_simpleParticles;
	SceneDynamicArray<PhysicsParticle> m_physicsParticles;
	Second m_timeLeftForNextEmission = 0.0;
	U32 m_aliveParticleCount = 0;

	SegregatedListsGpuMemoryPoolToken m_gpuScenePositions;
	SegregatedListsGpuMemoryPoolToken m_gpuSceneAlphas;
	SegregatedListsGpuMemoryPoolToken m_gpuSceneScales;
	SegregatedListsGpuMemoryPoolToken m_gpuSceneUniforms;
	U32 m_gpuSceneIndex = kMaxU32;

	Bool m_resourceUpdated = true;
	SimulationType m_simulationType = SimulationType::kUndefined;

	Error update(SceneComponentUpdateInfo& info, Bool& updated);

	template<typename TParticle>
	void simulate(Second prevUpdateTime, Second crntTime, WeakArray<TParticle> particles, Vec3*& positions,
				  F32*& scales, F32*& alphas, Aabb& aabbWorld);
};
/// @}

} // end namespace anki
