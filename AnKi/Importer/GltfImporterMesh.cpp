// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Importer/GltfImporter.h>
#include <AnKi/Util/StringList.h>
#include <AnKi/Collision/Plane.h>
#include <AnKi/Collision/Functions.h>
#include <AnKi/Collision/Sphere.h>
#include <AnKi/Resource/MeshBinary.h>
#include <AnKi/Shaders/Include/MeshTypes.h>
#include <MeshOptimizer/meshoptimizer.h>

namespace anki {

static U cgltfComponentCount(cgltf_type type)
{
	U out;
	switch(type)
	{
	case cgltf_type_scalar:
		out = 1;
		break;
	case cgltf_type_vec2:
		out = 2;
		break;
	case cgltf_type_vec3:
		out = 3;
		break;
	case cgltf_type_vec4:
		out = 4;
		break;
	default:
		ANKI_ASSERT(!"TODO");
		out = 0;
	}

	return out;
}

static U cgltfComponentSize(cgltf_component_type type)
{
	U out;
	switch(type)
	{
	case cgltf_component_type_r_32f:
		out = sizeof(F32);
		break;
	case cgltf_component_type_r_16u:
		out = sizeof(U16);
		break;
	case cgltf_component_type_r_8u:
		out = sizeof(U8);
		break;
	default:
		ANKI_ASSERT(!"TODO");
		out = 0;
	}

	return out;
}

#if 0
static U calcImplicitStride(const cgltf_attribute& attrib)
{
	return cgltfComponentCount(attrib.data->type) * cgltfComponentSize(attrib.data->component_type);
}
#endif

template<typename T>
static Error checkAttribute(const cgltf_attribute& attrib)
{
	if(cgltfComponentCount(attrib.data->type) != T::kComponentCount)
	{
		ANKI_IMPORTER_LOGE("Wrong component count for attribute: %s", attrib.name);
		return Error::kUserData;
	}

	if(cgltfComponentSize(attrib.data->component_type) != sizeof(typename T::Scalar))
	{
		ANKI_IMPORTER_LOGE("Incompatible type: %s", attrib.name);
		return Error::kUserData;
	}

	ANKI_ASSERT(attrib.data);
	const U count = U(attrib.data->count);
	if(count == 0)
	{
		ANKI_IMPORTER_LOGE("Zero vertex count");
		return Error::kUserData;
	}

	return Error::kNone;
}

class TempVertex
{
public:
	Vec3 m_position;
	F32 m_padding0;

	Vec4 m_boneWeights;

	Vec3 m_normal;
	F32 m_padding1;

	Vec2 m_uv;
	U16Vec4 m_boneIds;

	TempVertex()
	{
		zeroMemory(*this);
	}
};
static_assert(sizeof(TempVertex) == 4 * sizeof(Vec4), "Will be hashed");

class ImporterMeshlet
{
public:
	U32 m_firstVertex = 0;
	U32 m_vertexCount = 0;
	U32 m_firstLocalIndex = 0;
	U32 m_localIndexCount = 0;

	Vec3 m_coneApex;
	Vec3 m_coneDir;
	F32 m_coneAngle = 0.0f;

	Sphere m_sphere;
	Aabb m_aabb;
};

class SubMesh
{
public:
	ImporterDynamicArray<TempVertex> m_verts;
	ImporterDynamicArray<U32> m_indices;

	ImporterDynamicArray<ImporterMeshlet> m_meshlets;
	ImporterDynamicArray<U8> m_localIndices;

	Vec3 m_aabbMin = Vec3(kMaxF32);
	Vec3 m_aabbMax = Vec3(kMinF32);

	Vec3 m_sphereCenter;
	F32 m_sphereRadius = 0.0f;
};

static void reindexSubmesh(SubMesh& submesh)
{
	const U32 vertSize = sizeof(submesh.m_verts[0]);

	ImporterDynamicArray<U32> remap;
	remap.resize(submesh.m_verts.getSize(), 0);

	const U32 vertCount = U32(meshopt_generateVertexRemap(&remap[0], &submesh.m_indices[0], submesh.m_indices.getSize(), &submesh.m_verts[0],
														  submesh.m_verts.getSize(), vertSize));

	ImporterDynamicArray<U32> newIdxArray;
	newIdxArray.resize(submesh.m_indices.getSize(), 0);
	ImporterDynamicArray<TempVertex> newVertArray;
	newVertArray.resize(vertCount);

	meshopt_remapIndexBuffer(&newIdxArray[0], &submesh.m_indices[0], submesh.m_indices.getSize(), &remap[0]);
	meshopt_remapVertexBuffer(&newVertArray[0], &submesh.m_verts[0], submesh.m_verts.getSize(), vertSize, &remap[0]);

	submesh.m_indices = std::move(newIdxArray);
	submesh.m_verts = std::move(newVertArray);
}

/// Optimize a submesh using meshoptimizer.
static void optimizeSubmesh(SubMesh& submesh)
{
	const PtrSize vertSize = sizeof(submesh.m_verts[0]);

	// Vert cache
	{
		ImporterDynamicArray<U32> newIdxArray;
		newIdxArray.resize(submesh.m_indices.getSize(), 0);

		meshopt_optimizeVertexCache(&newIdxArray[0], &submesh.m_indices[0], submesh.m_indices.getSize(), submesh.m_verts.getSize());

		submesh.m_indices = std::move(newIdxArray);
	}

	// Overdraw
	{
		ImporterDynamicArray<U32> newIdxArray;
		newIdxArray.resize(submesh.m_indices.getSize(), 0);

		meshopt_optimizeOverdraw(&newIdxArray[0], &submesh.m_indices[0], submesh.m_indices.getSize(), &submesh.m_verts[0].m_position.x(),
								 submesh.m_verts.getSize(), vertSize, 1.05f);

		submesh.m_indices = std::move(newIdxArray);
	}

	// Vert fetch
	{
		ImporterDynamicArray<TempVertex> newVertArray;
		newVertArray.resize(submesh.m_verts.getSize());

		const U32 newVertCount =
			U32(meshopt_optimizeVertexFetch(&newVertArray[0],
											&submesh.m_indices[0], // Inplace
											submesh.m_indices.getSize(), &submesh.m_verts[0], submesh.m_verts.getSize(), vertSize));

		if(newVertCount != submesh.m_verts.getSize())
		{
			newVertArray.resize(newVertCount);
		}
		ANKI_ASSERT(newVertArray.getSize() == newVertCount);

		submesh.m_verts = std::move(newVertArray);
	}
}

/// Decimate a submesh using meshoptimizer.
static void decimateSubmesh(F32 factor, SubMesh& submesh)
{
	ANKI_ASSERT(factor > 0.0f && factor < 1.0f);
	const PtrSize targetIndexCount = PtrSize(F32(submesh.m_indices.getSize() / 3) * factor) * 3;
	if(targetIndexCount == 0)
	{
		return;
	}

	// Decimate
	ImporterDynamicArray<U32> newIndices;
	newIndices.resize(submesh.m_indices.getSize());
	newIndices.resize(U32(meshopt_simplify(&newIndices[0], &submesh.m_indices[0], submesh.m_indices.getSize(), &submesh.m_verts[0].m_position.x(),
										   submesh.m_verts.getSize(), sizeof(TempVertex), targetIndexCount, 1e-2f)));

	// Re-pack
	ImporterDynamicArray<U32> reindexedIndices;
	ImporterDynamicArray<TempVertex> newVerts;
	ImporterHashMap<U32, U32> vertexStored;
	for(U32 idx = 0; idx < newIndices.getSize(); ++idx)
	{
		U32 newIdx;
		auto it = vertexStored.find(newIndices[idx]);
		if(it == vertexStored.getEnd())
		{
			// Store the vertex
			newVerts.emplaceBack(submesh.m_verts[newIndices[idx]]);
			newIdx = newVerts.getSize() - 1;
			vertexStored.emplace(newIndices[idx], newIdx);
		}
		else
		{
			// Already stored
			newIdx = *it;
		}

		// Store the new index
		reindexedIndices.emplaceBack(newIdx);
	}

	// Move back
	submesh.m_indices = std::move(reindexedIndices);
	submesh.m_verts = std::move(newVerts);
}

/// If normal A and normal B have the same position then try to merge them. Do that before optimizations.
static void fixNormals(const F32 normalsMergeAngle, SubMesh& submesh)
{
	for(U32 v = 0; v < submesh.m_verts.getSize(); ++v)
	{
		const Vec3& pos = submesh.m_verts[v].m_position;
		Vec3& normal = submesh.m_verts[v].m_normal;

		for(U32 prevV = 0; prevV < v; ++prevV)
		{
			const Vec3& otherPos = submesh.m_verts[prevV].m_position;

			// Check the positions dist
			const F32 posDist = (otherPos - pos).getLengthSquared();
			if(posDist > kEpsilonf * kEpsilonf)
			{
				continue;
			}

			// Check angle of the normals
			Vec3& otherNormal = submesh.m_verts[prevV].m_normal;
			const F32 ang = acos(clamp(otherNormal.dot(normal), -1.0f, 1.0f));
			if(ang > normalsMergeAngle)
			{
				continue;
			}

			// Merge normals
			const Vec3 newNormal = (otherNormal + normal).getNormalized();
			normal = newNormal;
			otherNormal = newNormal;
		}
	}
}

static Bool isConvex(const ImporterList<SubMesh>& submeshes)
{
	Bool convex = true;
	for(const SubMesh& submesh : submeshes)
	{
		for(U32 i = 0; i < submesh.m_indices.getSize(); i += 3)
		{
			const U32 i0 = submesh.m_indices[i + 0];
			const U32 i1 = submesh.m_indices[i + 1];
			const U32 i2 = submesh.m_indices[i + 2];

			const Vec3& v0 = submesh.m_verts[i0].m_position;
			const Vec3& v1 = submesh.m_verts[i1].m_position;
			const Vec3& v2 = submesh.m_verts[i2].m_position;

			if(computeTriangleArea(v0, v1, v2) <= kEpsilonf)
			{
				continue;
			}

			// Check that all positions are behind the plane
			const Plane plane(v0.xyz0(), v1.xyz0(), v2.xyz0());

			for(const SubMesh& submeshB : submeshes)
			{
				for(const TempVertex& vertB : submeshB.m_verts)
				{
					const F32 test = testPlane(plane, vertB.m_position.xyz0());
					if(test > kEpsilonf)
					{
						convex = false;
						break;
					}
				}

				if(!convex)
				{
					break;
				}
			}

			if(!convex)
			{
				break;
			}
		}
	}

	return convex;
}

static void generateMeshlets(SubMesh& submesh)
{
	// Allocate the arrays
	const U32 maxMeshlets = U32(meshopt_buildMeshletsBound(submesh.m_indices.getSize(), kMaxVerticesPerMeshlet, kMaxPrimitivesPerMeshlet));

	ImporterDynamicArray<U32> indicesToVertexBuffer;
	indicesToVertexBuffer.resize(maxMeshlets * kMaxVerticesPerMeshlet);

	ImporterDynamicArray<U8> localIndices;
	localIndices.resize(maxMeshlets * kMaxPrimitivesPerMeshlet * 3);

	ImporterDynamicArray<meshopt_Meshlet> meshlets;
	meshlets.resize(maxMeshlets);

	// Meshletize
	constexpr F32 coneWeight = 0.0f;
	const U32 meshletCount =
		U32(meshopt_buildMeshlets(meshlets.getBegin(), indicesToVertexBuffer.getBegin(), localIndices.getBegin(), submesh.m_indices.getBegin(),
								  submesh.m_indices.getSize(), &submesh.m_verts[0].m_position.x(), submesh.m_verts.getSize(), sizeof(TempVertex),
								  kMaxVerticesPerMeshlet, kMaxPrimitivesPerMeshlet, coneWeight));

	// Trim the arrays
	const meshopt_Meshlet& last = meshlets[meshletCount - 1u];
	indicesToVertexBuffer.resize(last.vertex_offset + last.vertex_count);
	localIndices.resize(last.triangle_offset + ((last.triangle_count * 3u + 3u) & ~3u));
	meshlets.resize(meshletCount);

	// Create the new vertex and global index buffer
	submesh.m_meshlets.destroy();
	submesh.m_meshlets.resize(meshletCount);
	submesh.m_localIndices.destroy();

	ImporterDynamicArray<U32> newIndexBuffer;
	ImporterDynamicArray<TempVertex> newVertexBuffer;

	for(U32 meshletIdx = 0; meshletIdx < meshletCount; ++meshletIdx)
	{
		const meshopt_Meshlet& inMeshlet = meshlets[meshletIdx];
		ImporterMeshlet& outMeshlet = submesh.m_meshlets[meshletIdx];

		outMeshlet.m_firstLocalIndex = submesh.m_localIndices.getSize();
		outMeshlet.m_localIndexCount = inMeshlet.triangle_count * 3;
		outMeshlet.m_firstVertex = newVertexBuffer.getSize();
		outMeshlet.m_vertexCount = inMeshlet.vertex_count;

		ImporterHashMap<U8, U32> localIndexToNewGlobalIndex;

		for(U32 tri = 0; tri < inMeshlet.triangle_count; ++tri)
		{
			const U8Vec3 localIdx3(localIndices[inMeshlet.triangle_offset + tri * 3], localIndices[inMeshlet.triangle_offset + tri * 3 + 1],
								   localIndices[inMeshlet.triangle_offset + tri * 3 + 2]);
			for(U32 j = 0; j < 3; j++)
			{
				const U8 localIdx = localIdx3[j];

				// Search if we processed the same index before
				auto it = localIndexToNewGlobalIndex.find(localIdx);
				const Bool newVertex = (it == localIndexToNewGlobalIndex.getEnd());

				// Add the vertex, global index
				if(newVertex)
				{
					const U32 newGlobalIdx = newVertexBuffer.getSize();

					newIndexBuffer.emplaceBack(newGlobalIdx);

					const U32 globalIdx = indicesToVertexBuffer[inMeshlet.vertex_offset + localIdx];
					const TempVertex vert = submesh.m_verts[globalIdx];
					newVertexBuffer.emplaceBack(vert);

					localIndexToNewGlobalIndex.emplace(localIdx, newGlobalIdx);
				}
				else
				{
					const U32 newGlobalIdx = *it;

					newIndexBuffer.emplaceBack(newGlobalIdx);
				}

				// Append the local index
				submesh.m_localIndices.emplaceBack(localIdx);
			}
		}

		ANKI_ASSERT(localIndexToNewGlobalIndex.getSize() == outMeshlet.m_vertexCount);

		// Compute bounds
		const meshopt_Bounds bounds =
			meshopt_computeMeshletBounds(&indicesToVertexBuffer[inMeshlet.vertex_offset], &localIndices[inMeshlet.triangle_offset],
										 inMeshlet.triangle_count, &submesh.m_verts[0].m_position.x(), submesh.m_verts.getSize(), sizeof(TempVertex));
		outMeshlet.m_coneApex = Vec3(&bounds.cone_apex[0]);
		outMeshlet.m_coneDir = Vec3(&bounds.cone_axis[0]);
		outMeshlet.m_coneAngle = acos(bounds.cone_cutoff) * 2.0f;

		outMeshlet.m_sphere =
			computeBoundingSphere(&newVertexBuffer[outMeshlet.m_firstVertex].m_position, outMeshlet.m_vertexCount, sizeof(TempVertex));

		if(bounds.radius < outMeshlet.m_sphere.getRadius() && bounds.radius > 0.0f)
		{
			// meshopt computed smaller sphere, use that one
			outMeshlet.m_sphere.setCenter(Vec3(&bounds.center[0]));
			outMeshlet.m_sphere.setRadius(bounds.radius);
		}

		outMeshlet.m_aabb = computeBoundingAabb(&newVertexBuffer[outMeshlet.m_firstVertex].m_position, outMeshlet.m_vertexCount, sizeof(TempVertex));
	}

	const F64 avgPrimCountPerMeshlet = F64(newIndexBuffer.getSize() / 3) / F64(meshletCount);
	const F64 avgVertCountPerMeshlet = F64(newVertexBuffer.getSize()) / F64(meshletCount);
	ANKI_IMPORTER_LOGV("Meshletization stats: %f%% more vertices, %u meshlets, primitive_count/meshlet %f, vert_count/meshlet %f",
					   (F32(newVertexBuffer.getSize()) - F32(submesh.m_verts.getSize())) / F32(submesh.m_verts.getSize()) * 100.0f, meshletCount,
					   avgPrimCountPerMeshlet, avgVertCountPerMeshlet);

	submesh.m_indices = std::move(newIndexBuffer);
	submesh.m_verts = std::move(newVertexBuffer);
}

static void writeVertexAttribAndBufferInfoToHeader(VertexStreamId stream, MeshBinaryHeader& header, const Vec4& scale = Vec4(1.0f),
												   const Vec4& translation = Vec4(0.0f))
{
	MeshBinaryVertexAttribute& attrib = header.m_vertexAttributes[stream];
	attrib.m_bufferIndex = U32(stream);
	attrib.m_format = kMeshRelatedVertexStreamFormats[stream];
	attrib.m_relativeOffset = 0;
	attrib.m_scale = {scale[0], scale[1], scale[2], scale[3]};
	attrib.m_translation = {translation[0], translation[1], translation[2], translation[3]};

	MeshBinaryVertexBuffer& buff = header.m_vertexBuffers[stream];
	buff.m_vertexStride = getFormatInfo(attrib.m_format).m_texelSize;
}

U32 GltfImporter::getMeshTotalVertexCount(const cgltf_mesh& mesh)
{
	U32 totalVertexCount = 0;

	for(const cgltf_primitive* primitive = mesh.primitives; primitive < mesh.primitives + mesh.primitives_count; ++primitive)
	{
		totalVertexCount += U32(primitive->attributes[0].data->count);
	}

	return totalVertexCount;
}

Error GltfImporter::writeMesh(const cgltf_mesh& mesh) const
{
	const Error err = writeMeshInternal(mesh);
	if(err)
	{
		ANKI_IMPORTER_LOGE("Failed to write mesh: %s", mesh.name);
	}

	return err;
}

Error GltfImporter::writeMeshInternal(const cgltf_mesh& mesh) const
{
	const ImporterString meshName = computeMeshResourceFilename(mesh);
	ImporterString fname;
	fname.sprintf("%s%s", m_outDir.cstr(), meshName.cstr());
	ANKI_IMPORTER_LOGV("Importing mesh (%s): %s", (m_optimizeMeshes) ? "optimize" : "WON'T optimize", fname.cstr());

	Array<ImporterList<SubMesh>, kMaxLodCount> submeshes;
	Vec3 aabbMin(kMaxF32);
	Vec3 aabbMax(kMinF32);
	Bool hasBoneWeights = false;
	ImporterDynamicArray<Vec3> allPositions; // Used to calculate the overall bounding sphere

	// Iterate primitives. Every primitive is a submesh
	for(const cgltf_primitive* primitive = mesh.primitives; primitive < mesh.primitives + mesh.primitives_count; ++primitive)
	{
		if(primitive->type != cgltf_primitive_type_triangles)
		{
			ANKI_IMPORTER_LOGE("Expecting triangles got %d", primitive->type);
			return Error::kUserData;
		}

		SubMesh& submesh = *submeshes[0].emplaceBack();

		// All attributes should have the same vertex count
		U minVertCount = kMaxU;
		U maxVertCount = kMinU;
		for(const cgltf_attribute* attrib = primitive->attributes; attrib < primitive->attributes + primitive->attributes_count; ++attrib)
		{
			minVertCount = min(minVertCount, U(attrib->data->count));
			maxVertCount = max(maxVertCount, U(attrib->data->count));
		}

		if(maxVertCount == 0 || minVertCount != maxVertCount)
		{
			ANKI_IMPORTER_LOGE("Wrong number of vertices");
			return Error::kUserData;
		}

		U32 vertCount = U32(primitive->attributes[0].data->count);
		submesh.m_verts.resize(vertCount);

		//
		// Gather positions + normals + UVs + bone stuff
		//
		for(const cgltf_attribute* attrib = primitive->attributes; attrib < primitive->attributes + primitive->attributes_count; ++attrib)
		{
			if(attrib->type == cgltf_attribute_type_position)
			{
				U32 count = 0;
				ANKI_CHECK(checkAttribute<Vec3>(*attrib));
				visitAccessor<Vec3>(*attrib->data, [&](const Vec3& pos) {
					submesh.m_aabbMin = submesh.m_aabbMin.min(pos);
					submesh.m_aabbMax = submesh.m_aabbMax.max(pos);
					submesh.m_verts[count++].m_position = pos;

					allPositions.emplaceBack(pos);
				});
			}
			else if(attrib->type == cgltf_attribute_type_normal)
			{
				U32 count = 0;
				ANKI_CHECK(checkAttribute<Vec3>(*attrib));
				visitAccessor<Vec3>(*attrib->data, [&](const Vec3& normal) {
					submesh.m_verts[count++].m_normal = normal;
				});
			}
			else if(attrib->type == cgltf_attribute_type_texcoord && CString(attrib->name) == "TEXCOORD_0")
			{
				U32 count = 0;
				ANKI_CHECK(checkAttribute<Vec2>(*attrib));
				visitAccessor<Vec2>(*attrib->data, [&](const Vec2& uv) {
					submesh.m_verts[count++].m_uv = uv;
				});
			}
			else if(attrib->type == cgltf_attribute_type_joints)
			{
				U32 count = 0;
				if(cgltfComponentSize(attrib->data->component_type) == 2)
				{
					ANKI_CHECK(checkAttribute<U16Vec4>(*attrib));
					visitAccessor<U16Vec4>(*attrib->data, [&](const U16Vec4& x) {
						submesh.m_verts[count++].m_boneIds = x;
					});
				}
				else
				{
					ANKI_CHECK(checkAttribute<U8Vec4>(*attrib));
					visitAccessor<U8Vec4>(*attrib->data, [&](const U8Vec4& x) {
						submesh.m_verts[count++].m_boneIds = U16Vec4(x);
					});
				}
				hasBoneWeights = true;
			}
			else if(attrib->type == cgltf_attribute_type_weights)
			{
				U32 count = 0;
				ANKI_CHECK(checkAttribute<Vec4>(*attrib));
				visitAccessor<Vec4>(*attrib->data, [&](const Vec4& bw) {
					submesh.m_verts[count++].m_boneWeights = bw;
				});
			}
			else
			{
				ANKI_IMPORTER_LOGV("Ignoring attribute: %s", attrib->name);
			}
		}

		aabbMin = aabbMin.min(submesh.m_aabbMin);
		submesh.m_aabbMax += kEpsilonf * 10.0f; // Bump aabbMax a bit
		aabbMax = aabbMax.max(submesh.m_aabbMax);

		const Sphere s = computeBoundingSphere(&submesh.m_verts[0].m_position, submesh.m_verts.getSize(), sizeof(submesh.m_verts[0]));
		submesh.m_sphereCenter = s.getCenter().xyz();
		submesh.m_sphereRadius = max(kEpsilonf * 10.0f, s.getRadius());

		// Fix normals
		fixNormals(m_normalsMergeAngle, submesh);

		//
		// Load indices
		//
		{
			ANKI_ASSERT(primitive->indices);
			if(primitive->indices->count == 0 || (primitive->indices->count % 3) != 0)
			{
				ANKI_IMPORTER_LOGE("Incorect index count: %lu", primitive->indices->count);
				return Error::kUserData;
			}
			submesh.m_indices.resize(U32(primitive->indices->count));
			const U8* base = static_cast<const U8*>(primitive->indices->buffer_view->buffer->data) + primitive->indices->offset
							 + primitive->indices->buffer_view->offset;
			for(U32 i = 0; i < primitive->indices->count; ++i)
			{
				U32 idx;
				if(primitive->indices->component_type == cgltf_component_type_r_32u)
				{
					idx = *reinterpret_cast<const U32*>(base + sizeof(U32) * i);
				}
				else if(primitive->indices->component_type == cgltf_component_type_r_16u)
				{
					idx = *reinterpret_cast<const U16*>(base + sizeof(U16) * i);
				}
				else
				{
					ANKI_ASSERT(0);
					idx = 0;
				}

				submesh.m_indices[i] = idx;
			}
		}

		// Re-index meshes now
		if(m_optimizeMeshes)
		{
			reindexSubmesh(submesh);
			vertCount = submesh.m_verts.getSize();
		}

		// Optimize
		if(m_optimizeMeshes)
		{
			optimizeSubmesh(submesh);
		}

		// Finalize
		if(submesh.m_indices.getSize() == 0 || submesh.m_verts.getSize() == 0)
		{
			ANKI_UTIL_LOGE("Mesh degenerate: %s", meshName.cstr());
			return Error::kUserData;
		}
	} // for all GLTF primitives

	// Generate submeshes for the other LODs
	ANKI_ASSERT(m_lodCount <= kMaxLodCount && m_lodCount > 0);
	U32 maxLod = 0;
	for(U32 lod = 1; lod < m_lodCount; ++lod)
	{
		if(skipMeshLod(mesh, lod))
		{
			break;
		}

		for(const SubMesh& lod0Submesh : submeshes[0])
		{
			SubMesh& newSubmesh = *submeshes[lod].emplaceBack();
			newSubmesh = lod0Submesh; // Copy LOD0 data to new submesh

			decimateSubmesh(computeLodFactor(lod), newSubmesh);
		}

		maxLod = lod;
	}

	ANKI_IMPORTER_LOGV("Mesh lod count: %s %u", fname.cstr(), maxLod + 1);

	// Meshletize
	for(U32 lod = 0; lod < m_lodCount; ++lod)
	{
		for(SubMesh& submesh : submeshes[lod])
		{
			generateMeshlets(submesh);
		}
	}

	// Start writing the file
	File file;
	ANKI_CHECK(file.open(fname.toCString(), FileOpenFlag::kWrite | FileOpenFlag::kBinary));

	// Populate the header
	MeshBinaryHeader header;
	memset(&header, 0, sizeof(header));
	memcpy(&header.m_magic[0], kMeshMagic, 8);

	header.m_flags = (isConvex(submeshes[0])) ? MeshBinaryFlag::kConvex : MeshBinaryFlag::kNone;
	header.m_indexType = IndexType::kU16;
	header.m_meshletPrimitiveFormat = Format::kR8G8B8A8_Uint;
	header.m_subMeshCount = U32(submeshes[0].getSize());
	header.m_lodCount = maxLod + 1;
	header.m_maxPrimitivesPerMeshlet = kMaxPrimitivesPerMeshlet;
	header.m_maxVerticesPerMeshlet = kMaxVerticesPerMeshlet;
	header.m_boundingVolume.m_aabbMin = aabbMin;
	header.m_boundingVolume.m_aabbMax = aabbMax;
	const Sphere s = computeBoundingSphere(&allPositions[0], allPositions.getSize(), sizeof(allPositions[0]));
	header.m_boundingVolume.m_sphereCenter = s.getCenter().xyz();
	header.m_boundingVolume.m_sphereRadius = s.getRadius();

	// Compute the pos scale and transform. The scale is uniform because it's applied to the model matrix of the object
	F32 posScale = kMinF32;
	for(U c = 0; c < 3; c++)
	{
		posScale = max(posScale, aabbMax[c] - aabbMin[c]);
	}
	posScale = (posScale < 1.0f) ? 1.0f : (1.0f / posScale);
	const Vec3 posTranslation = -aabbMin;

	writeVertexAttribAndBufferInfoToHeader(VertexStreamId::kPosition, header, Vec4(1.0f / posScale), (-posTranslation).xyz1());
	writeVertexAttribAndBufferInfoToHeader(VertexStreamId::kNormal, header);
	writeVertexAttribAndBufferInfoToHeader(VertexStreamId::kUv, header);
	if(hasBoneWeights)
	{
		writeVertexAttribAndBufferInfoToHeader(VertexStreamId::kBoneIds, header);
		writeVertexAttribAndBufferInfoToHeader(VertexStreamId::kBoneWeights, header);
	}

	// Write sub meshes
	ImporterDynamicArray<MeshBinarySubMesh> outSubmeshes;
	outSubmeshes.resize(U32(submeshes[0].getSize()));

	for(U32 submeshIdx = 0; submeshIdx < outSubmeshes.getSize(); ++submeshIdx)
	{
		MeshBinarySubMesh& out = outSubmeshes[submeshIdx];
		memset(&out, 0, sizeof(out));

		for(U32 lod = 0; lod <= maxLod; ++lod)
		{
			const SubMesh& inSubmesh = *(submeshes[lod].getBegin() + submeshIdx);

			if(lod == 0)
			{
				out.m_boundingVolume.m_aabbMin = inSubmesh.m_aabbMin;
				out.m_boundingVolume.m_aabbMax = inSubmesh.m_aabbMax;
				out.m_boundingVolume.m_sphereCenter = inSubmesh.m_sphereCenter;
				out.m_boundingVolume.m_sphereRadius = inSubmesh.m_sphereRadius;
			}

			out.m_lods[lod].m_firstIndex = header.m_indexCounts[lod];
			out.m_lods[lod].m_indexCount = inSubmesh.m_indices.getSize();
			out.m_lods[lod].m_firstMeshlet = header.m_meshletCounts[lod];
			out.m_lods[lod].m_meshletCount = inSubmesh.m_meshlets.getSize();

			header.m_indexCounts[lod] += inSubmesh.m_indices.getSize();
			header.m_vertexCounts[lod] += inSubmesh.m_verts.getSize();
			header.m_meshletCounts[lod] += inSubmesh.m_meshlets.getSize();
			header.m_meshletPrimitiveCounts[lod] += inSubmesh.m_localIndices.getSize() / 3u;
		}
	}

	ANKI_CHECK(file.write(&header, sizeof(header)));
	ANKI_CHECK(file.write(&outSubmeshes[0], outSubmeshes.getSizeInBytes()));

	// Write LODs
	for(I32 lod = I32(maxLod); lod >= 0; --lod)
	{
		// Write index buffer
		U32 vertCount = 0;
		for(const SubMesh& submesh : submeshes[lod])
		{
			ImporterDynamicArray<U16> indices;
			indices.resize(submesh.m_indices.getSize());
			for(U32 i = 0; i < indices.getSize(); ++i)
			{
				const U32 idx = submesh.m_indices[i] + vertCount;
				if(idx > kMaxU16)
				{
					ANKI_IMPORTER_LOGE("Only supports 16bit indices for now (%u): %s", idx, fname.cstr());
					return Error::kUserData;
				}

				indices[i] = U16(idx);
			}

			ANKI_CHECK(file.write(&indices[0], indices.getSizeInBytes()));
			vertCount += submesh.m_verts.getSize();
		}

		// Write positions
		for(const SubMesh& submesh : submeshes[lod])
		{
			ImporterDynamicArray<U16Vec4> positions;
			positions.resize(submesh.m_verts.getSize());
			for(U32 v = 0; v < submesh.m_verts.getSize(); ++v)
			{
				Vec3 localPos = (submesh.m_verts[v].m_position + posTranslation) * posScale;
				localPos = localPos.clamp(0.0f, 1.0f);
				localPos *= F32(kMaxU16);
				localPos = localPos.round();
				positions[v] = U16Vec4(localPos.xyz0());
			}

			ANKI_CHECK(file.write(&positions[0], positions.getSizeInBytes()));
		}

		// Write normals
		for(const SubMesh& submesh : submeshes[lod])
		{
			ImporterDynamicArray<U32> normals;
			normals.resize(submesh.m_verts.getSize());
			for(U32 v = 0; v < submesh.m_verts.getSize(); ++v)
			{
				normals[v] = packSnorm4x8(submesh.m_verts[v].m_normal.xyz0());
			}

			ANKI_CHECK(file.write(&normals[0], normals.getSizeInBytes()));
		}

		// Write UV
		for(const SubMesh& submesh : submeshes[lod])
		{
			ImporterDynamicArray<Vec2> uvs;
			uvs.resize(submesh.m_verts.getSize());
			for(U32 v = 0; v < submesh.m_verts.getSize(); ++v)
			{
				uvs[v] = submesh.m_verts[v].m_uv;
			}

			ANKI_CHECK(file.write(&uvs[0], uvs.getSizeInBytes()));
		}

		if(hasBoneWeights)
		{
			// Bone IDs
			for(const SubMesh& submesh : submeshes[lod])
			{
				ImporterDynamicArray<U8Vec4> boneids;
				boneids.resize(submesh.m_verts.getSize());
				for(U32 v = 0; v < submesh.m_verts.getSize(); ++v)
				{
					boneids[v] = U8Vec4(submesh.m_verts[v].m_boneIds);
				}

				ANKI_CHECK(file.write(&boneids[0], boneids.getSizeInBytes()));
			}

			// Bone weights
			for(const SubMesh& submesh : submeshes[lod])
			{
				ImporterDynamicArray<U32> boneWeights;
				boneWeights.resize(submesh.m_verts.getSize());
				for(U32 v = 0; v < submesh.m_verts.getSize(); ++v)
				{
					boneWeights[v] = packSnorm4x8(submesh.m_verts[v].m_boneWeights);
				}

				ANKI_CHECK(file.write(&boneWeights[0], boneWeights.getSizeInBytes()));
			}
		}

		// Write meshlets
		U32 primitiveCount = 0;
		U32 vertCount2 = 0;
		for(const SubMesh& submesh : submeshes[lod])
		{
			ImporterDynamicArray<MeshBinaryMeshlet> meshlets;
			meshlets.resize(submesh.m_meshlets.getSize());

			for(U32 v = 0; v < submesh.m_meshlets.getSize(); ++v)
			{
				MeshBinaryMeshlet& out = meshlets[v];
				const ImporterMeshlet& in = submesh.m_meshlets[v];

				out.m_firstPrimitive = primitiveCount;
				out.m_primitiveCount = in.m_localIndexCount / 3;
				primitiveCount += out.m_primitiveCount;

				out.m_firstVertex = vertCount2;
				out.m_vertexCount = in.m_vertexCount;
				vertCount2 += in.m_vertexCount;

				out.m_boundingVolume.m_sphereCenter = in.m_sphere.getCenter().xyz();
				out.m_boundingVolume.m_sphereRadius = in.m_sphere.getRadius();
				out.m_boundingVolume.m_aabbMin = in.m_aabb.getMin().xyz();
				out.m_boundingVolume.m_aabbMax = in.m_aabb.getMax().xyz();

				out.m_coneApex = in.m_coneApex;
				out.m_coneDirection = in.m_coneDir;
				out.m_coneAngle = in.m_coneAngle;
			}

			ANKI_CHECK(file.write(&meshlets[0], meshlets.getSizeInBytes()));
		}
		ANKI_ASSERT(vertCount2 == vertCount);
		ANKI_ASSERT(primitiveCount == header.m_meshletPrimitiveCounts[lod]);

		// Write local indices
		for(const SubMesh& submesh : submeshes[lod])
		{
			ImporterDynamicArray<U8> localIndices;
			localIndices.resize(submesh.m_localIndices.getSize() / 3 * sizeof(U8Vec4));

			U32 count = 0;
			for(U32 v = 0; v < submesh.m_localIndices.getSize(); v += 3)
			{
				localIndices[count++] = submesh.m_localIndices[v];
				localIndices[count++] = submesh.m_localIndices[v + 1];
				localIndices[count++] = submesh.m_localIndices[v + 2];
				localIndices[count++] = 0;
			}

			ANKI_CHECK(file.write(&localIndices[0], localIndices.getSizeInBytes()));
		}
	}

	return Error::kNone;
}

} // end namespace anki
