// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/importer/GltfImporter.h>
#include <anki/util/StringList.h>
#include <anki/collision/Plane.h>
#include <anki/collision/Functions.h>
#include <anki/resource/MeshLoader.h>
#include <meshoptimizer/meshoptimizer.h>

namespace anki
{

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
Error checkAttribute(const cgltf_attribute& attrib)
{
	if(cgltfComponentCount(attrib.data->type) != T::COMPONENT_COUNT)
	{
		ANKI_GLTF_LOGE("Wrong component count for attribute: %s", attrib.name);
		return Error::USER_DATA;
	}

	if(cgltfComponentSize(attrib.data->component_type) != sizeof(typename T::Scalar))
	{
		ANKI_GLTF_LOGE("Incompatible type: %s", attrib.name);
		return Error::USER_DATA;
	}

	ANKI_ASSERT(attrib.data);
	const U count = attrib.data->count;
	if(count == 0)
	{
		ANKI_GLTF_LOGE("Zero vertex count");
		return Error::USER_DATA;
	}

	return Error::NONE;
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
	DynamicArrayAuto<TempVertex> m_verts;
	DynamicArrayAuto<U32> m_indices;

	Vec3 m_aabbMin{MAX_F32};
	Vec3 m_aabbMax{MIN_F32};

	U32 m_firstIdx = MAX_U32;
	U32 m_idxCount = MAX_U32;

	SubMesh(GenericMemoryPoolAllocator<U8>& alloc)
		: m_verts(alloc)
		, m_indices(alloc)
	{
	}
};

struct WeightVertex
{
	U16Vec4 m_boneIndices{MAX_U16};
	U8Vec4 m_weights{0_U8};
};

static void reindexSubmesh(SubMesh& submesh, GenericMemoryPoolAllocator<U8> alloc)
{
	const U32 vertSize = sizeof(submesh.m_verts[0]);

	DynamicArrayAuto<U32> remap(alloc);
	remap.create(submesh.m_verts.getSize(), 0);

	const U32 vertCount = U32(meshopt_generateVertexRemap(&remap[0], &submesh.m_indices[0], submesh.m_indices.getSize(),
														  &submesh.m_verts[0], submesh.m_verts.getSize(), vertSize));

	DynamicArrayAuto<U32> newIdxArray(alloc);
	newIdxArray.create(submesh.m_indices.getSize(), 0);
	DynamicArrayAuto<TempVertex> newVertArray(alloc);
	newVertArray.create(vertCount);

	meshopt_remapIndexBuffer(&newIdxArray[0], &submesh.m_indices[0], submesh.m_indices.getSize(), &remap[0]);
	meshopt_remapVertexBuffer(&newVertArray[0], &submesh.m_verts[0], submesh.m_verts.getSize(), vertSize, &remap[0]);

	submesh.m_indices = std::move(newIdxArray);
	submesh.m_verts = std::move(newVertArray);
}

/// Optimize a submesh using meshoptimizer.
static void optimizeSubmesh(SubMesh& submesh, GenericMemoryPoolAllocator<U8> alloc)
{
	const PtrSize vertSize = sizeof(submesh.m_verts[0]);

	// Vert cache
	{
		DynamicArrayAuto<U32> newIdxArray(alloc);
		newIdxArray.create(submesh.m_indices.getSize(), 0);

		meshopt_optimizeVertexCache(&newIdxArray[0], &submesh.m_indices[0], submesh.m_indices.getSize(),
									submesh.m_verts.getSize());

		submesh.m_indices = std::move(newIdxArray);
	}

	// Overdraw
	{
		DynamicArrayAuto<U32> newIdxArray(alloc);
		newIdxArray.create(submesh.m_indices.getSize(), 0);

		meshopt_optimizeOverdraw(&newIdxArray[0], &submesh.m_indices[0], submesh.m_indices.getSize(),
								 &submesh.m_verts[0].m_position.x(), submesh.m_verts.getSize(), vertSize, 1.05f);

		submesh.m_indices = std::move(newIdxArray);
	}

	// Vert fetch
	{
		DynamicArrayAuto<TempVertex> newVertArray(alloc);
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
static void decimateSubmesh(F32 factor, SubMesh& submesh, GenericMemoryPoolAllocator<U8> alloc)
{
	ANKI_ASSERT(factor > 0.0f && factor < 1.0f);
	const PtrSize targetIndexCount = PtrSize(F32(submesh.m_indices.getSize() / 3) * factor) * 3;
	if(targetIndexCount == 0)
	{
		return;
	}

	// Decimate
	DynamicArrayAuto<U32> newIndices(alloc, submesh.m_indices.getSize());
	newIndices.resize(U32(meshopt_simplify(&newIndices[0], &submesh.m_indices[0], submesh.m_indices.getSize(),
										   &submesh.m_verts[0].m_position.x(), submesh.m_verts.getSize(),
										   sizeof(TempVertex), targetIndexCount, 1e-2f)));

	// Re-pack
	DynamicArrayAuto<U32> reindexedIndices(alloc);
	DynamicArrayAuto<TempVertex> newVerts(alloc);
	HashMapAuto<U32, U32> vertexStored(alloc);
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

Error GltfImporter::writeMesh(const cgltf_mesh& mesh, CString nameOverride, F32 decimateFactor)
{
	StringAuto fname(m_alloc);
	fname.sprintf("%s%s.ankimesh", m_outDir.cstr(), (nameOverride.isEmpty()) ? mesh.name : nameOverride.cstr());
	ANKI_GLTF_LOGI("Importing mesh (%s, decimate factor %f): %s", (m_optimizeMeshes) ? "optimze" : "WON'T optimize",
				   decimateFactor, fname.cstr());

	ListAuto<SubMesh> submeshes(m_alloc);
	U32 totalIndexCount = 0;
	U32 totalVertexCount = 0;
	Vec3 aabbMin(MAX_F32);
	Vec3 aabbMax(MIN_F32);
	F32 maxUvDistance = MIN_F32;
	F32 minUvDistance = MAX_F32;
	Bool hasBoneWeights = false;

	// Iterate primitives. Every primitive is a submesh
	for(const cgltf_primitive* primitive = mesh.primitives; primitive < mesh.primitives + mesh.primitives_count;
		++primitive)
	{
		if(primitive->type != cgltf_primitive_type_triangles)
		{
			ANKI_GLTF_LOGE("Expecting triangles got %d", primitive->type);
			return Error::USER_DATA;
		}

		SubMesh& submesh = *submeshes.emplaceBack(m_alloc);

		U minVertCount = MAX_U;
		U maxVertCount = MIN_U;
		for(const cgltf_attribute* attrib = primitive->attributes;
			attrib < primitive->attributes + primitive->attributes_count; ++attrib)
		{
			minVertCount = min(minVertCount, U(attrib->data->count));
			maxVertCount = max(maxVertCount, U(attrib->data->count));
		}

		if(maxVertCount == 0 || minVertCount != maxVertCount)
		{
			ANKI_GLTF_LOGE("Wrong number of vertices");
			return Error::USER_DATA;
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
				visitAccessor<Vec3>(*attrib->data,
									[&](const Vec3& normal) { submesh.m_verts[count++].m_normal = normal; });
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
				ANKI_CHECK(checkAttribute<U16Vec4>(*attrib));
				visitAccessor<U16Vec4>(*attrib->data,
									   [&](const U16Vec4& x) { submesh.m_verts[count++].m_boneIds = x; });
				hasBoneWeights = true;
			}
			else if(attrib->type == cgltf_attribute_type_weights)
			{
				U32 count = 0;
				ANKI_CHECK(checkAttribute<Vec4>(*attrib));
				visitAccessor<Vec4>(*attrib->data,
									[&](const Vec4& bw) { submesh.m_verts[count++].m_boneWeights = bw; });
			}
			else
			{
				ANKI_GLTF_LOGW("Ignoring attribute: %s", attrib->name);
			}
		}

		aabbMin = aabbMin.min(submesh.m_aabbMin);
		// Bump aabbMax a bit
		submesh.m_aabbMax += EPSILON * 10.0f;
		aabbMax = aabbMax.max(submesh.m_aabbMax);

		//
		// Fix normals. If normal A and normal B have the same position then try to merge them
		//
		for(U32 v = 0; v < vertCount; ++v)
		{
			const Vec3& pos = submesh.m_verts[v].m_position;
			Vec3& normal = submesh.m_verts[v].m_normal;

			for(U32 prevV = 0; prevV < v; ++prevV)
			{
				const Vec3& otherPos = submesh.m_verts[prevV].m_position;

				// Check the positions dist
				const F32 posDist = (otherPos - pos).getLengthSquared();
				if(posDist > EPSILON * EPSILON)
				{
					continue;
				}

				// Check angle of the normals
				Vec3& otherNormal = submesh.m_verts[prevV].m_normal;
				const F32 ang = acos(clamp(otherNormal.dot(normal), -1.0f, 1.0f));
				if(ang > m_normalsMergeAngle)
				{
					continue;
				}

				// Merge normals
				const Vec3 newNormal = (otherNormal + normal).getNormalized();
				normal = newNormal;
				otherNormal = newNormal;
			}
		}

		//
		// Load indices
		//
		{
			ANKI_ASSERT(primitive->indices);
			if(primitive->indices->count == 0 || (primitive->indices->count % 3) != 0)
			{
				ANKI_GLTF_LOGE("Incorect index count: %lu", primitive->indices->count);
				return Error::USER_DATA;
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
			reindexSubmesh(submesh, m_alloc);
			vertCount = submesh.m_verts.getSize();
		}

		//
		// Compute tangent
		//
		{
			DynamicArrayAuto<Vec3> bitangents(m_alloc);
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

				if(t.getLengthSquared() < EPSILON)
				{
					t = Vec3(1.0f, 0.0f, 0.0f); // Something random
				}
				else
				{
					t.normalize();
				}

				if(b.getLengthSquared() < EPSILON)
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

				if(t.getLengthSquared() < EPSILON)
				{
					t = Vec3(1.0f, 0.0f, 0.0f); // Something random
				}
				else
				{
					t.normalize();
				}

				if(b.getLengthSquared() < EPSILON)
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

		// Optimize
		if(m_optimizeMeshes)
		{
			optimizeSubmesh(submesh, m_alloc);
		}

		// Simplify
		if(decimateFactor < 1.0f)
		{
			decimateSubmesh(decimateFactor, submesh, m_alloc);
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
		ANKI_GLTF_LOGE("Mesh contains degenerate geometry");
		return Error::USER_DATA;
	}

	// Find if it's a convex shape
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

			if(computeTriangleArea(v0, v1, v2) <= EPSILON)
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
					if(test > EPSILON)
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

	// Chose the formats of the attributes
	MeshBinaryFile::Header header = {};
	{
		// Positions
		const Vec3 dist3d = aabbMin.abs().max(aabbMax.abs());
		const F32 maxPositionDistance = max(max(dist3d.x(), dist3d.y()), dist3d.z());
		MeshBinaryFile::VertexAttribute& posa = header.m_vertexAttributes[VertexAttributeLocation::POSITION];
		posa.m_bufferBinding = 0;
		posa.m_format = (maxPositionDistance < 2.0) ? Format::R16G16B16A16_SFLOAT : Format::R32G32B32_SFLOAT;
		posa.m_relativeOffset = 0;
		posa.m_scale = 1.0f;

		// Normals
		MeshBinaryFile::VertexAttribute& na = header.m_vertexAttributes[VertexAttributeLocation::NORMAL];
		na.m_bufferBinding = 1;
		na.m_format = Format::A2B10G10R10_SNORM_PACK32;
		na.m_relativeOffset = 0;
		na.m_scale = 1.0f;

		// Tangents
		MeshBinaryFile::VertexAttribute& ta = header.m_vertexAttributes[VertexAttributeLocation::TANGENT];
		ta.m_bufferBinding = 1;
		ta.m_format = Format::A2B10G10R10_SNORM_PACK32;
		ta.m_relativeOffset = sizeof(U32);
		ta.m_scale = 1.0;

		// UVs
		MeshBinaryFile::VertexAttribute& uva = header.m_vertexAttributes[VertexAttributeLocation::UV];
		uva.m_bufferBinding = 1;
		if(minUvDistance >= 0.0 && maxUvDistance <= 1.0)
		{
			uva.m_format = Format::R16G16_UNORM;
		}
		else
		{
			uva.m_format = Format::R16G16_SFLOAT;
		}
		uva.m_relativeOffset = sizeof(U32) * 2;
		uva.m_scale = 1.0f;

		// Bone weight
		if(hasBoneWeights)
		{
			MeshBinaryFile::VertexAttribute& bidxa = header.m_vertexAttributes[VertexAttributeLocation::BONE_INDICES];
			bidxa.m_bufferBinding = 2;
			bidxa.m_format = Format::R16G16B16A16_UINT;
			bidxa.m_relativeOffset = 0;
			bidxa.m_scale = 1.0f;

			MeshBinaryFile::VertexAttribute& wa = header.m_vertexAttributes[VertexAttributeLocation::BONE_WEIGHTS];
			wa.m_bufferBinding = 2;
			wa.m_format = Format::R8G8B8A8_UNORM;
			wa.m_relativeOffset = sizeof(U16Vec4);
			wa.m_scale = 1.0f;
		}
	}

	// Arange the attributes into vert buffers
	{
		// First buff has positions
		const MeshBinaryFile::VertexAttribute& posa = header.m_vertexAttributes[VertexAttributeLocation::POSITION];
		if(posa.m_format == Format::R32G32B32_SFLOAT)
		{
			header.m_vertexBuffers[0].m_vertexStride = sizeof(F32) * 3;
		}
		else if(posa.m_format == Format::R16G16B16A16_SFLOAT)
		{
			header.m_vertexBuffers[0].m_vertexStride = sizeof(U16) * 4;
		}
		else
		{
			ANKI_ASSERT(0);
		}
		++header.m_vertexBufferCount;

		// 2nd buff has normal + tangent + texcoords
		header.m_vertexBuffers[1].m_vertexStride = sizeof(U32) * 2 + sizeof(U16) * 2;
		++header.m_vertexBufferCount;

		// 3rd has bone weights
		if(hasBoneWeights)
		{
			header.m_vertexBuffers[2].m_vertexStride = sizeof(WeightVertex);
			++header.m_vertexBufferCount;
		}
	}

	// Write some other header stuff
	{
		memcpy(&header.m_magic[0], MeshBinaryFile::MAGIC, 8);
		header.m_flags = MeshBinaryFile::Flag::NONE;
		if(convex)
		{
			header.m_flags |= MeshBinaryFile::Flag::CONVEX;
		}
		header.m_indexType = IndexType::U16;
		header.m_totalIndexCount = totalIndexCount;
		header.m_totalVertexCount = totalVertexCount;
		header.m_subMeshCount = U32(submeshes.getSize());
		header.m_aabbMin = aabbMin;
		header.m_aabbMax = aabbMax;
	}

	// Open file
	File file;
	ANKI_CHECK(file.open(fname.toCString(), FileOpenFlag::WRITE | FileOpenFlag::BINARY));

	// Write header
	ANKI_CHECK(file.write(&header, sizeof(header)));

	// Write sub meshes
	for(const SubMesh& in : submeshes)
	{
		MeshBinaryFile::SubMesh out;
		out.m_firstIndex = in.m_firstIdx;
		out.m_indexCount = in.m_idxCount;
		out.m_aabbMin = in.m_aabbMin;
		out.m_aabbMax = in.m_aabbMax;

		ANKI_CHECK(file.write(&out, sizeof(out)));
	}

	// Write indices
	for(const SubMesh& submesh : submeshes)
	{
		DynamicArrayAuto<U16> indices(m_alloc);
		indices.create(submesh.m_indices.getSize());
		U32 vertCount = 0;
		for(U32 i = 0; i < indices.getSize(); ++i)
		{
			const U32 idx = submesh.m_indices[i] + vertCount;
			if(idx > MAX_U16)
			{
				ANKI_GLTF_LOGE("Only supports 16bit indices for now");
				return Error::USER_DATA;
			}

			indices[i] = U16(idx);
		}

		ANKI_CHECK(file.write(&indices[0], indices.getSizeInBytes()));
		vertCount += submesh.m_verts.getSize();
	}

	// Write first vert buffer
	for(const SubMesh& submesh : submeshes)
	{
		const MeshBinaryFile::VertexAttribute& posa = header.m_vertexAttributes[VertexAttributeLocation::POSITION];
		if(posa.m_format == Format::R32G32B32_SFLOAT)
		{
			DynamicArrayAuto<Vec3> positions(m_alloc);
			positions.create(submesh.m_verts.getSize());
			for(U32 v = 0; v < submesh.m_verts.getSize(); ++v)
			{
				positions[v] = submesh.m_verts[v].m_position;
			}
			ANKI_CHECK(file.write(&positions[0], positions.getSizeInBytes()));
		}
		else if(posa.m_format == Format::R16G16B16A16_SFLOAT)
		{
			DynamicArrayAuto<HVec4> pos16(m_alloc);
			pos16.create(submesh.m_verts.getSize());

			for(U32 v = 0; v < submesh.m_verts.getSize(); ++v)
			{
				pos16[v] = HVec4(F16(submesh.m_verts[v].m_position.x()), F16(submesh.m_verts[v].m_position.y()),
								 F16(submesh.m_verts[v].m_position.z()), F16(0.0f));
			}

			ANKI_CHECK(file.write(&pos16[0], pos16.getSizeInBytes()));
		}
		else
		{
			ANKI_ASSERT(0);
		}
	}

	// Write the 2nd vert buffer
	for(const SubMesh& submesh : submeshes)
	{
		struct Vert
		{
			U32 m_n;
			U32 m_t;
			U16 m_uv[2];
		};

		DynamicArrayAuto<Vert> verts(m_alloc);
		verts.create(submesh.m_verts.getSize());

		for(U32 i = 0; i < verts.getSize(); ++i)
		{
			const Vec3& normal = submesh.m_verts[i].m_normal;
			const Vec4& tangent = submesh.m_verts[i].m_tangent;
			const Vec2& uv = submesh.m_verts[i].m_uv;

			verts[i].m_n = packColorToR10G10B10A2SNorm(normal.x(), normal.y(), normal.z(), 0.0f);
			verts[i].m_t = packColorToR10G10B10A2SNorm(tangent.x(), tangent.y(), tangent.z(), tangent.w());

			const Format uvfmt = header.m_vertexAttributes[VertexAttributeLocation::UV].m_format;
			if(uvfmt == Format::R16G16_UNORM)
			{
				assert(uv[0] <= 1.0 && uv[0] >= 0.0 && uv[1] <= 1.0 && uv[1] >= 0.0);
				verts[i].m_uv[0] = U16(uv[0] * 0xFFFF);
				verts[i].m_uv[1] = U16(uv[1] * 0xFFFF);
			}
			else if(uvfmt == Format::R16G16_SFLOAT)
			{
				verts[i].m_uv[0] = F16(uv[0]).toU16();
				verts[i].m_uv[1] = F16(uv[1]).toU16();
			}
			else
			{
				ANKI_ASSERT(0);
			}
		}

		ANKI_CHECK(file.write(&verts[0], verts.getSizeInBytes()));
	}

	// Write 3rd vert buffer
	if(hasBoneWeights)
	{
		for(const SubMesh& submesh : submeshes)
		{
			DynamicArrayAuto<WeightVertex> verts(m_alloc);
			verts.create(submesh.m_verts.getSize());

			for(U32 i = 0; i < verts.getSize(); ++i)
			{
				WeightVertex vert;

				for(U32 c = 0; c < 4; ++c)
				{
					vert.m_boneIndices[c] = submesh.m_verts[i].m_boneIds[c];
					vert.m_weights[c] = U8(submesh.m_verts[i].m_boneWeights[c] * F32(MAX_U8));
				}

				verts[i] = vert;
			}

			ANKI_CHECK(file.write(&verts[0], verts.getSizeInBytes()));
		}
	}

	return Error::NONE;
}

} // end namespace anki
