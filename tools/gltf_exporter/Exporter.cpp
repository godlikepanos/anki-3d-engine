// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "Exporter.h"
#include <anki/resource/MeshLoader.h>
#include <anki/util/File.h>
#include <string>

namespace anki
{

class WeightVertex
{
public:
	U16 m_boneIndices[4] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
	U8 m_weights[4] = {0, 0, 0, 0};
};

static std::string replaceAllString(const std::string& str, const std::string& from, const std::string& to)
{
	if(from.empty())
	{
		return str;
	}

	std::string out = str;
	size_t start_pos = 0;
	while((start_pos = out.find(from, start_pos)) != std::string::npos)
	{
		out.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}

	return out;
}

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
	Vec3 aabbMin(MAX_F32), aabbMax(MIN_F32);
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

			aabbMin = aabbMin.min(positions[v]);
			aabbMax = aabbMax.max(positions[v]);
		}

		aabbMax += EPSILON * 10.0f; // Bump it a bit
	}

	// Get normals and UVs
	DynamicArrayAuto<Vec3> normals(m_alloc);
	DynamicArrayAuto<Vec2> uvs(m_alloc);
	F32 maxUvDistance = MIN_F32;
	F32 minUvDistance = MAX_F32;
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

			maxUvDistance = max(maxUvDistance, max(uvs[v].x(), uvs[v].y()));
			minUvDistance = min(minUvDistance, min(uvs[v].x(), uvs[v].y()));
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
	DynamicArrayAuto<WeightVertex> weights(m_alloc);

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

		weights.create(vertCount);

		for(U v = 0; v < vertCount; ++v)
		{
			WeightVertex w;

			const U16* inIdxs = reinterpret_cast<const U16*>(&bonesBuff[v * bonesStride]);
			w.m_boneIndices[0] = inIdxs[0];
			w.m_boneIndices[1] = inIdxs[1];
			w.m_boneIndices[2] = inIdxs[2];
			w.m_boneIndices[3] = inIdxs[3];

			const F32* inW = reinterpret_cast<const F32*>(&weights[v * weightsStride]);
			w.m_weights[0] = inW[0] * 0xFF;
			w.m_weights[1] = inW[1] * 0xFF;
			w.m_weights[2] = inW[2] * 0xFF;
			w.m_weights[3] = inW[3] * 0xFF;
		}
	}

	// Find if it's a convex shape
	Bool convex = true;
	for(U i = 0; i < indices.getSize(); i += 3)
	{
		const U i0 = indices[i + 0];
		const U i1 = indices[i + 1];
		const U i2 = indices[i + 2];

		const Vec3& v0 = positions[i0];
		const Vec3& v1 = positions[i1];
		const Vec3& v2 = positions[i2];

		// Check that all positions are behind the plane
		Plane plane(v0.xyz0(), v1.xyz0(), v2.xyz0());

		for(U j = 0; j < positions.getSize(); ++j)
		{
			const Vec3& pos = positions[j];

			F32 test = plane.test(pos.xyz0());
			if(test > EPSILON)
			{
				convex = false;
				break;
			}
		}

		if(convex == false)
		{
			break;
		}
	}

	// Chose the formats of the attributes
	MeshBinaryFile::Header header = {};
	{
		// Positions
		Vec3 dist3d = aabbMin.getAbs().max(aabbMax.getAbs());
		const F32 maxPositionDistance = max(max(dist3d.x(), dist3d.y()), dist3d.z());
		auto& posa = header.m_vertexAttributes[VertexAttributeLocation::POSITION];
		posa.m_bufferBinding = 0;
		posa.m_format = (maxPositionDistance < 2.0) ? Format::R16G16B16A16_SFLOAT : Format::R32G32B32_SFLOAT;
		posa.m_relativeOffset = 0;
		posa.m_scale = 1.0;

		// Normals
		auto& na = header.m_vertexAttributes[VertexAttributeLocation::NORMAL];
		na.m_bufferBinding = 1;
		na.m_format = Format::A2B10G10R10_SNORM_PACK32;
		na.m_relativeOffset = 0;
		na.m_scale = 1.0;

		// Tangents
		auto& ta = header.m_vertexAttributes[VertexAttributeLocation::TANGENT];
		ta.m_bufferBinding = 1;
		ta.m_format = Format::A2B10G10R10_SNORM_PACK32;
		ta.m_relativeOffset = sizeof(U32);
		ta.m_scale = 1.0;

		// UVs
		auto& uva = header.m_vertexAttributes[VertexAttributeLocation::UV];
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

		// Bone weight
		if(weights.getSize())
		{
			auto& bidxa = header.m_vertexAttributes[VertexAttributeLocation::BONE_INDICES];
			bidxa.m_bufferBinding = 2;
			bidxa.m_format = Format::R16G16B16A16_UINT;
			bidxa.m_relativeOffset = 0;
			bidxa.m_scale = 1.0;

			auto& wa = header.m_vertexAttributes[VertexAttributeLocation::BONE_WEIGHTS];
			wa.m_bufferBinding = 2;
			wa.m_format = Format::R8G8B8A8_UNORM;
			wa.m_relativeOffset = sizeof(U16) * 4;
			wa.m_scale = 1.0;
		}
	}

	// Arange the attributes into vert buffers
	{
		header.m_vertexBufferCount = 2;

		// First buff has positions
		const auto& posa = header.m_vertexAttributes[VertexAttributeLocation::POSITION];
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

		// 3rd has bone weights
		if(weights.getSize())
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
		header.m_totalIndexCount = indices.getSize();
		header.m_totalVertexCount = vertCount;
		header.m_subMeshCount = 1;
		header.m_aabbMin = aabbMin;
		header.m_aabbMax = aabbMax;
	}

	// Open file
	File file;
	ANKI_CHECK(file.open(
		StringAuto(m_alloc).sprintf("%s/%s.ankimesh", m_outputDirectory.cstr(), mesh.name.c_str()).toCString(),
		FileOpenFlag::WRITE | FileOpenFlag::BINARY));

	// Write header
	ANKI_CHECK(file.write(&header, sizeof(header)));

	// Write sub meshes
	{
		MeshBinaryFile::SubMesh smesh;
		smesh.m_firstIndex = 0;
		smesh.m_indexCount = header.m_totalIndexCount;
		smesh.m_aabbMin = aabbMin;
		smesh.m_aabbMax = aabbMax;

		ANKI_CHECK(file.write(&smesh, sizeof(smesh)));
	}

	// Write indices
	ANKI_CHECK(file.write(&indices[0], indices.getSizeInBytes()));

	// Write first vert buffer
	{
		const auto& posa = header.m_vertexAttributes[VertexAttributeLocation::POSITION];
		if(posa.m_format == Format::R32G32B32_SFLOAT)
		{
			ANKI_CHECK(file.write(&positions[0], positions.getSizeInBytes()));
		}
		else if(posa.m_format == Format::R16G16B16A16_SFLOAT)
		{
			DynamicArrayAuto<F16> pos16(m_alloc);
			pos16.create(vertCount * 4);

			const Vec3* p32 = &positions[0];
			const Vec3* p32end = p32 + positions.getSize();
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

	// Write 2nd vert buffer
	{
		struct Vert
		{
			U32 m_n;
			U32 m_t;
			U16 m_uv[2];
		};

		DynamicArrayAuto<Vert> verts(m_alloc);
		verts.create(vertCount);

		for(U i = 0; i < vertCount; ++i)
		{
			const Vec3& normal = normals[i];
			const Vec4& tangent = tangents[i];
			const Vec2& uv = uvs[i];

			verts[i].m_n = packColorToR10G10B10A2SNorm(normal.x(), normal.y(), normal.z(), 0.0f);
			verts[i].m_t = packColorToR10G10B10A2SNorm(tangent.x(), tangent.y(), tangent.z(), tangent.w());

			const Format uvfmt = header.m_vertexAttributes[VertexAttributeLocation::UV].m_format;
			if(uvfmt == Format::R16G16_UNORM)
			{
				assert(uv[0] <= 1.0 && uv[0] >= 0.0 && uv[1] <= 1.0 && uv[1] >= 0.0);
				verts[i].m_uv[0] = uv[0] * 0xFFFF;
				verts[i].m_uv[1] = uv[1] * 0xFFFF;
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
	if(weights.getSize())
	{
		ANKI_CHECK(file.write(&weights[0], weights.getSizeInBytes()));
	}

	return Error::NONE;
}

Bool Exporter::getTexture(const tinygltf::Material& mtl, CString texName, StringAuto& fname) const
{
	fname.destroy();

	if(mtl.values.find(texName.cstr()) != mtl.values.end())
	{
		const I textureIdx = mtl.values.find(texName.cstr())->second.TextureIndex();
		ANKI_ASSERT(textureIdx > -1);
		const I imageIdx = m_model.textures[textureIdx].source;
		fname.sprintf("%s/%s", m_texrpath.cstr(), m_model.images[imageIdx].uri.c_str());

		return true;
	}
	else
	{
		return false;
	}
}

Bool Exporter::getMaterialAttrib(const tinygltf::Material& mtl, CString attribName, Vec4& value) const
{
	auto it = mtl.values.find(attribName.cstr());
	if(it != mtl.values.end())
	{
		const tinygltf::Parameter& param = it->second;
		if(param.has_number_value)
		{
			value = Vec4(param.number_value);
		}
		else
		{
			value =
				Vec4(param.ColorFactor()[0], param.ColorFactor()[1], param.ColorFactor()[2], param.ColorFactor()[3]);
		}

		return true;
	}
	else
	{
		return false;
	}
}

Error Exporter::exportMaterial(const tinygltf::Material& mtl)
{
	const char* MATERIAL_TEMPLATE = R"(<?xml version="1.0" encoding="UTF-8" ?>
<!-- This file is auto generated -->
<material shaderProgram="shaders/GBufferGeneric.glslp">
	<mutators>
		<mutator name="DIFFUSE_TEX" value="%diffTexMutator%"/>
		<mutator name="SPECULAR_TEX" value="%specTexMutator%"/>
		<mutator name="ROUGHNESS_TEX" value="%roughnessTexMutator%"/>
		<mutator name="METAL_TEX" value="%metalTexMutator%"/>
		<mutator name="NORMAL_TEX" value="%normalTexMutator%"/>
		<mutator name="PARALLAX" value="%parallaxMutator%"/>
		<mutator name="EMISSIVE_TEX" value="%emissiveTexMutator%"/>
	</mutators>
	<inputs>
		<input shaderInput="mvp" builtin="MODEL_VIEW_PROJECTION_MATRIX"/>
		<input shaderInput="prevMvp" builtin="PREVIOUS_MODEL_VIEW_PROJECTION_MATRIX"/>
		<input shaderInput="rotationMat" builtin="ROTATION_MATRIX"/>
		%parallaxInput%

		%diff%
		%spec%
		%roughness%
		%metallic%
		%normal%
		%emission%
		%subsurface%
		%height%
	</inputs>
</material>
)";

	std::string xml = MATERIAL_TEMPLATE;

	// Diffuse
	StringAuto fname(m_alloc);
	Vec4 v4;
	if(getTexture(mtl, "baseColorTexture", fname))
	{
		xml = replaceAllString(xml, "%diffTexMutator%", "1");
		xml = replaceAllString(
			xml, "%diff%", "<input shaderInput=\"diffTex\" value=\"" + std::string(fname.cstr()) + "\"/>");
	}
	else if(getMaterialAttrib(mtl, "baseColorFactor", v4))
	{
		xml = replaceAllString(xml, "%diffTexMutator%", "0");
		xml = replaceAllString(xml,
			"%diff%",
			"<input shaderInput=\"diffColor\" value=\"" + std::to_string(v4[0]) + " " + std::to_string(v4[1]) + " "
				+ std::to_string(v4[2]) + "\"/>");
	}

	// Normal
	if(getTexture(mtl, "normalTexture", fname))
	{
		xml = replaceAllString(
			xml, "%normal%", "<input shaderInput=\"normalTex\" value=\"" + std::string(fname.cstr()) + "\"/>");
		xml = replaceAllString(xml, "%normalTexMutator%", "1");
	}
	else
	{
		xml = replaceAllString(xml, "%normal%", "");
		xml = replaceAllString(xml, "%normalTexMutator%", "0");
	}

	// Roughness & metallic
	I metallic = -1;
	I roughness = -1;
	if(getTexture(mtl, "metallicRoughnessTexture", fname))
	{
		if(fname.find("_rm.") != String::NPOS)
		{
			roughness = 0;
			metallic = 1;
		}
		else if(fname.find("_r.") != String::NPOS)
		{
			roughness = 0;
		}
		else if(fname.find("_m.") != String::NPOS)
		{
			metallic = 0;
		}
		else
		{
			ANKI_LOGE("Can't identify the type of metallicRoughnessTexture by its filename");
			return Error::USER_DATA;
		}

		if(metallic > -1)
		{
			xml = replaceAllString(
				xml, "%metallic%", "<input shaderInput=\"metalTex\" value=\"" + std::string(fname.cstr()) + "\"/>");
			xml = replaceAllString(xml, "%metalTexMutator%", std::to_string(metallic + 1));
		}

		if(roughness > -1)
		{
			xml = replaceAllString(xml,
				"%roughness%",
				"<input shaderInput=\"roughnessTex\" value=\"" + std::string(fname.cstr()) + "\"/>");
			xml = replaceAllString(xml, "%roughnessTexMutator%", std::to_string(roughness + 1));
		}
	}

	if(metallic == -1)
	{
		EXPORT_ASSERT(getMaterialAttrib(mtl, "metallicFactor", v4));
		xml = replaceAllString(
			xml, "%metallic%", "<input shaderInput=\"metallic\" value=\"" + std::to_string(v4.x()) + "\"/>");

		xml = replaceAllString(xml, "%metalTexMutator%", "0");
	}

	if(roughness == -1)
	{
		EXPORT_ASSERT(getMaterialAttrib(mtl, "roughnessFactor", v4));

		xml = replaceAllString(
			xml, "%roughness%", "<input shaderInput=\"roughness\" value=\"" + std::to_string(v4.x()) + "\" />");

		xml = replaceAllString(xml, "%roughnessTexMutator%", "0");
	}

	// Replace texture extensions with .anki
	xml = replaceAllString(xml, ".tga", ".ankitex");
	xml = replaceAllString(xml, ".png", ".ankitex");
	xml = replaceAllString(xml, ".jpg", ".ankitex");
	xml = replaceAllString(xml, ".jpeg", ".ankitex");

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

	for(const tinygltf::Material& mtl : m_model.materials)
	{
		const Error err = exportMaterial(mtl);
		if(err)
		{
			ANKI_LOGE("Failed to load material %s", mtl.name.c_str());
			return err;
		}
	}

	return Error::NONE;
}

} // end namespace anki