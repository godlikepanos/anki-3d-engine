// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Resource/MeshResource.h>
#include <AnKi/Scene/GpuSceneArray.h>
#include <AnKi/GpuMemory/UnifiedGeometryBuffer.h>

namespace anki {

enum class MeshComponentType : U8
{
	kMeshResource,
	kPrimitive,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(MeshComponentType)

inline constexpr Array<const Char*, U32(MeshComponentType::kCount)> kMeshComponentTypeNames = {"Mesh Resource", "Primitive"};

enum class MeshComponentPrimitiveType : U8
{
	kBox,
	kSphere,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(MeshComponentPrimitiveType)

inline constexpr Array<const Char*, U32(MeshComponentPrimitiveType::kCount)> kMeshComponentPrimitiveTypeNames = {"Box", "Sphere"};

// Holds geometry information.
class MeshComponent final : public SceneComponent
{
	ANKI_SCENE_COMPONENT(MeshComponent)

public:
	static constexpr U32 kMaxSpherePrimitiveSubdivision = 6;

	MeshComponent(const SceneComponentInitInfo& init);

	~MeshComponent();

	MeshComponent& setMeshComponentType(MeshComponentType type)
	{
		if(ANKI_EXPECT(type < MeshComponentType::kCount) && m_type != type)
		{
			m_type = type;
			m_dirty = true;
		}
		return *this;
	}

	MeshComponentType getMeshComponentType() const
	{
		return m_type;
	}

	MeshComponent& setMeshComponentPrimitiveType(MeshComponentPrimitiveType type)
	{
		if(ANKI_EXPECT(m_primitiveType < MeshComponentPrimitiveType::kCount) && m_primitiveType != type)
		{
			m_primitiveType = type;
			m_dirty = m_type == MeshComponentType::kPrimitive;
		}
		return *this;
	}

	MeshComponentPrimitiveType getMeshComponentPrimitiveType() const
	{
		return m_primitiveType;
	}

	MeshComponent& setSpherePrimitiveSubdivision(U32 subdivision)
	{
		if(!ANKI_EXPECT(subdivision > 1 && subdivision < kMaxSpherePrimitiveSubdivision))
		{
			subdivision = clamp(subdivision, 1u, kMaxSpherePrimitiveSubdivision);
		}

		if(m_sphereSubdivision != subdivision)
		{
			m_sphereSubdivision = subdivision;
			m_dirty = m_type == MeshComponentType::kPrimitive && m_primitiveType == MeshComponentPrimitiveType::kSphere;
		}
		return *this;
	}

	U32 getSpherePrimitiveSubdivision() const
	{
		return m_sphereSubdivision;
	}

	MeshComponent& setMeshFilename(CString fname);

	Bool hasMeshResource() const
	{
		return !!m_resource;
	}

	CString getMeshFilename() const;

	Bool isValid() const;

	Aabb getLocalAabb() const
	{
		if(ANKI_EXPECT(isValid()))
		{
			return (m_type == MeshComponentType::kMeshResource) ? m_resource->getBoundingShape() : Aabb(Vec3(-1.0f), Vec3(1.0f));
		}
		else
		{
			return Aabb(Vec3(-1.0f), Vec3(1.0f));
		}
	}

	ANKI_INTERNAL const MeshResource& getMeshResource() const
	{
		ANKI_ASSERT(isValid());
		return *m_resource;
	}

	ANKI_INTERNAL U32 getGpuSceneMeshLodsIndex(U32 submeshIdx) const
	{
		ANKI_ASSERT(isValid());
		return m_gpuSceneMeshLods[submeshIdx].getIndex() * kMaxLodCount;
	}

	ANKI_INTERNAL Bool gpuSceneReallocationsThisFrame() const
	{
		ANKI_ASSERT(isValid());
		return m_gpuSceneMeshLodsReallocatedThisFrame;
	}

	ANKI_INTERNAL PhysicsCollisionShape* getPrimitiveCollisionShape() const;

private:
	class PrimitivesGeometryCache;

	MeshResourcePtr m_resource;

	SceneDynamicArray<GpuSceneArrays::MeshLod::Allocation> m_gpuSceneMeshLods;

	MeshComponentType m_type = MeshComponentType::kMeshResource;
	MeshComponentPrimitiveType m_primitiveType = MeshComponentPrimitiveType::kBox;
	Bool m_dirty = true;
	Bool m_gpuSceneMeshLodsReallocatedThisFrame = false;

	void* m_primitiveGometry = nullptr;
	U32 m_sphereSubdivision = 1;

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;
	void updateTypeMeshResource(SceneComponentUpdateInfo& info);
	void updateTypePrimitive(SceneComponentUpdateInfo& info);

	Error serialize(SceneSerializer& serializer) override;
};

} // end namespace anki
