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

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override;

	void setWorldTransform(const Transform& transform)
	{
		m_transform = transform;
	}

	const Aabb& getAabbWorldSpace() const
	{
		return m_worldBoundingVolume;
	}

	ParticleEmitterResourcePtr getParticleEmitterResource() const
	{
		return m_particleEmitterResource;
	}

	/// RenderComponent callback. The userData is the component.
	static void drawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData)
	{
		ANKI_ASSERT(userData.getSize() == 1);
		static_cast<const ParticleEmitterComponent*>(userData[0])->draw(ctx);
	}

	Bool isEnabled() const
	{
		return m_particleEmitterResource.isCreated();
	}

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

	static constexpr U32 kVertexSize = 5 * sizeof(F32);

	SceneNode* m_node = nullptr;

	ParticleEmitterProperties m_props;

	ParticleEmitterResourcePtr m_particleEmitterResource;
	DynamicArray<SimpleParticle> m_simpleParticles;
	DynamicArray<PhysicsParticle> m_physicsParticles;
	Second m_timeLeftForNextEmission = 0.0;
	U32 m_aliveParticleCount = 0;

	Transform m_transform = Transform::getIdentity();
	Aabb m_worldBoundingVolume = Aabb(Vec3(-1.0f), Vec3(1.0f));

	U32 m_vertBuffSize = 0;
	void* m_verts = nullptr;

	ImageResourcePtr m_dbgImage;

	SimulationType m_simulationType = SimulationType::kUndefined;

	template<typename TParticle>
	void simulate(Second prevUpdateTime, Second crntTime, WeakArray<TParticle> particles);

	void draw(RenderQueueDrawContext& ctx) const;
};
/// @}

} // end namespace anki
