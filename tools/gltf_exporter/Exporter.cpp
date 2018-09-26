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

void Exporter::getAttributeInfo(const tinygltf::Primitive& primitive,
	CString attribName,
	const U8*& buff,
	U32& stride,
	U32& count,
	Format& fmt) const
{
	const tinygltf::Accessor& accessor = m_model.accessors[primitive.attributes.find(attribName.cstr())->second];
	count = accessor.count;
	const tinygltf::BufferView& view = m_model.bufferViews[accessor.bufferView];
	buff = reinterpret_cast<const U8*>(&(m_model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
	stride = view.byteStride;
	ANKI_ASSERT(stride > 0);

	const Bool normalized = accessor.normalized;
	switch(accessor.componentType)
	{
	case TINYGLTF_COMPONENT_TYPE_BYTE:
		switch(accessor.type)
		{
		case TINYGLTF_TYPE_SCALAR:
			fmt = (normalized) ? Format::R8_SNORM : Format::R8_SINT;
			break;
		case TINYGLTF_TYPE_VEC2:
			fmt = (normalized) ? Format::R8G8_SNORM : Format::R8G8_SINT;
			break;
		case TINYGLTF_TYPE_VEC3:
			fmt = (normalized) ? Format::R8G8B8_SNORM : Format::R8G8B8_SINT;
			break;
		case TINYGLTF_TYPE_VEC4:
			fmt = (normalized) ? Format::R8G8B8A8_SNORM : Format::R8G8B8A8_SINT;
			break;
		default:
			ANKI_ASSERT(!"TODO");
		}
		break;
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
		switch(accessor.type)
		{
		case TINYGLTF_TYPE_SCALAR:
			fmt = (normalized) ? Format::R8_UNORM : Format::R8_UINT;
			break;
		case TINYGLTF_TYPE_VEC2:
			fmt = (normalized) ? Format::R8G8_UNORM : Format::R8G8_UINT;
			break;
		case TINYGLTF_TYPE_VEC3:
			fmt = (normalized) ? Format::R8G8B8_UNORM : Format::R8G8B8_UINT;
			break;
		case TINYGLTF_TYPE_VEC4:
			fmt = (normalized) ? Format::R8G8B8A8_UNORM : Format::R8G8B8A8_UINT;
			break;
		default:
			ANKI_ASSERT(!"TODO");
		}
		break;
	case TINYGLTF_COMPONENT_TYPE_SHORT:
		switch(accessor.type)
		{
		case TINYGLTF_TYPE_SCALAR:
			fmt = (normalized) ? Format::R16_SNORM : Format::R16_SINT;
			break;
		case TINYGLTF_TYPE_VEC2:
			fmt = (normalized) ? Format::R16G16_SNORM : Format::R16G16_SINT;
			break;
		case TINYGLTF_TYPE_VEC3:
			fmt = (normalized) ? Format::R16G16B16_SNORM : Format::R16G16B16_SINT;
			break;
		case TINYGLTF_TYPE_VEC4:
			fmt = (normalized) ? Format::R16G16B16A16_SNORM : Format::R16G16B16A16_SINT;
			break;
		default:
			ANKI_ASSERT(!"TODO");
		}
		break;
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
		switch(accessor.type)
		{
		case TINYGLTF_TYPE_SCALAR:
			fmt = (normalized) ? Format::R16_UNORM : Format::R16_UINT;
			break;
		case TINYGLTF_TYPE_VEC2:
			fmt = (normalized) ? Format::R16G16_UNORM : Format::R16G16_UINT;
			break;
		case TINYGLTF_TYPE_VEC3:
			fmt = (normalized) ? Format::R16G16B16_UNORM : Format::R16G16B16_UINT;
			break;
		case TINYGLTF_TYPE_VEC4:
			fmt = (normalized) ? Format::R16G16B16A16_UNORM : Format::R16G16B16A16_UINT;
			break;
		default:
			ANKI_ASSERT(!"TODO");
		}
		break;
	case TINYGLTF_COMPONENT_TYPE_INT:
		switch(accessor.type)
		{
		case TINYGLTF_TYPE_SCALAR:
			fmt = Format::R32_SINT;
			break;
		case TINYGLTF_TYPE_VEC2:
			fmt = Format::R32G32_SINT;
			break;
		case TINYGLTF_TYPE_VEC3:
			fmt = Format::R32G32B32_SINT;
			break;
		case TINYGLTF_TYPE_VEC4:
			fmt = Format::R32G32B32A32_SINT;
			break;
		default:
			ANKI_ASSERT(!"TODO");
		}
		break;
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
		switch(accessor.type)
		{
		case TINYGLTF_TYPE_SCALAR:
			fmt = Format::R32_UINT;
			break;
		case TINYGLTF_TYPE_VEC2:
			fmt = Format::R32G32_UINT;
			break;
		case TINYGLTF_TYPE_VEC3:
			fmt = Format::R32G32B32_UINT;
			break;
		case TINYGLTF_TYPE_VEC4:
			fmt = Format::R32G32B32A32_UINT;
			break;
		default:
			ANKI_ASSERT(!"TODO");
		}
		break;
	case TINYGLTF_COMPONENT_TYPE_FLOAT:
		switch(accessor.type)
		{
		case TINYGLTF_TYPE_SCALAR:
			fmt = Format::R32_SFLOAT;
			break;
		case TINYGLTF_TYPE_VEC2:
			fmt = Format::R32G32_SFLOAT;
			break;
		case TINYGLTF_TYPE_VEC3:
			fmt = Format::R32G32B32_SFLOAT;
			break;
		case TINYGLTF_TYPE_VEC4:
			fmt = Format::R32G32B32A32_SFLOAT;
			break;
		default:
			ANKI_ASSERT(!"TODO");
		}
		break;
	default:
		ANKI_ASSERT(!"TODO");
	}
}

Error Exporter::exportMesh(const tinygltf::Mesh& mesh)
{
	EXPORT_ASSERT(mesh.primitives.size() == 1);
	const tinygltf::Primitive& primitive = mesh.primitives[0];

	// Get indices
	DynamicArrayAuto<U16> indices(m_alloc);
	{
		const tinygltf::Accessor& accessor = m_model.accessors[primitive.indices];
		const tinygltf::BufferView& bufferView = m_model.bufferViews[accessor.bufferView];
		const tinygltf::Buffer& buffer = m_model.buffers[bufferView.buffer];

		EXPORT_ASSERT((accessor.count % 3) == 0);
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
			EXPORT_ASSERT(!"TODO");
			break;
		}
	}

	// Get positions
	DynamicArrayAuto<Vec3> positions(m_alloc);
	Vec3 minPos, maxPos;
	U32 vertCount;
	{
		EXPORT_ASSERT(primitive.attributes.find("POSITION") != primitive.attributes.end());

		const U8* posBuff;
		U32 posStride;
		Format fmt;
		getAttributeInfo(primitive, "POSITION", posBuff, posStride, vertCount, fmt);
		EXPORT_ASSERT(posStride == sizeof(Vec3) && fmt == Format::R32G32B32_SFLOAT);

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
		EXPORT_ASSERT(primitive.attributes.find("NORMAL") != primitive.attributes.end());
		EXPORT_ASSERT(primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end());

		const U8* normalBuff;
		U32 normalStride;
		U32 count;
		Format fmt;
		getAttributeInfo(primitive, "NORMAL", normalBuff, normalStride, count, fmt);
		EXPORT_ASSERT(count == vertCount && fmt == Format::R32G32B32_SFLOAT);

		const U8* uvBuff;
		U32 uvStride;
		getAttributeInfo(primitive, "TEXCOORD_0", uvBuff, uvStride, count, fmt);
		EXPORT_ASSERT(count == vertCount && fmt == Format::R32G32_SFLOAT);

		normals.create(vertCount);
		uvs.create(vertCount);

		for(U v = 0; v < vertCount; ++v)
		{
			normals[v] = Vec3(reinterpret_cast<const F32*>(&normalBuff[v * normalStride]));
			uvs[v] = Vec2(reinterpret_cast<const F32*>(&uvBuff[v * uvStride]));
		}
	}

	// Fix normals
	for(U v = 0; v < vertCount; ++v)
	{
		const Vec3& pos = positions[v];
		Vec3& normal = normals[v];

		for(U prevV = 0; prevV < v; ++prevV)
		{
			const Vec3& otherPos = positions[prevV];

			// Check the positions dist
			const F32 posDist = (otherPos - pos).getLength();
			if(posDist > EPSILON)
			{
				continue;
			}

			// Check angle of the normals
			Vec3& otherNormal = normals[prevV];
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

	// Compute tangents
	DynamicArrayAuto<Vec4> tangents(m_alloc);
	{
		DynamicArrayAuto<Vec3> bitangents(m_alloc);

		tangents.create(vertCount, Vec4(0.0f));
		bitangents.create(vertCount, Vec3(0.0f));

		for(U i = 0; i < indices.getSize(); i += 3)
		{
			const U i0 = indices[i + 0];
			const U i1 = indices[i + 1];
			const U i2 = indices[i + 2];

			const Vec3& v0 = positions[i0];
			const Vec3& v1 = positions[i1];
			const Vec3& v2 = positions[i2];
			const Vec3 edge01 = v1 - v0;
			const Vec3 edge02 = v2 - v0;

			const Vec2 uvedge01 = uvs[i1] - uvs[i0];
			const Vec2 uvedge02 = uvs[i2] - uvs[i0];

			F32 det = (uvedge01.y() * uvedge02.x()) - (uvedge01.x() * uvedge02.y());
			det = (isZero(det)) ? 0.0001f : (1.0f / det);

			Vec3 t = (edge02 * uvedge01.y() - edge01 * uvedge02.y()) * det;
			Vec3 b = (edge02 * uvedge01.x() - edge01 * uvedge02.x()) * det;
			t.normalize();
			b.normalize();

			tangents[i0] += Vec4(t, 0.0f);
			tangents[i1] += Vec4(t, 0.0f);
			tangents[i2] += Vec4(t, 0.0f);

			bitangents[i0] += b;
			bitangents[i1] += b;
			bitangents[i2] += b;
		}

		for(U i = 0; i < vertCount; ++i)
		{
			Vec3 t = Vec3(tangents[i].xyz());
			const Vec3& n = normals[i];
			Vec3& b = bitangents[i];

			t.normalize();
			b.normalize();

			const F32 w = ((n.cross(t)).dot(b) < 0.0f) ? 1.0f : -1.0f;
			tangents[i] = Vec4(t, w);
		}
	}

	// Load bone info
	DynamicArrayAuto<UVec4> boneIds(m_alloc);
	DynamicArrayAuto<Vec4> weights(m_alloc);

	if(primitive.attributes.find("JOINTS_0") != primitive.attributes.end()
		&& primitive.attributes.find("WEIGHTS_0") != primitive.attributes.end())
	{
		const U8* bonesBuff;
		U32 bonesStride;
		U32 count;
		Format fmt;
		getAttributeInfo(primitive, "JOINTS_0", bonesBuff, bonesStride, count, fmt);
		EXPORT_ASSERT(count == vertCount && fmt == Format::R16G16B16A16_UINT);

		const U8* weightsBuff;
		U32 weightsStride;
		getAttributeInfo(primitive, "WEIGHTS_0", weightsBuff, weightsStride, count, fmt);
		EXPORT_ASSERT(count == vertCount && fmt == Format::R32G32B32A32_SFLOAT);

		boneIds.create(vertCount);
		weights.create(vertCount);

		for(U v = 0; v < vertCount; ++v)
		{
			boneIds[v] = UVec4(*reinterpret_cast<const U16*>(&bonesBuff[v * bonesStride]),
				*reinterpret_cast<const U16*>(&bonesBuff[v * bonesStride + sizeof(U16)]),
				*reinterpret_cast<const U16*>(&bonesBuff[v * bonesStride + sizeof(U16) * 2]),
				*reinterpret_cast<const U16*>(&bonesBuff[v * bonesStride + sizeof(U16) * 3]));

			weights[v] = Vec4(reinterpret_cast<const F32*>(&weights[v * weightsStride]));
		}
	}

	return Error::NONE;
}

Error Exporter::exportAll()
{
	for(const tinygltf::Mesh& mesh : m_model.meshes)
	{
		const Error err = exportMesh(mesh);
		if(err)
		{
			ANKI_LOGE("Failed to load mesh %s", mesh.name.c_str());
			return err;
		}
	}

	return Error::NONE;
}

} // end namespace anki