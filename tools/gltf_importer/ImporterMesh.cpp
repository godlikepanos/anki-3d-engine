// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "Importer.h"

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
	default:
		ANKI_ASSERT(!"TODO");
		out = 0;
	}

	return out;
}

static U calcImplicitStride(const cgltf_attribute& attrib)
{
	return cgltfComponentCount(attrib.data->type) * cgltfComponentSize(attrib.data->component_type);
}

template<typename T, typename TFunc>
static Error appendAttribute(const cgltf_attribute& attrib, DynamicArrayAuto<T>& out, TFunc func)
{
	if(cgltfComponentCount(attrib.data->type) != T::COMPONENT_COUNT)
	{
		ANKI_GLTF_LOGE("Wrong component count for attribute: %s", attrib.name);
		return Error::USER_DATA;
	}

	ANKI_ASSERT(attrib.data);

	const U8* base = static_cast<const U8*>(attrib.data->buffer_view->buffer->data) + attrib.data->offset
					 + attrib.data->buffer_view->offset;

	const PtrSize stride = (attrib.data->stride == 0) ? calcImplicitStride(attrib) : attrib.data->stride == 0;

	const U count = attrib.data->count;

	if(count == 0)
	{
		ANKI_GLTF_LOGE("Zero vertex count");
		return Error::USER_DATA;
	}

	for(U i = 0; i < count; ++i)
	{
		const U8* ptr = base + stride * i;
		T val;
		memcpy(&val, ptr, sizeof(T)); // Memcpy because it might not be aligned
		func(val);
		out.emplaceBack(val);
	}

	return Error::NONE;
}

class SubMesh
{
public:
	DynamicArrayAuto<Vec3> m_positions;
	DynamicArrayAuto<Vec3> m_normals;
	DynamicArrayAuto<Vec4> m_tangents;
	DynamicArrayAuto<Vec2> m_uvs;
	DynamicArrayAuto<U16> m_indices;

	Vec3 m_aabbMin{MAX_F32};
	Vec3 m_aabbMax{MIN_F32};

	U32 m_firstIdx = MAX_U32;
	U32 m_idxCount = MAX_U32;

	SubMesh(HeapAllocator<U8>& alloc)
		: m_positions(alloc)
		, m_normals(alloc)
		, m_tangents(alloc)
		, m_uvs(alloc)
		, m_indices(alloc)
	{
	}
};

Error Importer::writeMesh(const cgltf_mesh& mesh)
{
	StringAuto fname(m_alloc);
	fname.sprintf("%s%s.ankimesh", m_outDir.cstr(), mesh.name);
	ANKI_GLTF_LOGI("Exporting %s", fname.cstr());

	DynamicArrayAuto<SubMesh> submeshes(m_alloc);
	U32 totalVertCount = 0;
	U32 totalIndexCount = 0;
	Vec3 aabbMin(MAX_F32);
	Vec3 aabbMax(MIN_F32);
	F32 maxUvDistance = MIN_F32;
	F32 minUvDistance = MAX_F32;

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

		//
		// Gather positions + normals + UVs
		//
		for(const cgltf_attribute* attrib = primitive->attributes;
			attrib < primitive->attributes + primitive->attributes_count;
			++attrib)
		{
			if(attrib->type == cgltf_attribute_type_position)
			{
				appendAttribute(*attrib, submesh.m_positions, [&](const Vec3& pos) {
					submesh.m_aabbMin = submesh.m_aabbMin.min(pos);
					submesh.m_aabbMax = submesh.m_aabbMax.min(pos);
				});
			}
			else if(attrib->type == cgltf_attribute_type_normal)
			{
				appendAttribute(*attrib, submesh.m_normals, [](const Vec3&) {});
			}
			else if(attrib->type == cgltf_attribute_type_texcoord)
			{
				appendAttribute(*attrib, submesh.m_uvs, [&](const Vec2& uv) {
					maxUvDistance = max(maxUvDistance, max(uv.x(), uv.y()));
					minUvDistance = min(minUvDistance, min(uv.x(), uv.y()));
				});
			}
			else
			{
				ANKI_GLTF_LOGW("Ignoring attribute: %s", attrib->name);
			}
		}

		aabbMin = aabbMin.min(submesh.m_aabbMin);
		aabbMax = aabbMax.min(submesh.m_aabbMax);

		const U32 vertCount = U32(submesh.m_positions.getSize());
		if(submesh.m_positions.getSize() == 0)
		{
			ANKI_GLTF_LOGE("Zero size of positions");
			return Error::USER_DATA;
		}

		if(submesh.m_normals.getSize() != vertCount)
		{
			ANKI_GLTF_LOGE("Normal count is incorrect. Is %u, should be %u", submesh.m_normals.getSize(), vertCount);
			return Error::USER_DATA;
		}

		if(submesh.m_uvs.getSize() != vertCount)
		{
			ANKI_GLTF_LOGE("UV count is incorrect. Is %u, should be %u", submesh.m_uvs.getSize(), vertCount);
			return Error::USER_DATA;
		}

		//
		// Fix normals. If normal A and normal B have the same position then merge them
		//
		for(U v = 0; v < vertCount; ++v)
		{
			const Vec3& pos = submesh.m_positions[v];
			Vec3& normal = submesh.m_normals[v];

			for(U prevV = 0; prevV < v; ++prevV)
			{
				const Vec3& otherPos = submesh.m_positions[prevV];

				// Check the positions dist
				const F32 posDist = (otherPos - pos).getLengthSquared();
				if(posDist > EPSILON * EPSILON)
				{
					continue;
				}

				// Check angle of the normals
				Vec3& otherNormal = submesh.m_normals[prevV];
				const F32 cosAng = otherNormal.dot(normal);
				if(cosAng > m_normalsMergeCosAngle)
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
				ANKI_GLTF_LOGE("Incorect index count: %d", primitive->indices->count);
				return Error::USER_DATA;
			}
			submesh.m_indices.create(primitive->indices->count);
			const U8* base = static_cast<const U8*>(primitive->indices->buffer_view->buffer->data)
							 + primitive->indices->offset + primitive->indices->buffer_view->offset;
			for(U i = 0; i < primitive->indices->count; ++i)
			{
				U idx;
				if(primitive->indices->component_type == cgltf_component_type_r_32u)
				{
					idx = totalVertCount + *reinterpret_cast<const U32*>(base + sizeof(U32) * i);
				}
				else if(primitive->indices->component_type == cgltf_component_type_r_16u)
				{
					idx = totalVertCount + *reinterpret_cast<const U16*>(base + sizeof(U16) * i);
				}
				else
				{
					ANKI_ASSERT(0);
				}

				ANKI_ASSERT(idx < MAX_U16);
				submesh.m_indices[i] = U16(idx);
			}

			submesh.m_firstIdx = totalIndexCount;
			submesh.m_idxCount = U32(primitive->indices->count);
			totalIndexCount += U32(primitive->indices->count);
		}

		//
		// Compute tangent
		//
		{
			DynamicArrayAuto<Vec3> bitangents(m_alloc);

			submesh.m_tangents.create(vertCount, Vec4(0.0f));
			bitangents.create(vertCount, Vec3(0.0f));

			for(U i = 0; i < submesh.m_indices.getSize(); i += 3)
			{
				const U i0 = submesh.m_indices[i + 0];
				const U i1 = submesh.m_indices[i + 1];
				const U i2 = submesh.m_indices[i + 2];

				const Vec3& v0 = submesh.m_positions[i0];
				const Vec3& v1 = submesh.m_positions[i1];
				const Vec3& v2 = submesh.m_positions[i2];
				const Vec3 edge01 = v1 - v0;
				const Vec3 edge02 = v2 - v0;

				const Vec2 uvedge01 = submesh.m_uvs[i1] - submesh.m_uvs[i0];
				const Vec2 uvedge02 = submesh.m_uvs[i2] - submesh.m_uvs[i0];

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

				submesh.m_tangents[i0] += Vec4(t, 0.0f);
				submesh.m_tangents[i1] += Vec4(t, 0.0f);
				submesh.m_tangents[i2] += Vec4(t, 0.0f);

				bitangents[i0] += b;
				bitangents[i1] += b;
				bitangents[i2] += b;
			}

			for(U i = 0; i < vertCount; ++i)
			{
				Vec3 t = Vec3(submesh.m_tangents[i].xyz());
				const Vec3& n = submesh.m_normals[i];
				Vec3& b = bitangents[i];

				t.normalize();
				b.normalize();

				const F32 w = ((n.cross(t)).dot(b) < 0.0f) ? 1.0f : -1.0f;
				submesh.m_tangents[i] = Vec4(t, w);
			}
		}

		// Finalize
		totalVertCount += vertCount;
	}

	// Find if it's a convex shape
	Bool convex = true;
	for(const SubMesh& submesh : submeshes)
	{
		for(U i = 0; i < submesh.m_indices.getSize(); i += 3)
		{
			const U i0 = submesh.m_indices[i + 0];
			const U i1 = submesh.m_indices[i + 1];
			const U i2 = submesh.m_indices[i + 2];

			const Vec3& v0 = submesh.m_positions[i0];
			const Vec3& v1 = submesh.m_positions[i1];
			const Vec3& v2 = submesh.m_positions[i2];

			if(computeTriangleArea(v0, v1, v2) <= EPSILON)
			{
				continue;
			}

			// Check that all positions are behind the plane
			const Plane plane(v0.xyz0(), v1.xyz0(), v2.xyz0());

			for(const SubMesh& submeshB : submeshes)
			{
				for(const Vec3& posB : submeshB.m_positions)
				{
					const F32 test = testPlane(plane, posB.xyz0());
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
		uva.m_scale = 1.0;
	}

	// Arange the attributes into vert buffers
	{
		header.m_vertexBufferCount = 2;

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

		// 2nd buff has normal + tangent + texcoords
		header.m_vertexBuffers[1].m_vertexStride = sizeof(U32) * 2 + sizeof(U16) * 2;
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
		header.m_totalVertexCount = totalVertCount;
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
		ANKI_CHECK(file.write(&submesh.m_indices[0], submesh.m_indices.getSizeInBytes()));
	}

	// Write first vert buffer
	for(const SubMesh& submesh : submeshes)
	{
		const MeshBinaryFile::VertexAttribute& posa = header.m_vertexAttributes[VertexAttributeLocation::POSITION];
		if(posa.m_format == Format::R32G32B32_SFLOAT)
		{
			ANKI_CHECK(file.write(&submesh.m_positions[0], submesh.m_positions.getSizeInBytes()));
		}
		else if(posa.m_format == Format::R16G16B16A16_SFLOAT)
		{
			DynamicArrayAuto<F16> pos16(m_alloc);
			pos16.create(submesh.m_positions.getSize() * 4);

			const Vec3* p32 = &submesh.m_positions[0];
			const Vec3* p32end = p32 + submesh.m_positions.getSize();
			F16* p16 = &pos16[0];
			while(p32 != p32end)
			{
				p16[0] = F16(p32->x());
				p16[1] = F16(p32->y());
				p16[2] = F16(p32->z());
				p16[3] = F16(0.0f);

				p32 += 1;
				p16 += 4;
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
		verts.create(submesh.m_positions.getSize());

		for(U i = 0; i < verts.getSize(); ++i)
		{
			const Vec3& normal = submesh.m_normals[i];
			const Vec4& tangent = submesh.m_tangents[i];
			const Vec2& uv = submesh.m_uvs[i];

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

	return Error::NONE;
}

} // end namespace anki
