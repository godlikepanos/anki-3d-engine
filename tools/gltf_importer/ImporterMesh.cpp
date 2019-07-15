// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "Importer.h"

namespace anki
{

template<typename T>
static Error appendAttribute(const cgltf_attribute& attrib, DynamicArrayAuto<T>& out)
{
	ANKI_ASSERT(attrib.data);
	if(attrib.data->type == cgltf_type_vec3 && attrib.data->component_type == cgltf_component_type_r_32f)
	{
		const U8* base = static_cast<const U8*>(attrib.data->buffer_view->buffer->data) + attrib.data->offset
						 + attrib.data->buffer_view->offset;

		const PtrSize stride = (attrib.data->stride == 0) ? sizeof(Vec3) : attrib.data->stride == 0;

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
	}
	else
	{
		ANKI_ASSERT(!"TODO");
	}

	return Error::NONE;
}

Error Importer::writeMesh(const cgltf_mesh& mesh)
{
	StringAuto fname(m_alloc);
	fname.sprintf("%s%s.ankimesh", m_outDir.cstr(), mesh.name);
	ANKI_GLTF_LOGI("Exporting %s", fname.cstr());

	DynamicArrayAuto<Vec3> positions(m_alloc);
	DynamicArrayAuto<Vec3> normals(m_alloc);
	DynamicArrayAuto<Vec4> tangents(m_alloc);
	DynamicArrayAuto<Vec2> uvs(m_alloc);

	// Iterate primitives
	for(const cgltf_primitive* primitive = mesh.primitives; primitive < mesh.primitives + mesh.primitives_count;
		++primitive)
	{
		if(primitive->type != cgltf_primitive_type_triangles)
		{
			ANKI_GLTF_LOGE("Expecting triangles got %d", primitive->type);
			return Error::USER_DATA;
		}

		ANKI_ASSERT(primitive->indices);
		if(primitive->indices->count == 0 || (primitive->indices->count % 3) != 0)
		{
			ANKI_GLTF_LOGE("Incorect index count: %d", primitive->indices->count);
			return Error::USER_DATA;
		}

		for(const cgltf_attribute* attrib = primitive->attributes;
			attrib < primitive->attributes + primitive->attributes_count;
			++attrib)
		{
			if(attrib->type == cgltf_attribute_type_position)
			{
				appendAttribute(*attrib, positions);
			}
		}
	}

	return Error::NONE;
}

} // end namespace anki