// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "Exporter.h"

#if defined(__GNUC__)
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wmisleading-indentation"
#	pragma GCC diagnostic ignored "-Wshift-negative-value"
#endif
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tinygltf/tiny_gltf.h>
#if defined(__GNUC__)
#	pragma GCC diagnostic pop
#endif

namespace anki
{

Error Exporter::load()
{
	std::string err, warn;
	Bool ret = m_loader.LoadASCIIFromFile(&m_model, &err, &warn, m_inputFilename.cstr());

	if(!warn.empty())
	{
		ANKI_LOGW(warn.c_str());
	}

	if(!err.empty())
	{
		ANKI_LOGE(err.c_str());
	}

	if(!ret)
	{
		ANKI_LOGE("Failed to parse glTF");
		return Error::USER_DATA;
	}

	return Error::NONE;
}

void Exporter::getAttributeInfo(
	const tinygltf::Primitive& primitive, CString attribName, const U8*& buff, U32& stride, U32& count) const
{
	const tinygltf::Accessor& accessor = m_model.accessors[primitive.attributes.find(attribName.cstr())->second];
	count = accessor.count;
	const tinygltf::BufferView& view = m_model.bufferViews[accessor.bufferView];
	buff = reinterpret_cast<const U8*>(&(m_model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
	stride = view.byteStride;
}

Error Exporter::exportMesh(const tinygltf::Mesh& mesh)
{
	if(mesh.primitives.size() != 1)
	{
		ANKI_LOGE("Can't handle this");
		return Error::USER_DATA;
	}
	const tinygltf::Primitive& primitive = mesh.primitives[0];

	// Get indices
	DynamicArrayAuto<U16> indices(m_alloc);
	{
		const tinygltf::Accessor& accessor = m_model.accessors[primitive.indices];
		const tinygltf::BufferView& bufferView = m_model.bufferViews[accessor.bufferView];
		const tinygltf::Buffer& buffer = m_model.buffers[bufferView.buffer];

		indices.create(accessor.count);

		switch(accessor.componentType)
		{
		case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
			for(U i = 0; i < indices.getSize(); ++i)
			{
				indices[i] = *reinterpret_cast<const U32*>(buffer.data[i * sizeof(U32)]);
			}
			break;
		case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
			for(U i = 0; i < indices.getSize(); ++i)
			{
				indices[i] = *reinterpret_cast<const U16*>(buffer.data[i * sizeof(U16)]);
			}
			break;
		default:
			ANKI_ASSERT(!"TODO");
			break;
		}
	}

	// Get positions
	DynamicArrayAuto<Vec3> positions(m_alloc);
	Vec3 minPos, maxPos;
	U32 vertCount;
	{
		if(primitive.attributes.find("POSITION") != primitive.attributes.end())
		{
			ANKI_LOGE("Positions are missing for mesh %s", mesh.name.c_str());
			return Error::USER_DATA;
		}

		const U8* posBuff;
		U32 posStride;
		getAttributeInfo(primitive, "POSITION", posBuff, posStride, vertCount);

		if(posStride < sizeof(Vec3))
		{
			ANKI_LOGE("Position stride is wrong for mesh %s", mesh.name.c_str());
			return Error::USER_DATA;
		}

		positions.create(vertCount);
		for(U v = 0; v < vertCount; ++v)
		{
			positions[v] = Vec3(reinterpret_cast<const F32*>(&posBuff[v * posStride]));

			// TODO minPos, maxPos
		}
	}

	// Get normals and UVs
	DynamicArrayAuto<Vec3> normals(m_alloc);
	DynamicArrayAuto<Vec2> uvs(m_alloc);

	{
		if(primitive.attributes.find("NORMAL") != primitive.attributes.end())
		{
			ANKI_LOGE("Normals are missing for mesh %s", mesh.name.c_str());
			return Error::USER_DATA;
		}
		if(primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
		{
			ANKI_LOGE("Texcoords are missing for mesh %s", mesh.name.c_str());
			return Error::USER_DATA;
		}

		const U8* normalBuff;
		U32 normalStride;
		U32 count;
		getAttributeInfo(primitive, "NORMAL", normalBuff, normalStride, count);
		ANKI_ASSERT(count == vertCount);

		const U8* uvBuff;
		U32 uvStride;
		getAttributeInfo(primitive, "TEXCOORD_0", uvBuff, uvStride, count);
		ANKI_ASSERT(count == vertCount);

		normals.create(vertCount);
		uvs.create(vertCount);

		for(U v = 0; v < vertCount; ++v)
		{
			normals[v] = Vec3(reinterpret_cast<const F32*>(&normalBuff[v * normalStride]));
			uvs[v] = Vec2(reinterpret_cast<const F32*>(&uvBuff[v * uvStride]));
		}
	}

	return Error::NONE;
}

Error Exporter::exportAll()
{
	for(const tinygltf::Mesh& mesh : m_model.meshes)
	{
		ANKI_CHECK(exportMesh(mesh));
	}

	return Error::NONE;
}

} // end namespace anki