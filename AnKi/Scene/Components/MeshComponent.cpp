// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/MeshComponent.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/MeshResource.h>
#include <AnKi/GpuMemory/CopyEngine.h>
#include <AnKi/Physics/PhysicsCollisionShape.h>
#include <AnKi/Physics/PhysicsWorld.h>
#include <AnKi/Core/App.h>

namespace anki {

// Creates a box that with positions from -1 to 1
static void generateBoxPrimitive(DynamicArray<U16Vec3, MemoryPoolPtrWrapper<StackMemoryPool>>& indexBuffer,
								 DynamicArray<Vec3, MemoryPoolPtrWrapper<StackMemoryPool>>& positions,
								 DynamicArray<Vec3, MemoryPoolPtrWrapper<StackMemoryPool>>& normals,
								 DynamicArray<Vec2, MemoryPoolPtrWrapper<StackMemoryPool>>& uvs)
{
	constexpr U32 kFaceCount = 6;
	constexpr U32 kVertsPerFace = 4;

	struct Face
	{
		Vec3 m_normal;
		Array<Vec3, kVertsPerFace> m_positions; // Counter-clockwise when viewed from outside the box
	};

	// 6 faces, each with its own 4 vertices so every face gets a flat normal and a full UV quad
	static const Array<Face, kFaceCount> faces = {{
		// +X
		{Vec3(1.0f, 0.0f, 0.0f), {Vec3(1, -1, 1), Vec3(1, -1, -1), Vec3(1, 1, -1), Vec3(1, 1, 1)}},
		// -X
		{Vec3(-1.0f, 0.0f, 0.0f), {Vec3(-1, -1, -1), Vec3(-1, -1, 1), Vec3(-1, 1, 1), Vec3(-1, 1, -1)}},
		// +Y
		{Vec3(0.0f, 1.0f, 0.0f), {Vec3(-1, 1, 1), Vec3(1, 1, 1), Vec3(1, 1, -1), Vec3(-1, 1, -1)}},
		// -Y
		{Vec3(0.0f, -1.0f, 0.0f), {Vec3(-1, -1, -1), Vec3(1, -1, -1), Vec3(1, -1, 1), Vec3(-1, -1, 1)}},
		// +Z
		{Vec3(0.0f, 0.0f, 1.0f), {Vec3(-1, -1, 1), Vec3(1, -1, 1), Vec3(1, 1, 1), Vec3(-1, 1, 1)}},
		// -Z
		{Vec3(0.0f, 0.0f, -1.0f), {Vec3(1, -1, -1), Vec3(-1, -1, -1), Vec3(-1, 1, -1), Vec3(1, 1, -1)}},
	}};

	static const Array<Vec2, kVertsPerFace> faceUvs = {Vec2(0.0f, 0.0f), Vec2(1.0f, 0.0f), Vec2(1.0f, 1.0f), Vec2(0.0f, 1.0f)};

	positions.resize(kFaceCount * kVertsPerFace);
	normals.resize(kFaceCount * kVertsPerFace);
	uvs.resize(kFaceCount * kVertsPerFace);
	indexBuffer.resize(kFaceCount * 2); // 2 triangles per face

	for(U32 f = 0; f < kFaceCount; ++f)
	{
		const Face& face = faces[f];
		const U32 vertBase = f * kVertsPerFace;

		for(U32 v = 0; v < kVertsPerFace; ++v)
		{
			positions[vertBase + v] = face.m_positions[v];
			normals[vertBase + v] = face.m_normal;
			uvs[vertBase + v] = faceUvs[v];
		}

		// Two triangles, counter-clockwise: (0, 1, 2) and (0, 2, 3)
		const U16 b = U16(vertBase);
		indexBuffer[f * 2 + 0] = U16Vec3(b, U16(b + 1), U16(b + 2));
		indexBuffer[f * 2 + 1] = U16Vec3(b, U16(b + 2), U16(b + 3));
	}
}

// Creates a unit sphere (radius 1, so positions live in [-1, 1]) by subdividing an icosahedron. "subdivisions" controls the polygon count: the
// triangle count is 20 * 4^subdivisions and the vertex count is 10 * 4^subdivisions + 2. Normals are smooth (the normalized position) and the UVs use
// a simple equirectangular (latitude/longitude) mapping.
static void generateSpherePrimitive(U32 subdivisions, DynamicArray<U16Vec3, MemoryPoolPtrWrapper<StackMemoryPool>>& indexBuffer,
									DynamicArray<Vec3, MemoryPoolPtrWrapper<StackMemoryPool>>& positions,
									DynamicArray<Vec3, MemoryPoolPtrWrapper<StackMemoryPool>>& normals,
									DynamicArray<Vec2, MemoryPoolPtrWrapper<StackMemoryPool>>& uvs)
{
	// The U16 index buffer caps the vertex count at 65536. Subdivision 6 produces 40962 vertices, 7 would overflow
	ANKI_ASSERT(subdivisions <= 6 && "Too many subdivisions for a U16 index buffer");

	using Pool = MemoryPoolPtrWrapper<StackMemoryPool>;
	const Pool pool = positions.getMemoryPool();

	// The 12 vertices of an icosahedron. "t" is the golden ratio: with these coordinates all 30 edges have equal length
	const F32 t = (1.0f + sqrt(5.0f)) / 2.0f;
	const Array<Vec3, 12> icoVerts = {{Vec3(-1, t, 0), Vec3(1, t, 0), Vec3(-1, -t, 0), Vec3(1, -t, 0), Vec3(0, -1, t), Vec3(0, 1, t), Vec3(0, -1, -t),
									   Vec3(0, 1, -t), Vec3(t, 0, -1), Vec3(t, 0, 1), Vec3(-t, 0, -1), Vec3(-t, 0, 1)}};

	DynamicArray<Vec3, Pool> verts(pool);
	verts.resize(icoVerts.getSize());
	for(U32 i = 0; i < icoVerts.getSize(); ++i)
	{
		verts[i] = icoVerts[i].normalize(); // Push every vertex onto the unit sphere
	}

	// The 20 triangular faces of the icosahedron, wound counter-clockwise when viewed from outside (matches the box primitive)
	const Array<U16Vec3, 20> icoTris = {{U16Vec3(0, 11, 5), U16Vec3(0, 5, 1),  U16Vec3(0, 1, 7),   U16Vec3(0, 7, 10), U16Vec3(0, 10, 11),
										 U16Vec3(1, 5, 9),  U16Vec3(5, 11, 4), U16Vec3(11, 10, 2), U16Vec3(10, 7, 6), U16Vec3(7, 1, 8),
										 U16Vec3(3, 9, 4),  U16Vec3(3, 4, 2),  U16Vec3(3, 2, 6),   U16Vec3(3, 6, 8),  U16Vec3(3, 8, 9),
										 U16Vec3(4, 9, 5),  U16Vec3(2, 4, 11), U16Vec3(6, 2, 10),  U16Vec3(8, 6, 7),  U16Vec3(9, 8, 1)}};

	DynamicArray<U16Vec3, Pool> tris(pool);
	tris.resize(icoTris.getSize());
	for(U32 i = 0; i < icoTris.getSize(); ++i)
	{
		tris[i] = icoTris[i];
	}

	// Subdivide each triangle into 4. New vertices sit at the edge midpoints, pushed back onto the sphere. The cache makes
	// sure an edge shared by two triangles produces a single shared midpoint, which keeps the mesh watertight and small
	for(U32 s = 0; s < subdivisions; ++s)
	{
		HashMap<U64, U32, DefaultHasher<U64>, Pool, HashMapSparseArrayConfig> midpointCache(pool);

		auto getMidpoint = [&](U32 i0, U32 i1) -> U32 {
			const U64 key = (U64(min(i0, i1)) << 32) | U64(max(i0, i1));
			auto it = midpointCache.find(key);
			if(it != midpointCache.getEnd())
			{
				return *it;
			}

			const Vec3 mid = ((verts[i0] + verts[i1]) * 0.5f).normalize();
			const U32 newIdx = verts.getSize();
			verts.emplaceBack(mid);
			midpointCache.emplace(key, newIdx);
			return newIdx;
		};

		DynamicArray<U16Vec3, Pool> newTris(pool);
		newTris.resize(tris.getSize() * 4);
		U32 outTri = 0;

		for(const U16Vec3& tri : tris)
		{
			const U16 a = U16(getMidpoint(tri.x, tri.y));
			const U16 b = U16(getMidpoint(tri.y, tri.z));
			const U16 c = U16(getMidpoint(tri.z, tri.x));

			// Same winding as the parent triangle: 3 corner triangles plus the central one
			newTris[outTri++] = U16Vec3(tri.x, a, c);
			newTris[outTri++] = U16Vec3(a, tri.y, b);
			newTris[outTri++] = U16Vec3(c, b, tri.z);
			newTris[outTri++] = U16Vec3(a, b, c);
		}

		tris = std::move(newTris);
	}

	// Emit the geometry. Positions are already unit length so they fit the [-1, 1] range the box primitive also uses
	positions.resize(verts.getSize());
	normals.resize(verts.getSize());
	uvs.resize(verts.getSize());
	for(U32 i = 0; i < verts.getSize(); ++i)
	{
		const Vec3& p = verts[i];
		positions[i] = p;
		normals[i] = p; // The smooth normal of a unit sphere is just the normalized position

		// Equirectangular mapping. It leaves a UV seam where the longitude wraps from 1 back to 0 and some stretching at
		// the poles, but it's the simplest correct mapping and good enough for a debug primitive
		const F32 u = 0.5f + atan2(p.z, p.x) / (2.0f * kPi);
		const F32 v = 0.5f - asin(clamp(p.y, -1.0f, 1.0f)) / kPi;
		uvs[i] = Vec2(u, v);
	}

	indexBuffer.resize(tris.getSize());
	for(U32 i = 0; i < tris.getSize(); ++i)
	{
		indexBuffer[i] = tris[i];
	}
}

// Caches primitive shapes to avoid duplication of data
class MeshComponent::PrimitivesGeometryCache : public MakeSingletonLazyInit<PrimitivesGeometryCache>
{
public:
	class GeometryParams
	{
	public:
		U32 m_subdivision = 0;

		Bool operator==(const GeometryParams& b) const = default;
	};

	class Geometry : public IntrusiveListEnabled<Geometry>
	{
	public:
		UnifiedGeometryBufferAllocation m_indexBuffer;
		UnifiedGeometryBufferAllocation m_positionBuffer;
		UnifiedGeometryBufferAllocation m_normalsBuffer;
		UnifiedGeometryBufferAllocation m_uvsBuffer;
		UnifiedGeometryBufferAllocation m_asBuffer;
		AccelerationStructurePtr m_as;
		PhysicsCollisionShapePtr m_collisionShape;

		GeometryParams m_params;
		U32 m_users = 0;
		MeshComponentPrimitiveType m_type = MeshComponentPrimitiveType::kCount;
	};

	class Type
	{
	public:
		IntrusiveList<Geometry> m_geometries;

		Geometry* findGeometry(const GeometryParams& params)
		{
			for(Geometry& geom : m_geometries)
			{
				if(params == geom.m_params)
				{
					return &geom;
				}
			}

			return nullptr;
		}
	};

	Array<Type, U32(MeshComponentPrimitiveType::kCount)> m_types;
	Mutex m_mtx;

	// It's thread-safe
	Geometry* getOrCreatePrimitive(MeshComponentPrimitiveType type, const GeometryParams& params, StackMemoryPool& pool)
	{
		LockGuard lock(m_mtx);

		// Find if exists
		Geometry* geom = m_types[type].findGeometry(params);
		if(geom)
		{
			ANKI_ASSERT(geom->m_users > 0);
			++geom->m_users;
			return geom;
		}

		geom = newInstance<Geometry>(SceneMemoryPool::getSingleton());
		m_types[type].m_geometries.pushBack(geom);
		geom->m_users = 1;
		geom->m_type = type;

		DynamicArray<U16Vec3, MemoryPoolPtrWrapper<StackMemoryPool>> indexBuffer(&pool);
		DynamicArray<Vec3, MemoryPoolPtrWrapper<StackMemoryPool>> positions(&pool);
		DynamicArray<Vec3, MemoryPoolPtrWrapper<StackMemoryPool>> normals(&pool);
		DynamicArray<Vec2, MemoryPoolPtrWrapper<StackMemoryPool>> uvs(&pool);

		if(type == MeshComponentPrimitiveType::kBox)
		{
			generateBoxPrimitive(indexBuffer, positions, normals, uvs);
		}
		else
		{
			ANKI_ASSERT(type == MeshComponentPrimitiveType::kSphere);
			generateSpherePrimitive(params.m_subdivision, indexBuffer, positions, normals, uvs);
		}

		geom->m_collisionShape = PhysicsWorld::getSingleton().newConvexHullShape(positions);

		const U32 indexCount = indexBuffer.getSize() * 3;

		// Copy the index buffer
		{
			const U32 size = U32(indexBuffer.getSizeInBytes());

			UnifiedGeometryBuffer::getSingleton().deferredFree(geom->m_indexBuffer);
			geom->m_indexBuffer = UnifiedGeometryBuffer::getSingleton().allocate(size, getIndexSize(IndexType::kU16));

			WeakArray<U8> mappedMem;
			const CopyEngineLockGuard lock = CopyEngine::getSingleton().copyBufferToBuffer(size, mappedMem, geom->m_indexBuffer);

			memcpy(mappedMem.getBegin(), indexBuffer.getBegin(), size);
		}

		// Copy the positions
		{
			const VertexStreamId vertStream = VertexStreamId::kPosition;
			const Format fmt = kMeshRelatedVertexStreamFormats[vertStream];
			ANKI_ASSERT(fmt == Format::kR16G16B16A16_Unorm);
			UnifiedGeometryBufferAllocation& alloc = geom->m_positionBuffer;

			UnifiedGeometryBuffer::getSingleton().deferredFree(alloc);
			alloc = UnifiedGeometryBuffer::getSingleton().allocateFormat(fmt, positions.getSize());

			const U32 size = positions.getSize() * getFormatInfo(fmt).m_texelSize;
			WeakArray<U8> mappedMem;
			const CopyEngineLockGuard lock = CopyEngine::getSingleton().copyBufferToBuffer(size, mappedMem, alloc);

			U32 offset = 0;
			for(Vec3 pos : positions)
			{
				pos = pos / 2.0f + 0.5f;
				pos = pos.clamp(0.0f, 1.0f);
				pos *= F32(kMaxU16);
				pos = pos.round();
				const U16Vec4 packedPos = U16Vec4(pos.xyz0);

				ANKI_ASSERT(offset + sizeof(packedPos) <= mappedMem.getSizeInBytes());
				memcpy(mappedMem.getBegin() + offset, &packedPos, sizeof(packedPos));
				offset += sizeof(packedPos);
			}
		}

		// Copy the normals
		{
			const VertexStreamId vertStream = VertexStreamId::kNormal;
			const Format fmt = kMeshRelatedVertexStreamFormats[vertStream];
			ANKI_ASSERT(fmt == Format::kR8G8B8A8_Snorm);
			UnifiedGeometryBufferAllocation& alloc = geom->m_normalsBuffer;

			alloc = UnifiedGeometryBuffer::getSingleton().allocateFormat(fmt, normals.getSize());

			const U32 size = normals.getSize() * getFormatInfo(fmt).m_texelSize;
			WeakArray<U8> mappedMem;
			const CopyEngineLockGuard lock = CopyEngine::getSingleton().copyBufferToBuffer(size, mappedMem, alloc);

			U32 offset = 0;
			for(const Vec3& normal : normals)
			{
				const U32 packedNormal = normal.xyz0.packSnorm4x8();

				ANKI_ASSERT(offset + sizeof(packedNormal) <= mappedMem.getSizeInBytes());
				memcpy(mappedMem.getBegin() + offset, &packedNormal, sizeof(packedNormal));
				offset += sizeof(packedNormal);
			}
		}

		// Copy the UVs
		{
			const VertexStreamId vertStream = VertexStreamId::kUv;
			const Format fmt = kMeshRelatedVertexStreamFormats[vertStream];
			ANKI_ASSERT(fmt == Format::kR32G32_Sfloat);
			UnifiedGeometryBufferAllocation& alloc = geom->m_uvsBuffer;

			alloc = UnifiedGeometryBuffer::getSingleton().allocateFormat(fmt, uvs.getSize());

			const U32 size = uvs.getSize() * getFormatInfo(fmt).m_texelSize;
			WeakArray<U8> mappedMem;
			const CopyEngineLockGuard lock = CopyEngine::getSingleton().copyBufferToBuffer(size, mappedMem, alloc);

			ANKI_ASSERT(mappedMem.getSizeInBytes() == uvs.getSizeInBytes());
			memcpy(mappedMem.getBegin(), uvs.getBegin(), mappedMem.getSizeInBytes());
		}

		// Build the BLAS
		if(GrManager::getSingleton().getDeviceCapabilities().m_rayTracing)
		{
			// Create the BLAS
			AccelerationStructureInitInfo inf("MeshComponentPrimitive");
			inf.m_type = AccelerationStructureType::kBottomLevel;

			inf.m_bottomLevel.m_indexBuffer = BufferView(geom->m_indexBuffer);
			inf.m_bottomLevel.m_indexCount = indexCount;
			inf.m_bottomLevel.m_indexType = IndexType::kU16;
			inf.m_bottomLevel.m_positionBuffer = geom->m_positionBuffer;
			inf.m_bottomLevel.m_positionStride = getFormatInfo(kMeshRelatedVertexStreamFormats[VertexStreamId::kPosition]).m_texelSize;
			inf.m_bottomLevel.m_positionsFormat = kMeshRelatedVertexStreamFormats[VertexStreamId::kPosition];
			inf.m_bottomLevel.m_positionCount = positions.getSize();

			const PtrSize requiredMemory = GrManager::getSingleton().getAccelerationStructureMemoryRequirement(inf);
			geom->m_asBuffer = UnifiedGeometryBuffer::getSingleton().allocate(requiredMemory, 1);
			inf.m_accelerationStructureBuffer = geom->m_asBuffer;

			geom->m_as = GrManager::getSingleton().newAccelerationStructure(inf);

			// Set the barriers
			const BufferUsageBit unifiedGeometryBufferNonTransferUsage =
				UnifiedGeometryBuffer::getSingleton().getBuffer().getBufferUsage() ^ BufferUsageBit::kCopyDestination;

			BufferBarrierInfo bufferBarrier;
			bufferBarrier.m_bufferView = UnifiedGeometryBuffer::getSingleton().getBufferView();
			bufferBarrier.m_previousUsage = BufferUsageBit::kCopyDestination;
			bufferBarrier.m_nextUsage = unifiedGeometryBufferNonTransferUsage;

			AccelerationStructureBarrierInfo asBarrier;
			asBarrier.m_as = geom->m_as.get();
			asBarrier.m_previousUsage = AccelerationStructureUsageBit::kNone;
			asBarrier.m_nextUsage = AccelerationStructureUsageBit::kBuild;

			CopyEngine::getSingleton().setPipelineBarrier({}, {&bufferBarrier, 1}, {&asBarrier, 1});

			// Build the BLAS
			CopyEngine::getSingleton().buildAccelerationStructure(geom->m_as.get());
		}

		return geom;
	}

	void removeUser(Geometry* geom)
	{
		if(geom)
		{
			LockGuard lock(m_mtx);
			ANKI_ASSERT(geom->m_users > 0);
			if(--geom->m_users == 0)
			{
				m_types[geom->m_type].m_geometries.erase(geom);
				deleteInstance(SceneMemoryPool::getSingleton(), geom);
			}
		}
	}
};

MeshComponent::MeshComponent(const SceneComponentInitInfo& init)
	: SceneComponent(kClassType, init)
{
}

MeshComponent::~MeshComponent()
{
	PrimitivesGeometryCache::Geometry* geometry = static_cast<PrimitivesGeometryCache::Geometry*>(m_primitiveGometry);
	PrimitivesGeometryCache::getSingleton().removeUser(geometry);
}

MeshComponent& MeshComponent::setMeshFilename(CString fname)
{
	MeshResourcePtr newRsrc;
	const Error err = ResourceManager::getSingleton().loadResource(fname, newRsrc);
	if(err)
	{
		ANKI_SCENE_LOGE("Failed to load resource: %s", fname.cstr());
	}
	else
	{
		m_resource = newRsrc;
		m_dirty = true;
	}

	return *this;
}

CString MeshComponent::getMeshFilename() const
{
	return (m_resource) ? m_resource->getFilename() : "*Error*";
}

Bool MeshComponent::isValid() const
{
	return (m_type == MeshComponentType::kPrimitive && m_primitiveType < MeshComponentPrimitiveType::kCount)
		   || (m_type == MeshComponentType::kMeshResource && !!m_resource && m_resource->isLoaded());
}

void MeshComponent::update([[maybe_unused]] SceneComponentUpdateInfo& info, Bool& updated)
{
	if(!isValid()) [[unlikely]]
	{
		for(GpuSceneArrays::MeshLod::Allocation& alloc : m_gpuSceneMeshLods)
		{
			alloc.free();
		}
		return;
	}

	m_gpuSceneMeshLodsReallocatedThisFrame = false;
	if(!m_dirty) [[likely]]
	{
		return;
	}

	updated = true;
	m_dirty = false;

	if(m_type == MeshComponentType::kMeshResource)
	{
		updateTypeMeshResource(info);
	}
	else
	{
		ANKI_ASSERT(m_type == MeshComponentType::kPrimitive);
		updateTypePrimitive(info);
	}
}

void MeshComponent::updateTypeMeshResource([[maybe_unused]] SceneComponentUpdateInfo& info)
{
	const MeshResource& mesh = *m_resource;
	const U32 submeshCount = mesh.getSubMeshCount();

	if(m_gpuSceneMeshLods.getSize() != submeshCount)
	{
		m_gpuSceneMeshLods.destroy();
		m_gpuSceneMeshLods.resize(submeshCount);

		for(GpuSceneArrays::MeshLod::Allocation& a : m_gpuSceneMeshLods)
		{
			a.allocate();
			m_gpuSceneMeshLodsReallocatedThisFrame = true;
		}
	}

	for(U32 submeshIdx = 0; submeshIdx < submeshCount; ++submeshIdx)
	{
		Array<GpuSceneMeshLod, kMaxLodCount> meshLods;

		for(U32 l = 0; l < mesh.getLodCount(); ++l)
		{
			GpuSceneMeshLod& meshLod = meshLods[l];
			meshLod = {};
			meshLod.m_positionScale = mesh.getPositionsScale();
			meshLod.m_positionTranslation = mesh.getPositionsTranslation();

			U32 firstIndex, indexCount, firstMeshlet, meshletCount;
			Aabb aabb;
			mesh.getSubMeshInfo(l, submeshIdx, firstIndex, indexCount, firstMeshlet, meshletCount, aabb);

			U32 totalIndexCount;
			IndexType indexType;
			PtrSize indexUgbOffset;
			mesh.getIndexBufferInfo(l, indexUgbOffset, totalIndexCount, indexType);

			for(VertexStreamId stream = VertexStreamId::kMeshRelatedFirst; stream < VertexStreamId::kMeshRelatedCount; ++stream)
			{
				if(mesh.isVertexStreamPresent(stream))
				{
					U32 vertCount;
					PtrSize ugbOffset;
					mesh.getVertexBufferInfo(l, stream, ugbOffset, vertCount);
					const PtrSize elementSize = getFormatInfo(kMeshRelatedVertexStreamFormats[stream]).m_texelSize;
					ANKI_ASSERT(ugbOffset % elementSize == 0);
					meshLod.m_vertexOffsets[U32(stream)] = U32(ugbOffset / elementSize);
				}
				else
				{
					meshLod.m_vertexOffsets[U32(stream)] = kMaxU32;
				}
			}

			meshLod.m_indexCount = indexCount;

			ANKI_ASSERT(indexUgbOffset % getIndexSize(indexType) == 0);
			meshLod.m_firstIndex = U32(indexUgbOffset / getIndexSize(indexType)) + firstIndex;

			if(GrManager::getSingleton().getDeviceCapabilities().m_meshShaders || g_cvarCoreMeshletRendering)
			{
				U32 dummy;
				PtrSize meshletBoundingVolumesUgbOffset, meshletGometryDescriptorsUgbOffset;
				mesh.getMeshletBufferInfo(l, meshletBoundingVolumesUgbOffset, meshletGometryDescriptorsUgbOffset, dummy);

				meshLod.m_firstMeshletBoundingVolume = firstMeshlet + U32(meshletBoundingVolumesUgbOffset / sizeof(MeshletBoundingVolume));
				meshLod.m_firstMeshletGeometryDescriptor = firstMeshlet + U32(meshletGometryDescriptorsUgbOffset / sizeof(MeshletGeometryDescriptor));
				meshLod.m_meshletCount = meshletCount;
			}

			meshLod.m_lod = l;

			if(GrManager::getSingleton().getDeviceCapabilities().m_rayTracing)
			{
				const U64 address = mesh.getBottomLevelAccelerationStructure(l, submeshIdx)->getGpuAddress();
				memcpy(&meshLod.m_blasAddress[0], &address, sizeof(meshLod.m_blasAddress));

				meshLod.m_tlasInstanceMask = 0xFFFFFFFF;
			}
		}

		// Copy the last LOD to the rest just in case
		for(U32 l = mesh.getLodCount(); l < kMaxLodCount; ++l)
		{
			meshLods[l] = meshLods[l - 1];
		}

		m_gpuSceneMeshLods[submeshIdx].uploadToGpuScene(meshLods);
	}

	m_primitiveGometry = nullptr;
}

void MeshComponent::updateTypePrimitive(SceneComponentUpdateInfo& info)
{
	// Get the geometry
	PrimitivesGeometryCache::Geometry* geometry = static_cast<PrimitivesGeometryCache::Geometry*>(m_primitiveGometry);
	PrimitivesGeometryCache::getSingleton().removeUser(geometry);

	PrimitivesGeometryCache::GeometryParams params;
	if(m_primitiveType == MeshComponentPrimitiveType::kSphere)
	{
		params.m_subdivision = m_sphereSubdivision;
	}

	geometry = PrimitivesGeometryCache::getSingleton().getOrCreatePrimitive(m_primitiveType, params, *info.m_framePool);
	m_primitiveGometry = geometry;
	const U32 indexCount = geometry->m_indexBuffer.getAllocatedSize() / getIndexSize(IndexType::kU16);

	// Allocate mesh LODs
	const U32 submeshCount = 1;
	if(m_gpuSceneMeshLods.getSize() != submeshCount)
	{
		m_gpuSceneMeshLods.destroy();
		m_gpuSceneMeshLods.resize(submeshCount);

		m_gpuSceneMeshLods[0].allocate();
		m_gpuSceneMeshLodsReallocatedThisFrame = true;
	}

	// Fill the mesh lods
	{
		Array<GpuSceneMeshLod, kMaxLodCount> meshLods;

		GpuSceneMeshLod& meshLod = meshLods[0];
		meshLod = {};

		for(U32& id : meshLod.m_vertexOffsets)
		{
			id = kMaxU32;
		}

		meshLod.m_vertexOffsets[U32(VertexStreamId::kPosition)] =
			geometry->m_positionBuffer.getOffset() / getFormatInfo(kMeshRelatedVertexStreamFormats[VertexStreamId::kPosition]).m_texelSize;
		meshLod.m_vertexOffsets[U32(VertexStreamId::kNormal)] =
			geometry->m_normalsBuffer.getOffset() / getFormatInfo(kMeshRelatedVertexStreamFormats[VertexStreamId::kNormal]).m_texelSize;
		meshLod.m_vertexOffsets[U32(VertexStreamId::kUv)] =
			geometry->m_uvsBuffer.getOffset() / getFormatInfo(kMeshRelatedVertexStreamFormats[VertexStreamId::kUv]).m_texelSize;

		meshLod.m_indexCount = indexCount;
		meshLod.m_firstIndex = geometry->m_indexBuffer.getOffset() / getIndexSize(IndexType::kU16);
		meshLod.m_lod = 0;
		meshLod.m_firstMeshletBoundingVolume = kMaxU32;
		meshLod.m_firstMeshletGeometryDescriptor = kMaxU32;
		meshLod.m_meshletCount = 0;
		meshLod.m_positionScale = 2.0f;
		meshLod.m_positionTranslation = Vec3(-1.0f);
		const U64 address = (geometry->m_as) ? geometry->m_as->getGpuAddress() : 0;
		memcpy(&meshLod.m_blasAddress[0], &address, sizeof(address));
		meshLod.m_tlasInstanceMask = 0xFFFFFFFF;

		for(U32 l = 1; l < kMaxLodCount; ++l)
		{
			meshLods[l] = meshLods[l - 1];
		}

		m_gpuSceneMeshLods[0].uploadToGpuScene(meshLods);
	}
}

Error MeshComponent::serialize(SceneSerializer& serializer)
{
	ANKI_SERIALIZE(m_resource, 1);
	ANKI_SERIALIZE(m_type, 1);
	ANKI_SERIALIZE(m_primitiveType, 1);
	ANKI_SERIALIZE(m_sphereSubdivision, 1);

	return Error::kNone;
}

PhysicsCollisionShape* MeshComponent::getPrimitiveCollisionShape() const
{
	ANKI_ASSERT(isValid());
	ANKI_ASSERT(m_type == MeshComponentType::kPrimitive);
	ANKI_ASSERT(m_primitiveGometry);

	return static_cast<PrimitivesGeometryCache::Geometry*>(m_primitiveGometry)->m_collisionShape.get();
}

} // end namespace anki
