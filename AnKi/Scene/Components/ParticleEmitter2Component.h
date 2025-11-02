// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Scene/RenderStateBucket.h>
#include <AnKi/Scene/GpuSceneArray.h>
#include <AnKi/Resource/ParticleEmitterResource.h>
#include <AnKi/GpuMemory/UnifiedGeometryBuffer.h>
#include <AnKi/Collision/Aabb.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

enum class ParticleGeometryType : U8
{
	kQuad,
	kMeshComponent,
	kCount
};

// Contains a particle emitter resource and maybe connects to a mesh component
class ParticleEmitter2Component : public SceneComponent
{
	ANKI_SCENE_COMPONENT(ParticleEmitter2Component)

public:
	ParticleEmitter2Component(SceneNode* node);

	~ParticleEmitter2Component();

	ParticleEmitter2Component& setParticleEmitterFilename(CString filename);

	CString getParticleEmitterFilename() const;

	ParticleEmitter2Component& setParticleGeometryType(ParticleGeometryType type)
	{
		if(type != m_geomType && ANKI_EXPECT(type < ParticleGeometryType::kCount))
		{
			m_geomType = type;
			m_geomTypeDirty = true;
		}

		return *this;
	}

	ParticleGeometryType getParticleGeometryType() const
	{
		return m_geomType;
	}

	Bool isValid() const;

	ANKI_INTERNAL U32 getGpuSceneMeshLodIndex(U32 submeshIdx) const;

	ANKI_INTERNAL U32 getGpuSceneParticleEmitter2Index() const
	{
		ANKI_ASSERT(isValid());
		return m_gpuScene.m_gpuSceneParticleEmitter.getIndex();
	}

	// Return true if the above indices changed this frame
	ANKI_INTERNAL Bool gpuSceneReallocationsThisFrame() const
	{
		ANKI_ASSERT(isValid());
		return m_gpuSceneReallocationsThisFrame;
	}

	// The renderer will call it to update the bounds of the emitter
	ANKI_INTERNAL void updateBoundingVolume(Vec3 min, Vec3 max)
	{
		ANKI_ASSERT(min < max);
		m_boundingVolume = {min, max};
	}

	ANKI_INTERNAL Array<Vec3, 2> getBoundingVolume() const
	{
		ANKI_ASSERT(isValid());
		return m_boundingVolume;
	}

private:
	class ParticleEmitterQuadGeometry;

	ParticleEmitterResource2Ptr m_particleEmitterResource;

	MeshComponent* m_meshComponent = nullptr;

	class
	{
	public:
		Array<GpuSceneBufferAllocation, U32(ParticleProperty::kCount)> m_particleStreams;
		GpuSceneBufferAllocation m_aliveParticleIndices;
		GpuSceneBufferAllocation m_anKiParticleEmitterProperties;
		GpuSceneArrays::ParticleEmitter2::Allocation m_gpuSceneParticleEmitter;
	} m_gpuScene;

	Array<Vec3, 2> m_boundingVolume;

	ParticleGeometryType m_geomType = ParticleGeometryType::kQuad;
	Bool m_resourceDirty : 1 = true;
	Bool m_meshComponentDirty : 1 = true;
	Bool m_geomTypeDirty : 1 = true;
	Bool m_gpuSceneReallocationsThisFrame : 1 = true; // Only tracks the memory shared externally

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;

	void onOtherComponentRemovedOrAdded(SceneComponent* other, Bool added) override;
};

} // end namespace anki
