// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Scene/Spatial.h>
#include <AnKi/Scene/RenderStateBucket.h>
#include <AnKi/Scene/GpuSceneArray.h>
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

	void setupRenderableQueueElements(RenderingTechnique technique, WeakArray<RenderableQueueElement>& outRenderables) const;

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

	ParticleEmitterProperties m_props;

	Spatial m_spatial;

	ParticleEmitterResourcePtr m_particleEmitterResource;
	SceneDynamicArray<SimpleParticle> m_simpleParticles;
	SceneDynamicArray<PhysicsParticle> m_physicsParticles;
	Second m_timeLeftForNextEmission = 0.0;
	U32 m_aliveParticleCount = 0;

	GpuSceneBufferAllocation m_gpuScenePositions;
	GpuSceneBufferAllocation m_gpuSceneAlphas;
	GpuSceneBufferAllocation m_gpuSceneScales;
	GpuSceneBufferAllocation m_gpuSceneUniforms;
	GpuSceneArrays::ParticleEmitter::Allocation m_gpuSceneParticleEmitter;
	GpuSceneArrays::Renderable::Allocation m_gpuSceneRenderable;
	GpuSceneArrays::RenderableAabbGBuffer::Allocation m_gpuSceneRenderableAabbGBuffer;
	GpuSceneArrays::RenderableAabbDepth::Allocation m_gpuSceneRenderableAabbDepth;
	GpuSceneArrays::RenderableAabbForward::Allocation m_gpuSceneRenderableAabbForward;

	Array<RenderStateBucketIndex, U32(RenderingTechnique::kCount)> m_renderStateBuckets;

	Bool m_resourceUpdated = true;
	SimulationType m_simulationType = SimulationType::kUndefined;

	Error update(SceneComponentUpdateInfo& info, Bool& updated);

	template<typename TParticle>
	void simulate(Second prevUpdateTime, Second crntTime, const Transform& worldTransform, WeakArray<TParticle> particles, Vec3*& positions,
				  F32*& scales, F32*& alphas, Aabb& aabbWorld);
};
/// @}

} // end namespace anki
