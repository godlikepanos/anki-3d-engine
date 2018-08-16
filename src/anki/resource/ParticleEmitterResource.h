// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/ResourceObject.h>
#include <anki/resource/RenderingKey.h>
#include <anki/Math.h>
#include <anki/Gr.h>

namespace anki
{

class XmlElement;

/// @addtogroup resource
/// @{

/// The particle emitter properties. Different class from ParticleEmitterResource so it can be inherited
class ParticleEmitterProperties
{
public:
	ParticleEmitterProperties()
	{
	}

	ParticleEmitterProperties(const ParticleEmitterProperties& b)
	{
		*this = b;
	}

	ParticleEmitterProperties& operator=(const ParticleEmitterProperties& b);

	/// @name Particle specific properties
	/// @{
	class
	{
	public:
		Second m_minLife = 10.0;
		Second m_maxLife = 10.0;

		F32 m_minMass = 1.0f;
		F32 m_maxMass = 1.0f;

		F32 m_minInitialSize = 1.0f;
		F32 m_maxInitialSize = 1.0f;
		F32 m_minFinalSize = 1.0f;
		F32 m_maxFinalSize = 1.0f;

		F32 m_minInitialAlpha = 1.0f;
		F32 m_maxInitialAlpha = 1.0f;
		F32 m_minFinalAlpha = 1.0f;
		F32 m_maxFinalAlpha = 1.0f;

		Vec3 m_minForceDirection = Vec3(0.0f, 1.0f, 0.0f);
		Vec3 m_maxForceDirection = Vec3(0.0f, 1.0f, 0.0f);
		F32 m_minForceMagnitude = 0.0f;
		F32 m_maxForceMagnitude = 0.0f;

		/// If not set then it uses the world's default
		Vec3 m_minGravity = Vec3(MAX_F32);
		Vec3 m_maxGravity = Vec3(MAX_F32);

		/// This position is relevant to the particle emitter pos
		Vec3 m_minStartingPosition = Vec3(0.0);
		Vec3 m_maxStartingPosition = Vec3(0.0);
	} m_particle;
	/// @}

	/// @name Emitter specific properties
	/// @{
	U32 m_maxNumOfParticles = 16; ///< The size of the particles vector. Required

	F32 m_emissionPeriod = 1.0; ///< How often the emitter emits new particles. In secs. Required

	U32 m_particlesPerEmission = 1; ///< How many particles are emitted every emission. Required

	Bool8 m_usePhysicsEngine = false; ///< Use bullet for the simulation
	/// @}

	Bool forceEnabled() const
	{
		return m_particle.m_maxForceMagnitude > 0.0f;
	}

	Bool wordGravityEnabled() const
	{
		return m_particle.m_maxGravity.x() < MAX_F32;
	}
};

/// This is the properties of the particle emitter resource
class ParticleEmitterResource : public ResourceObject, private ParticleEmitterProperties
{
public:
	ParticleEmitterResource(ResourceManager* manager);

	~ParticleEmitterResource();

	const ParticleEmitterProperties& getProperties() const
	{
		return *this;
	}

	MaterialResourcePtr getMaterial() const
	{
		return m_material;
	}

	/// Get program for rendering.
	void getRenderingInfo(U lod, ShaderProgramPtr& prog) const;

	/// Load it
	ANKI_USE_RESULT Error load(const ResourceFilename& filename, Bool async);

private:
	MaterialResourcePtr m_material;
	U8 m_lodCount = 1; ///< Cache the value from the material

	void loadInternal(const XmlElement& el);

	template<typename T>
	ANKI_USE_RESULT Error readVar(const XmlElement& rootEl, CString varName, T& minVal, T& maxVal, const T* defaultVal);
};
/// @}

} // end namespace anki
