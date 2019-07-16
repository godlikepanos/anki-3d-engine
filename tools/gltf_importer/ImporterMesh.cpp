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

template<typename T>
static Error appendAttribute(const cgltf_attribute& attrib, DynamicArrayAuto<T>& out)
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
	DynamicArrayAuto<U32> m_indices;

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
	U totalVertCount = 0;

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
				appendAttribute(*attrib, submesh.m_positions);
			}
			else if(attrib->type == cgltf_attribute_type_normal)
			{
				appendAttribute(*attrib, submesh.m_normals);
			}
			else if(attrib->type == cgltf_attribute_type_texcoord)
			{
				appendAttribute(*attrib, submesh.m_uvs);
			}
			else
			{
				ANKI_GLTF_LOGW("Ignoring attribute: %s", attrib->name);
			}
		}

		const U vertCount = submesh.m_positions.getSize();
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
				if(primitive->indices->component_type == cgltf_component_type_r_32u)
				{
					submesh.m_indices[i] = totalVertCount + *reinterpret_cast<const U32*>(base + sizeof(U32) * i);
				}
				else if(primitive->indices->component_type == cgltf_component_type_r_16u)
				{
					submesh.m_indices[i] = totalVertCount + *reinterpret_cast<const U16*>(base + sizeof(U16) * i);
				}
				else
				{
					ANKI_ASSERT(0);
				}
			}
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

	// Time to write the mesh
	// TODO

	return Error::NONE;
}

} // end namespace anki