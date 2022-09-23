// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Collision/Aabb.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Resource/ParticleEmitterResource.h>

namespace anki {

/// @addtogroup scene
/// @{

/// GPU particle emitter.
class GpuParticleEmitterComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(GpuParticleEmitterComponent)

public:
	GpuParticleEmitterComponent(SceneNode* node);

	~GpuParticleEmitterComponent();

	Error loadParticleEmitterResource(CString filename);

	void setWorldTransform(const Transform& trf);

	ParticleEmitterResourcePtr getParticleEmitterResource() const
	{
		return m_particleEmitterResource;
	}

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override;

	/// Callback that will be used by the GenericGpuComputeJobComponent
	static void simulateCallback(GenericGpuComputeJobQueueElementContext& ctx, const void* userData)
	{
		ANKI_ASSERT(userData);
		static_cast<const GpuParticleEmitterComponent*>(userData)->simulate(ctx);
	}

	/// Callback that will be used by the RenderComponent
	static void drawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData)
	{
		ANKI_ASSERT(userData.getSize() == 1);
		static_cast<const GpuParticleEmitterComponent*>(userData[0])->draw(ctx);
	}

	const Aabb& getBoundingVolumeWorldSpace() const
	{
		return m_worldAabb;
	}

	Bool isEnabled() const
	{
		return m_particleEmitterResource.isCreated();
	}

private:
	static constexpr U32 kMaxRandFactors = 32;

	SceneNode* m_node;
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;
	U32 m_workgroupSizeX = 0;

	ParticleEmitterResourcePtr m_particleEmitterResource;

	BufferPtr m_propsBuff; ///< Constant buffer with particle properties.
	BufferPtr m_particlesBuff; ///< Particles buffer.
	BufferPtr m_randFactorsBuff; ///< Contains flots with random values. Values in range [0.0, 1.0].

	SamplerPtr m_nearestAnyClampSampler;

	Aabb m_emitterBoundingBoxLocal = Aabb(Vec3(0.0f), Vec3(1.0f));
	U32 m_maxParticleCount = 0;
	Second m_dt = 0.0;
	Vec3 m_worldPosition = Vec3(0.0f); //< Cache it.
	Mat3x4 m_worldRotation = Mat3x4::getIdentity();
	Aabb m_worldAabb = Aabb(Vec3(0.0f), Vec3(1.0f));

	ImageResourcePtr m_dbgImage;

	Bool m_markedForUpdate = true;

	void simulate(GenericGpuComputeJobQueueElementContext& ctx) const;
	void draw(RenderQueueDrawContext& ctx) const;
};
/// @}

} // end namespace anki
