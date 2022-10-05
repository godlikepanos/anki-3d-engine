// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Importer/GltfImporter.h>
#include <AnKi/Util/StringList.h>
#include <AnKi/Collision/Plane.h>
#include <AnKi/Collision/Functions.h>
#include <AnKi/Resource/MeshBinary.h>
#include <AnKi/Shaders/Include/ModelTypes.h>
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

/// Align after laying a buffer in a file.
static Error alignBufferInFile(PtrSize bufferSize, File& file)
{
	const PtrSize alignedBufferSize = getAlignedRoundUp(kMeshBinaryBufferAlignment, bufferSize);
	const PtrSize extraBytes = alignedBufferSize - bufferSize;

	for(U32 i = 0; i < extraBytes; ++i)
	{
		U8 value = 0;
		ANKI_CHECK(file.write(&value, sizeof(value)));
	}

	return Error::kNone;
}

class TempVertex
{
public:
	Vec3 m_position;
	F32 m_padding0;
	Vec4 m_tangent;
	Vec4 m_boneWeights;
	Vec3 m_normal;
	Vec2 m_uv;
	U16Vec4 m_boneIds;
	F32 m_padding1;

	TempVertex()
	{
		zeroMemory(*this);
	}
};
static_assert(sizeof(TempVertex) == 5 * sizeof(Vec4), "Will be hashed");

class SubMesh
{
public:
	DynamicArrayRaii<TempVertex> m_verts;
	DynamicArrayRaii<U32> m_indices;

	Vec3 m_aabbMin = Vec3(kMaxF32);
	Vec3 m_aabbMax = Vec3(kMinF32);

	U32 m_firstIdx = kMaxU32;
	U32 m_idxCount = kMaxU32;

	SubMesh(BaseMemoryPool* pool)
		: m_verts(pool)
		, m_indices(pool)
	{
	}
};

static void reindexSubmesh(SubMesh& submesh, BaseMemoryPool* pool)
{
	const U32 vertSize = sizeof(submesh.m_verts[0]);

	DynamicArrayRaii<U32> remap(pool);
	remap.create(submesh.m_verts.getSize(), 0);

	const U32 vertCount = U32(meshopt_generateVertexRemap(&remap[0], &submesh.m_indices[0], submesh.m_indices.getSize(),
														  &submesh.m_verts[0], submesh.m_verts.getSize(), vertSize));

	DynamicArrayRaii<U32> newIdxArray(pool);
	newIdxArray.create(submesh.m_indices.getSize(), 0);
	DynamicArrayRaii<TempVertex> newVertArray(pool);
	newVertArray.create(vertCount);

	meshopt_remapIndexBuffer(&newIdxArray[0], &submesh.m_indices[0], submesh.m_indices.getSize(), &remap[0]);
	meshopt_remapVertexBuffer(&newVertArray[0], &submesh.m_verts[0], submesh.m_verts.getSize(), vertSize, &remap[0]);

	submesh.m_indices = std::move(newIdxArray);
	submesh.m_verts = std::move(newVertArray);
}

/// Optimize a submesh using meshoptimizer.
static void optimizeSubmesh(SubMesh& submesh, BaseMemoryPool* pool)
{
	const PtrSize vertSize = sizeof(submesh.m_verts[0]);

	// Vert cache
	{
		DynamicArrayRaii<U32> newIdxArray(pool);
		newIdxArray.create(submesh.m_indices.getSize(), 0);

		meshopt_optimizeVertexCache(&newIdxArray[0], &submesh.m_indices[0], submesh.m_indices.getSize(),
									submesh.m_verts.getSize());

		submesh.m_indices = std::move(newIdxArray);
	}

	// Overdraw
	{
		DynamicArrayRaii<U32> newIdxArray(pool);
		newIdxArray.create(submesh.m_indices.getSize(), 0);

		meshopt_optimizeOverdraw(&newIdxArray[0], &submesh.m_indices[0], submesh.m_indices.getSize(),
								 &submesh.m_verts[0].m_position.x(), submesh.m_verts.getSize(), vertSize, 1.05f);

		submesh.m_indices = std::move(newIdxArray);
	}

	// Vert fetch
	{
		DynamicArrayRaii<TempVertex> newVertArray(pool);
		newVertArray.create(submesh.m_verts.getSize());

		const U32 newVertCount = U32(meshopt_optimizeVertexFetch(&newVertArray[0],
																 &submesh.m_indices[0], // Inplace
																 submesh.m_indices.getSize(), &submesh.m_verts[0],
																 submesh.m_verts.getSize(), vertSize));

		if(newVertCount != submesh.m_verts.getSize())
		{
			newVertArray.resize(newVertCount);
		}
		ANKI_ASSERT(newVertArray.getSize() == newVertCount);

		submesh.m_verts = std::move(newVertArray);
	}
}

/// Decimate a submesh using meshoptimizer.
static void decimateSubmesh(F32 factor, SubMesh& submesh, BaseMemoryPool* pool)
{
	ANKI_ASSERT(factor > 0.0f && factor < 1.0f);
	const PtrSize targetIndexCount = PtrSize(F32(submesh.m_indices.getSize() / 3) * factor) * 3;
	if(targetIndexCount == 0)
	{
		return;
	}

	// Decimate
	DynamicArrayRaii<U32> newIndices(pool, submesh.m_indices.getSize());
	newIndices.resize(U32(meshopt_simplify(&newIndices[0], &submesh.m_indices[0], submesh.m_indices.getSize(),
										   &submesh.m_verts[0].m_position.x(), submesh.m_verts.getSize(),
										   sizeof(TempVertex), targetIndexCount, 1e-2f)));

	// Re-pack
	DynamicArrayRaii<U32> reindexedIndices(pool);
	DynamicArrayRaii<TempVertex> newVerts(pool);
	HashMapRaii<U32, U32> vertexStored(pool);
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

static void computeTangents(SubMesh& submesh)
{
	DynamicArrayRaii<Vec3> bitangents(&submesh.m_verts.getMemoryPool());
	const U32 vertCount = submesh.m_verts.getSize();
	bitangents.create(vertCount, Vec3(0.0f));

	for(U32 i = 0; i < submesh.m_indices.getSize(); i += 3)
	{
		const U32 i0 = submesh.m_indices[i + 0];
		const U32 i1 = submesh.m_indices[i + 1];
		const U32 i2 = submesh.m_indices[i + 2];

		const Vec3& v0 = submesh.m_verts[i0].m_position;
		const Vec3& v1 = submesh.m_verts[i1].m_position;
		const Vec3& v2 = submesh.m_verts[i2].m_position;
		const Vec3 edge01 = v1 - v0;
		const Vec3 edge02 = v2 - v0;

		const Vec2 uvedge01 = submesh.m_verts[i1].m_uv - submesh.m_verts[i0].m_uv;
		const Vec2 uvedge02 = submesh.m_verts[i2].m_uv - submesh.m_verts[i0].m_uv;

		F32 det = (uvedge01.y() * uvedge02.x()) - (uvedge01.x() * uvedge02.y());
		det = (isZero(det)) ? 0.0001f : (1.0f / det);

		Vec3 t = (edge02 * uvedge01.y() - edge01 * uvedge02.y()) * det;
		Vec3 b = (edge02 * uvedge01.x() - edge01 * uvedge02.x()) * det;

		if(t.getLengthSquared() < kEpsilonf)
		{
			t = Vec3(1.0f, 0.0f, 0.0f); // Something random
		}
		else
		{
			t.normalize();
		}

		if(b.getLengthSquared() < kEpsilonf)
		{
			b = Vec3(0.0f, 1.0f, 0.0f); // Something random
		}
		else
		{
			b.normalize();
		}

		submesh.m_verts[i0].m_tangent += Vec4(t, 0.0f);
		submesh.m_verts[i1].m_tangent += Vec4(t, 0.0f);
		submesh.m_verts[i2].m_tangent += Vec4(t, 0.0f);

		bitangents[i0] += b;
		bitangents[i1] += b;
		bitangents[i2] += b;
	}

	for(U32 i = 0; i < vertCount; ++i)
	{
		Vec3 t = Vec3(submesh.m_verts[i].m_tangent.xyz());
		const Vec3& n = submesh.m_verts[i].m_normal;
		Vec3& b = bitangents[i];

		if(t.getLengthSquared() < kEpsilonf)
		{
			t = Vec3(1.0f, 0.0f, 0.0f); // Something random
		}
		else
		{
			t.normalize();
		}

		if(b.getLengthSquared() < kEpsilonf)
		{
			b = Vec3(0.0f, 1.0f, 0.0f); // Something random
		}
		else
		{
			b.normalize();
		}

		const F32 w = ((n.cross(t)).dot(b) < 0.0f) ? 1.0f : -1.0f;
		submesh.m_verts[i].m_tangent = Vec4(t, w);
	}
}

static Bool isConvex(const List<SubMesh>& submeshes)
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

U32 GltfImporter::getMeshTotalVertexCount(const cgltf_mesh& mesh)
{
	U32 totalVertexCount = 0;

	for(const cgltf_primitive* primitive = mesh.primitives; primitive < mesh.primitives + mesh.primitives_count;
		++primitive)
	{
		totalVertexCount += U32(primitive->attributes[0].data->count);
	}

	return totalVertexCount;
}

Error GltfImporter::writeMesh(const cgltf_mesh& mesh, U32 lod, F32 decimateFactor)
{
	StringRaii fname(m_pool);
	fname.sprintf("%s%s", m_outDir.cstr(), computeMeshResourceFilename(mesh, lod).cstr());
	ANKI_IMPORTER_LOGV("Importing mesh (%s, decimate factor %f): %s",
					   (m_optimizeMeshes) ? "optimize" : "WON'T optimize", decimateFactor, fname.cstr());

	ListRaii<SubMesh> submeshes(m_pool);
	U32 totalIndexCount = 0;
	U32 totalVertexCount = 0;
	Vec3 aabbMin(kMaxF32);
	Vec3 aabbMax(kMinF32);
	F32 maxUvDistance = kMinF32;
	F32 minUvDistance = kMaxF32;
	Bool hasBoneWeights = false;

	// Iterate primitives. Every primitive is a submesh
	for(const cgltf_primitive* primitive = mesh.primitives; primitive < mesh.primitives + mesh.primitives_count;
		++primitive)
	{
		if(primitive->type != cgltf_primitive_type_triangles)
		{
			ANKI_IMPORTER_LOGE("Expecting triangles got %d", primitive->type);
			return Error::kUserData;
		}

		SubMesh& submesh = *submeshes.emplaceBack(m_pool);

		U minVertCount = kMaxU;
		U maxVertCount = kMinU;
		for(const cgltf_attribute* attrib = primitive->attributes;
			attrib < primitive->attributes + primitive->attributes_count; ++attrib)
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
		submesh.m_verts.create(vertCount);

		//
		// Gather positions + normals + UVs
		//
		for(const cgltf_attribute* attrib = primitive->attributes;
			attrib < primitive->attributes + primitive->attributes_count; ++attrib)
		{
			if(attrib->type == cgltf_attribute_type_position)
			{
				U32 count = 0;
				ANKI_CHECK(checkAttribute<Vec3>(*attrib));
				visitAccessor<Vec3>(*attrib->data, [&](const Vec3& pos) {
					submesh.m_aabbMin = submesh.m_aabbMin.min(pos);
					submesh.m_aabbMax = submesh.m_aabbMax.max(pos);
					submesh.m_verts[count++].m_position = pos;
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
					maxUvDistance = max(maxUvDistance, max(uv.x(), uv.y()));
					minUvDistance = min(minUvDistance, min(uv.x(), uv.y()));
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
		// Bump aabbMax a bit
		submesh.m_aabbMax += kEpsilonf * 10.0f;
		aabbMax = aabbMax.max(submesh.m_aabbMax);

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
			submesh.m_indices.create(U32(primitive->indices->count));
			const U8* base = static_cast<const U8*>(primitive->indices->buffer_view->buffer->data)
							 + primitive->indices->offset + primitive->indices->buffer_view->offset;
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

		// Re-index meshes now and
		// - before the tanget calculation because that will create many unique verts
		// - after normal fix because that will create verts with same attributes
		if(m_optimizeMeshes || decimateFactor < 1.0f)
		{
			reindexSubmesh(submesh, m_pool);
			vertCount = submesh.m_verts.getSize();
		}

		// Compute tangent
		computeTangents(submesh);

		// Optimize
		if(m_optimizeMeshes)
		{
			optimizeSubmesh(submesh, m_pool);
		}

		// Simplify
		if(decimateFactor < 1.0f)
		{
			decimateSubmesh(decimateFactor, submesh, m_pool);
		}

		// Finalize
		if(submesh.m_indices.getSize() == 0 || submesh.m_verts.getSize() == 0)
		{
			// Digenerate
			submeshes.popBack();
		}
		else
		{
			// Finalize
			submesh.m_firstIdx = totalIndexCount;
			submesh.m_idxCount = submesh.m_indices.getSize();
			totalIndexCount += submesh.m_idxCount;
			totalVertexCount += submesh.m_verts.getSize();
		}
	}

	if(submeshes.getSize() == 0)
	{
		ANKI_IMPORTER_LOGE("Mesh contains degenerate geometry");
		return Error::kUserData;
	}

	// Find if it's a convex shape
	const Bool convex = isConvex(submeshes);

	// Chose the formats of the attributes
	MeshBinaryHeader header;
	memset(&header, 0, sizeof(header));
	{
		// Positions
		MeshBinaryVertexAttribute& posa = header.m_vertexAttributes[VertexAttributeId::kPosition];
		posa.m_bufferBinding = 0;
		posa.m_format = Format::kR32G32B32_Sfloat;
		posa.m_relativeOffset = 0;
		posa.m_scale = 1.0f;

		// Normals
		MeshBinaryVertexAttribute& na = header.m_vertexAttributes[VertexAttributeId::kNormal];
		na.m_bufferBinding = 1;
		na.m_format = Format::kA2B10G10R10_Snorm_Pack32;
		na.m_relativeOffset = 0;
		na.m_scale = 1.0f;

		// Tangents
		MeshBinaryVertexAttribute& ta = header.m_vertexAttributes[VertexAttributeId::kTangent];
		ta.m_bufferBinding = 1;
		ta.m_format = Format::kA2B10G10R10_Snorm_Pack32;
		ta.m_relativeOffset = sizeof(U32);
		ta.m_scale = 1.0f;

		// UVs
		MeshBinaryVertexAttribute& uva = header.m_vertexAttributes[VertexAttributeId::kUv0];
		uva.m_bufferBinding = 1;
		uva.m_format = Format::kR32G32_Sfloat;
		uva.m_relativeOffset = sizeof(U32) * 2;
		uva.m_scale = 1.0f;

		// Bone weight
		if(hasBoneWeights)
		{
			MeshBinaryVertexAttribute& bidxa = header.m_vertexAttributes[VertexAttributeId::kBoneIndices];
			bidxa.m_bufferBinding = 2;
			bidxa.m_format = Format::kR8G8B8A8_Uint;
			bidxa.m_relativeOffset = 0;
			bidxa.m_scale = 1.0f;

			MeshBinaryVertexAttribute& wa = header.m_vertexAttributes[VertexAttributeId::kBoneWeights];
			wa.m_bufferBinding = 2;
			wa.m_format = Format::kR8G8B8A8_Unorm;
			wa.m_relativeOffset = sizeof(U8Vec4);
			wa.m_scale = 1.0f;
		}
	}

	// Arange the attributes into vert buffers
	{
		// First buff has positions
		header.m_vertexBuffers[0].m_vertexStride = sizeof(Vec3);
		++header.m_vertexBufferCount;

		// 2nd buff has normal + tangent + texcoords
		header.m_vertexBuffers[1].m_vertexStride = sizeof(MainVertex);
		++header.m_vertexBufferCount;

		// 3rd has bone weights
		if(hasBoneWeights)
		{
			header.m_vertexBuffers[2].m_vertexStride = sizeof(BoneInfoVertex);
			++header.m_vertexBufferCount;
		}
	}

	// Write some other header stuff
	{
		memcpy(&header.m_magic[0], kMeshMagic, 8);
		header.m_flags = MeshBinaryFlag::kNone;
		if(convex)
		{
			header.m_flags |= MeshBinaryFlag::kConvex;
		}
		header.m_indexType = IndexType::kU16;
		header.m_totalIndexCount = totalIndexCount;
		header.m_totalVertexCount = totalVertexCount;
		header.m_subMeshCount = U32(submeshes.getSize());
		header.m_aabbMin = aabbMin;
		header.m_aabbMax = aabbMax;
	}

	// Open file
	File file;
	ANKI_CHECK(file.open(fname.toCString(), FileOpenFlag::kWrite | FileOpenFlag::kBinary));

	// Write header
	ANKI_CHECK(file.write(&header, sizeof(header)));

	// Write sub meshes
	for(const SubMesh& in : submeshes)
	{
		MeshBinarySubMesh out;
		out.m_firstIndex = in.m_firstIdx;
		out.m_indexCount = in.m_idxCount;
		out.m_aabbMin = in.m_aabbMin;
		out.m_aabbMax = in.m_aabbMax;

		ANKI_CHECK(file.write(&out, sizeof(out)));
	}

	// Write indices
	U32 vertCount = 0;
	for(const SubMesh& submesh : submeshes)
	{
		DynamicArrayRaii<U16> indices(m_pool);
		indices.create(submesh.m_indices.getSize());
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

	ANKI_CHECK(alignBufferInFile(header.m_totalIndexCount * sizeof(U16), file));

	// Write position vert buffer
	for(const SubMesh& submesh : submeshes)
	{
		DynamicArrayRaii<Vec3> positions(m_pool);
		positions.create(submesh.m_verts.getSize());
		for(U32 v = 0; v < submesh.m_verts.getSize(); ++v)
		{
			positions[v] = submesh.m_verts[v].m_position;
		}
		ANKI_CHECK(file.write(&positions[0], positions.getSizeInBytes()));
	}

	ANKI_CHECK(alignBufferInFile(header.m_totalVertexCount * sizeof(Vec3), file));

	// Write the 2nd vert buffer
	for(const SubMesh& submesh : submeshes)
	{
		DynamicArrayRaii<MainVertex> verts(m_pool);
		verts.create(submesh.m_verts.getSize());

		for(U32 i = 0; i < verts.getSize(); ++i)
		{
			const Vec3& normal = submesh.m_verts[i].m_normal;
			const Vec4& tangent = submesh.m_verts[i].m_tangent;
			const Vec2& uv = submesh.m_verts[i].m_uv;

			verts[i].m_normal = packColorToR10G10B10A2SNorm(normal.x(), normal.y(), normal.z(), 0.0f);
			verts[i].m_tangent = packColorToR10G10B10A2SNorm(tangent.x(), tangent.y(), tangent.z(), tangent.w());
			verts[i].m_uv0 = uv;
		}

		ANKI_CHECK(file.write(&verts[0], verts.getSizeInBytes()));
	}

	ANKI_CHECK(alignBufferInFile(header.m_totalVertexCount * sizeof(MainVertex), file));

	// Write 3rd vert buffer
	if(hasBoneWeights)
	{
		for(const SubMesh& submesh : submeshes)
		{
			DynamicArrayRaii<BoneInfoVertex> verts(m_pool);
			verts.create(submesh.m_verts.getSize());

			for(U32 i = 0; i < verts.getSize(); ++i)
			{
				BoneInfoVertex vert;

				for(U32 c = 0; c < 4; ++c)
				{
					if(submesh.m_verts[i].m_boneIds[c] > 0XFF)
					{
						ANKI_IMPORTER_LOGE("Only 256 bones are supported");
						return Error::kUserData;
					}

					vert.m_boneIndices[c] = U8(submesh.m_verts[i].m_boneIds[c]);
					vert.m_boneWeights[c] = U8(submesh.m_verts[i].m_boneWeights[c] * F32(kMaxU8));
				}

				verts[i] = vert;
			}

			ANKI_CHECK(file.write(&verts[0], verts.getSizeInBytes()));
		}

		ANKI_CHECK(alignBufferInFile(header.m_totalVertexCount * sizeof(BoneInfoVertex), file));
	}

	return Error::kNone;
}

} // end namespace anki
