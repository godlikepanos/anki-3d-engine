// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/components/SceneComponent.h>
#include <anki/resource/ParticleEmitterResource.h>
#include <anki/collision/Aabb.h>
#include <anki/util/WeakArray.h>

namespace anki
{

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

	ANKI_USE_RESULT Error loadParticleEmitterResource(CString filename);

	Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override;

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
	static void drawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData);

private:
	class ParticleBase;
	class SimpleParticle;
	class PhysicsParticle;

	enum class SimulationType : U8
	{
		UNDEFINED,
		SIMPLE,
		PHYSICS_ENGINE
	};

	static constexpr U32 VERTEX_SIZE = 5 * sizeof(F32);

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

	SimulationType m_simulationType = SimulationType::UNDEFINED;

	template<typename TParticle>
	void simulate(Second prevUpdateTime, Second crntTime, WeakArray<TParticle> particles);
};
/// @}

} // end namespace anki
