// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "Exporter.h"
#include <anki/resource/MeshLoader.h>
#include <anki/Math.h>
#include <cmath>
#include <cfloat>

void Exporter::exportMesh(const aiMesh& mesh, const aiMatrix4x4* transform, unsigned vertCountPerFace) const
{
	std::string name = mesh.mName.C_Str();
	LOGI("Exporting mesh %s", name.c_str());

	const bool hasBoneWeights = mesh.mNumBones > 0;

	anki::MeshBinaryFile::Header header;
	memset(&header, 0, sizeof(header));

	// Checks
	if(mesh.mNumFaces == 0)
	{
		ERROR("Incorrect face number");
	}

	if(mesh.mVertices == 0)
	{
		ERROR("Incorrect vertex count number");
	}

	if(!mesh.HasPositions())
	{
		ERROR("Missing positions");
	}

	if(!mesh.HasNormals())
	{
		ERROR("Missing normals");
	}

	if(!mesh.HasTangentsAndBitangents())
	{
		ERROR("Missing tangents");
	}

	if(!mesh.HasTextureCoords(0))
	{
		ERROR("Missing UVs");
	}

	//
	// Gather the attributes
	//
	struct WeightVertex
	{
		uint16_t m_boneIndices[4] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
		uint8_t m_weights[4] = {0, 0, 0, 0};
	};
	std::vector<WeightVertex> bweights;

	std::vector<float> positions;

	struct NTVertex
	{
		float m_n[3];
		float m_t[4];
		float m_uv[2];
	};

	std::vector<NTVertex> ntVerts;

	float maxPositionDistance = 0.0; // Distance of positions from zero
	float maxUvDistance = -FLT_MAX, minUvDistance = FLT_MAX;
	anki::Vec3 aabbMin(anki::MAX_F32), aabbMax(anki::MIN_F32);

	{
		const aiMatrix3x3 normalMat = (transform) ? aiMatrix3x3(*transform) : aiMatrix3x3();

		const unsigned vertCount = mesh.mNumVertices;

		positions.resize(vertCount * 3);
		ntVerts.resize(vertCount);

		for(unsigned i = 0; i < vertCount; i++)
		{
			aiVector3D pos = mesh.mVertices[i];
			aiVector3D n = mesh.mNormals[i];
			aiVector3D t = mesh.mTangents[i];
			aiVector3D b = mesh.mBitangents[i];
			const aiVector3D& uv = mesh.mTextureCoords[0][i];

			if(transform)
			{
				pos = (*transform) * pos;
				n = normalMat * n;
				t = normalMat * t;
				b = normalMat * b;
			}

			if(m_flipyz)
			{
				static const aiMatrix4x4 toLefthanded(1, 0, 0, 0, 0, 0, 1, 0, 0, -1, 0, 0, 0, 0, 0, 1);

				pos = toLefthanded * pos;
				n = toLefthanded * n;
				t = toLefthanded * t;
				b = toLefthanded * b;
			}

			positions[i * 3 + 0] = pos.x;
			positions[i * 3 + 1] = pos.y;
			positions[i * 3 + 2] = pos.z;
			for(int d = 0; d < 3; ++d)
			{
				maxPositionDistance = std::max<float>(maxPositionDistance, fabs(pos[d]));
				aabbMin[d] = std::min(aabbMin[d], pos[d]);
				aabbMax[d] = std::max(aabbMax[d], pos[d]);
			}

			ntVerts[i].m_n[0] = n.x;
			ntVerts[i].m_n[1] = n.y;
			ntVerts[i].m_n[2] = n.z;

			ntVerts[i].m_t[0] = t.x;
			ntVerts[i].m_t[1] = t.y;
			ntVerts[i].m_t[2] = t.z;
			ntVerts[i].m_t[3] = ((n ^ t) * b < 0.0) ? 1.0 : -1.0;

			ntVerts[i].m_uv[0] = uv.x;
			ntVerts[i].m_uv[1] = uv.y;
			maxUvDistance = std::max(maxUvDistance, std::max(uv.x, uv.y));
			minUvDistance = std::min(minUvDistance, std::min(uv.x, uv.y));
		}

		if(hasBoneWeights)
		{
			bweights.resize(vertCount);

			for(unsigned i = 0; i < mesh.mNumBones; ++i)
			{
				const aiBone& bone = *mesh.mBones[i];
				for(unsigned j = 0; j < bone.mNumWeights; ++j)
				{
					const aiVertexWeight& aiWeight = bone.mWeights[j];
					assert(aiWeight.mVertexId < bweights.size());

					WeightVertex& vert = bweights[aiWeight.mVertexId];

					unsigned idx;
					if(vert.m_boneIndices[0] == 0xFFFF)
					{
						idx = 0;
					}
					else if(vert.m_boneIndices[1] == 0xFFFF)
					{
						idx = 1;
					}
					else if(vert.m_boneIndices[2] == 0xFFFF)
					{
						idx = 2;
					}
					else if(vert.m_boneIndices[3] == 0xFFFF)
					{
						idx = 3;
					}
					else
					{
						ERROR("Vertex has more than 4 bone weights");
					}

					vert.m_boneIndices[idx] = i;
					vert.m_weights[idx] = aiWeight.mWeight * 0xFF;
				}
			}
		}

		// Bump aabbMax a bit
		aabbMax += anki::EPSILON * 10.0f;
	}

	// Chose the formats of the attributes
	{
		// Positions
		auto& posa = header.m_vertexAttributes[anki::VertexAttributeLocation::POSITION];
		posa.m_bufferBinding = 0;
		posa.m_format = (maxPositionDistance < 2.0) ? anki::Format::R16G16B16_SFLOAT : anki::Format::R32G32B32_SFLOAT;
		posa.m_relativeOffset = 0;
		posa.m_scale = 1.0;

		// Normals
		auto& na = header.m_vertexAttributes[anki::VertexAttributeLocation::NORMAL];
		na.m_bufferBinding = 1;
		na.m_format = anki::Format::A2B10G10R10_SNORM_PACK32;
		na.m_relativeOffset = 0;
		na.m_scale = 1.0;

		// Tangents
		auto& ta = header.m_vertexAttributes[anki::VertexAttributeLocation::TANGENT];
		ta.m_bufferBinding = 1;
		ta.m_format = anki::Format::A2B10G10R10_SNORM_PACK32;
		ta.m_relativeOffset = sizeof(uint32_t);
		ta.m_scale = 1.0;

		// UVs
		auto& uva = header.m_vertexAttributes[anki::VertexAttributeLocation::UV];
		uva.m_bufferBinding = 1;
		if(minUvDistance >= 0.0 && maxUvDistance <= 1.0)
		{
			uva.m_format = anki::Format::R16G16_UNORM;
		}
		else
		{
			uva.m_format = anki::Format::R16G16_SFLOAT;
		}
		uva.m_relativeOffset = sizeof(uint32_t) * 2;
		uva.m_scale = 1.0;

		// Bone weight
		if(hasBoneWeights)
		{
			auto& bidxa = header.m_vertexAttributes[anki::VertexAttributeLocation::BONE_INDICES];
			bidxa.m_bufferBinding = 2;
			bidxa.m_format = anki::Format::R16G16B16A16_UINT;
			bidxa.m_relativeOffset = 0;
			bidxa.m_scale = 1.0;

			auto& wa = header.m_vertexAttributes[anki::VertexAttributeLocation::BONE_WEIGHTS];
			wa.m_bufferBinding = 2;
			wa.m_format = anki::Format::R8G8B8A8_UNORM;
			wa.m_relativeOffset = sizeof(uint16_t) * 4;
			wa.m_scale = 1.0;
		}
	}

	// Arange the attributes into vert buffers
	{
		header.m_vertexBufferCount = 2;

		// First buff has positions
		const auto& posa = header.m_vertexAttributes[anki::VertexAttributeLocation::POSITION];
		if(posa.m_format == anki::Format::R32G32B32_SFLOAT)
		{
			header.m_vertexBuffers[0].m_vertexStride = sizeof(float) * 3;
		}
		else
		{
			header.m_vertexBuffers[0].m_vertexStride = sizeof(uint16_t) * 3;
		}

		// 2nd buff has normal + tangent + texcoords
		header.m_vertexBuffers[1].m_vertexStride = sizeof(uint32_t) * 2 + sizeof(uint16_t) * 2;

		// 3rd has bone weights
		if(hasBoneWeights)
		{
			header.m_vertexBuffers[2].m_vertexStride = sizeof(WeightVertex);
			++header.m_vertexBufferCount;
		}
	}

	// Write some other header stuff
	{
		memcpy(&header.m_magic[0], anki::MeshBinaryFile::MAGIC, 8);
		header.m_flags = (vertCountPerFace == 4) ? anki::MeshBinaryFile::Flag::QUAD : anki::MeshBinaryFile::Flag::NONE;
		header.m_indexType = anki::IndexType::U16;
		header.m_totalIndexCount = mesh.mNumFaces * vertCountPerFace;
		header.m_totalVertexCount = mesh.mNumVertices;
		header.m_subMeshCount = 1;
		header.m_aabbMin = aabbMin;
		header.m_aabbMax = aabbMax;
	}

	// Open file
	std::fstream file;
	file.open(m_outputDirectory + name + ".ankimesh", std::ios::out | std::ios::binary);

	// Write header
	file.write(reinterpret_cast<char*>(&header), sizeof(header));

	// Write sub meshes
	{
		anki::MeshBinaryFile::SubMesh smesh;
		smesh.m_firstIndex = 0;
		smesh.m_indexCount = header.m_totalIndexCount;
		smesh.m_aabbMin = aabbMin;
		smesh.m_aabbMax = aabbMax;

		file.write(reinterpret_cast<char*>(&smesh), sizeof(smesh));
	}

	// Write indices
	for(unsigned i = 0; i < mesh.mNumFaces; i++)
	{
		const aiFace& face = mesh.mFaces[i];

		if(face.mNumIndices != vertCountPerFace)
		{
			ERROR("For some reason assimp returned wrong number of verts for a face (face.mNumIndices=%d). Probably"
				  "degenerates in input file",
				face.mNumIndices);
		}

		for(unsigned j = 0; j < vertCountPerFace; j++)
		{
			uint32_t index32 = face.mIndices[j];
			if(index32 > 0xFFFF)
			{
				ERROR("Index too big");
			}

			uint16_t index = index32;
			file.write(reinterpret_cast<char*>(&index), sizeof(index));
		}
	}

	// Write first vert buffer
	{
		const auto& posa = header.m_vertexAttributes[anki::VertexAttributeLocation::POSITION];
		if(posa.m_format == anki::Format::R32G32B32_SFLOAT)
		{
			file.write(reinterpret_cast<char*>(&positions[0]), positions.size() * sizeof(positions[0]));
		}
		else if(posa.m_format == anki::Format::R16G16B16_SFLOAT)
		{
			std::vector<uint16_t> pos16;
			pos16.resize(positions.size());

			for(unsigned i = 0; i < positions.size(); ++i)
			{
				pos16[i] = anki::F16(positions[i]).toU16();
			}

			file.write(reinterpret_cast<char*>(&pos16[0]), pos16.size() * sizeof(pos16[0]));
		}
		else
		{
			assert(0);
		}
	}

	// Write 2nd vert buffer
	{
		struct Vert
		{
			uint32_t m_n;
			uint32_t m_t;
			uint16_t m_uv[2];
		};

		std::vector<Vert> verts;
		verts.resize(mesh.mNumVertices);

		for(unsigned i = 0; i < mesh.mNumVertices; ++i)
		{
			const auto& inVert = ntVerts[i];

			verts[i].m_n = anki::packColorToR10G10B10A2SNorm(inVert.m_n[0], inVert.m_n[1], inVert.m_n[2], 0.0);
			verts[i].m_t =
				anki::packColorToR10G10B10A2SNorm(inVert.m_t[0], inVert.m_t[1], inVert.m_t[2], inVert.m_t[3]);

			const float uv[2] = {inVert.m_uv[0], inVert.m_uv[1]};
			const anki::Format uvfmt = header.m_vertexAttributes[anki::VertexAttributeLocation::UV].m_format;
			if(uvfmt == anki::Format::R16G16_UNORM)
			{
				assert(uv[0] <= 1.0 && uv[0] >= 0.0 && uv[1] <= 1.0 && uv[1] >= 0.0);
				verts[i].m_uv[0] = uv[0] * 0xFFFF;
				verts[i].m_uv[1] = uv[1] * 0xFFFF;
			}
			else if(uvfmt == anki::Format::R16G16_SFLOAT)
			{
				verts[i].m_uv[0] = anki::F16(uv[0]).toU16();
				verts[i].m_uv[1] = anki::F16(uv[1]).toU16();
			}
			else
			{
				assert(0);
			}
		}

		file.write(reinterpret_cast<char*>(&verts[0]), verts.size() * sizeof(verts[0]));
	}

	// Write 3rd vert buffer
	if(hasBoneWeights)
	{
		file.write(reinterpret_cast<char*>(&bweights[0]), bweights.size() * sizeof(bweights[0]));
	}
}
