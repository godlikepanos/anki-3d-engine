// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "Exporter.h"

//==============================================================================
enum class ComponentFormat: uint32_t
{
	NONE,

	R8,
	R8G8,
	R8G8B8,
	R8G8B8A8,

	R16,
	R16G16,
	R16G16B16,
	R16G16B16A16,

	R32,
	R32G32,
	R32G32B32,
	R32G32B32A32,

	R10G10B10A2,

	COUNT
};

enum class FormatTransform: uint32_t
{
	NONE,

	UNORM,
	SNORM,
	UINT,
	SINT,
	FLOAT,

	COUNT
};

struct Format
{
	ComponentFormat m_components = ComponentFormat::NONE;
	FormatTransform m_transform = FormatTransform::NONE;
};

struct Header
{
	char m_magic[8]; ///< Magic word.
	uint32_t m_flags;
	uint32_t m_flags2;
	Format m_positionsFormat;
	Format m_normalsFormat;
	Format m_tangentsFormat;
	Format m_colorsFormat; ///< Vertex color.
	Format m_uvsFormat;
	Format m_boneWeightsFormat;
	Format m_boneIndicesFormat;
	Format m_indicesFormat; ///< Vertex indices.

	uint32_t m_totalIndicesCount;
	uint32_t m_totalVerticesCount;
	uint32_t m_uvsChannelCount;
	uint32_t m_subMeshCount;

	uint8_t m_padding[32];
};

struct SubMesh
{
	uint32_t m_firstIndex = 0;
	uint32_t m_indicesCount = 0;
};

struct Vertex
{
	float m_position[3];
	uint32_t m_normal;
	uint32_t m_tangent;
	uint16_t m_uv[2];
};

//==============================================================================
static uint16_t toF16(float f)
{
	union Val32
	{
		int32_t i;
		float f;
		uint32_t u;
	};

	uint16_t out;
	Val32 v32;
	v32.f = f;
	int32_t i = v32.i;
	int32_t s = (i >> 16) & 0x00008000;
	int32_t e = ((i >> 23) & 0x000000ff) - (127 - 15);
	int32_t m = i & 0x007fffff;

	if(e <= 0)
	{
		if(e < -10)
		{
			out = 0;
		}
		else
		{
			m = (m | 0x00800000) >> (1 - e);

			if(m & 0x00001000) 
			{
				m += 0x00002000;
			}

			out = s | (m >> 13);
		}
	}
	else if(e == 0xff - (127 - 15))
	{
		if(m == 0)
		{
			out = s | 0x7c00;
		}
		else
		{
			m >>= 13;
			out = s | 0x7c00 | m | (m == 0);
		}
	}
	else
	{
		if(m &  0x00001000)
		{
			m += 0x00002000;

			if(m & 0x00800000)
			{
				m =  0;
				e += 1;
			}
		}

		if (e > 30)
		{
			assert(0 && "Overflow");
			out = s | 0x7c00;
		}
		else
		{
			out = s | (e << 10) | (m >> 13);
		}
	}

	return out;
}

//==============================================================================
union SignedR10G10B10A10
{
	struct
	{
		int m_x: 10;
		int m_y: 10;
		int m_z: 10;
		int m_w: 2;
	} m_unpacked;
	uint32_t m_packed;
};

//==============================================================================
uint32_t toR10G10B10A2Sint(float r, float g, float b, float a)
{
	SignedR10G10B10A10 out;
	out.m_unpacked.m_x = int(round(r * 511.0));
	out.m_unpacked.m_y = int(round(g * 511.0));
	out.m_unpacked.m_z = int(round(b * 511.0));
	out.m_unpacked.m_w = int(round(a * 1.0));

	return out.m_packed;
}

//==============================================================================
void Exporter::exportMesh(
	const aiMesh& mesh, 
	const aiMatrix4x4* transform) const
{
#if 0
	std::string name = getMeshName(mesh);
	std::fstream file;
	LOGI("Exporting mesh %s", name.c_str());

	uint32_t vertsCount = mesh.mNumVertices;

	// Open file
	file.open(m_outputDirectory + name + ".ankimesh",
		std::ios::out | std::ios::binary);

	// Write magic word
	file.write("ANKIMESH", 8);

	// Write the name
	uint32_t size = name.size();
	file.write((char*)&size, sizeof(uint32_t));
	file.write(&name[0], size);

	// Write positions
	file.write((char*)&vertsCount, sizeof(uint32_t));
	for(uint32_t i = 0; i < mesh.mNumVertices; i++)
	{
		aiVector3D pos = mesh.mVertices[i];

		// Transform
		if(transform)
		{
			pos = (*transform) * pos;
		}

		// flip 
		if(m_flipyz)
		{
			static const aiMatrix4x4 toLefthanded(
				1, 0, 0, 0, 
				0, 0, 1, 0, 
				0, -1, 0, 0, 
				0, 0, 0, 1);

			pos = toLefthanded * pos;
		}

		for(uint32_t j = 0; j < 3; j++)
		{
			file.write((char*)&pos[j], sizeof(float));
		}
	}

	// Write the indices
	file.write((char*)&mesh.mNumFaces, sizeof(uint32_t));
	for(uint32_t i = 0; i < mesh.mNumFaces; i++)
	{
		const aiFace& face = mesh.mFaces[i];
		
		if(face.mNumIndices != 3)
		{
			ERROR("For some reason the assimp didn't triangulate");
		}

		for(uint32_t j = 0; j < 3; j++)
		{
			uint32_t index = face.mIndices[j];
			file.write((char*)&index, sizeof(uint32_t));
		}
	}

	// Write the tex coords
	file.write((char*)&vertsCount, sizeof(uint32_t));

	// For all channels
	for(uint32_t ch = 0; ch < mesh.GetNumUVChannels(); ch++)
	{
		if(mesh.mNumUVComponents[ch] != 2)
		{
			ERROR("Incorrect number of UV components");
		}

		// For all tex coords of this channel
		for(uint32_t i = 0; i < vertsCount; i++)
		{
			aiVector3D texCoord = mesh.mTextureCoords[ch][i];

			for(uint32_t j = 0; j < 2; j++)
			{
				file.write((char*)&texCoord[j], sizeof(float));
			}
		}
	}

	// Write bone weigths count
	if(mesh.HasBones())
	{
#if 0
		// Write file
		file.write((char*)&vertsCount, sizeof(uint32_t));

		// Gather info for each vertex
		std::vector<Vw> vw;
		vw.resize(vertsCount);
		memset(&vw[0], 0, sizeof(Vw) * vertsCount);

		// For all bones
		for(uint32_t i = 0; i < mesh.mNumBones; i++)
		{
			const aiBone& bone = *mesh.mBones[i];

			// for every weights of the bone
			for(uint32_t j = 0; j < bone.mWeightsCount; j++)
			{
				const aiVertexWeight& weigth = bone.mWeights[j];

				// Sanity check
				if(weight.mVertexId >= vertCount)
				{
					ERROR("Out of bounds vert ID");
				}

				Vm& a = vm[weight.mVertexId];

				// Check out of bounds
				if(a.bonesCount >= MAX_BONES_PER_VERTEX)
				{
					LOGW("Too many bones for vertex %d", weigth.mVertexId);
					continue;
				}

				// Write to vertex
				a.boneIds[a.bonesCount] = i;
				a.weigths[a.bonesCount] = weigth.mWeigth;
				++a.bonesCount;
			}

			// Now write the file
		}
#endif
	}
	else
	{
		uint32_t num = 0;
		file.write((char*)&num, sizeof(uint32_t));
	}
#endif

	std::string name = mesh.mName.C_Str();
	std::fstream file;
	LOGI("Exporting mesh %s", name.c_str());

	// Open file
	file.open(m_outputDirectory + name + ".ankimesh",
		std::ios::out | std::ios::binary);

	Header header;
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

	if(!mesh.HasPositions()
		|| !mesh.HasNormals()
		|| !mesh.HasTangentsAndBitangents()
		|| !mesh.HasTextureCoords(0))
	{
		ERROR("Missing attribute");
	}
	
	// Write header
	static const char* magic = "ANKIMES2";
	memcpy(&header.m_magic, magic, 8);
	
	header.m_positionsFormat.m_components = ComponentFormat::R32G32B32;
	header.m_positionsFormat.m_transform = FormatTransform::FLOAT;

	header.m_normalsFormat.m_components = ComponentFormat::R10G10B10A2;
	header.m_normalsFormat.m_transform = FormatTransform::SNORM;

	header.m_tangentsFormat.m_components = ComponentFormat::R10G10B10A2;
	header.m_tangentsFormat.m_transform = FormatTransform::SNORM;

	header.m_uvsFormat.m_components = ComponentFormat::R16G16;
	header.m_uvsFormat.m_transform = FormatTransform::FLOAT;

	header.m_indicesFormat.m_components = ComponentFormat::R16;
	header.m_indicesFormat.m_transform = FormatTransform::UINT;

	header.m_totalIndicesCount = mesh.mNumFaces * 3;
	header.m_totalVerticesCount = mesh.mNumVertices;
	header.m_uvsChannelCount = 1;
	header.m_subMeshCount = 1;

	file.write(reinterpret_cast<char*>(&header), sizeof(header));

	// Write sub meshes
	SubMesh smesh;
	smesh.m_firstIndex = 0;
	smesh.m_indicesCount = header.m_totalIndicesCount;
	file.write(reinterpret_cast<char*>(&smesh), sizeof(smesh));

	// Write indices
	for(unsigned i = 0; i < mesh.mNumFaces; i++)
	{
		const aiFace& face = mesh.mFaces[i];
		
		if(face.mNumIndices != 3)
		{
			ERROR("For some reason the assimp didn't triangulate");
		}

		for(unsigned j = 0; j < 3; j++)
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

	// Write vertices
	aiMatrix3x3 normalMat;
	if(transform)
	{
		normalMat = aiMatrix3x3(*transform);
	}

	for(unsigned i = 0; i < mesh.mNumVertices; i++)
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
			static const aiMatrix4x4 toLefthanded(
				1, 0, 0, 0, 
				0, 0, 1, 0, 
				0, -1, 0, 0, 
				0, 0, 0, 1);

			pos = toLefthanded * pos;
			n = toLefthanded * n;
			t = toLefthanded * t;
			b = toLefthanded * b;
		}

		Vertex vert;

		// Position
		vert.m_position[0] = pos[0];
		vert.m_position[1] = pos[1];
		vert.m_position[2] = pos[2];

		// Normal
		vert.m_normal = toR10G10B10A2Sint(n[0], n[1], n[2], 0.0);

		// Tangent
		float w = ((n ^ t) * b < 0.0) ? 1.0 : -1.0;
		vert.m_tangent = toR10G10B10A2Sint(t[0], t[1], t[2], w);

		// Tex coords
		vert.m_uv[0] = toF16(uv[0]);
		vert.m_uv[1] = toF16(uv[1]);

		// Write
		file.write(reinterpret_cast<char*>(&vert), sizeof(vert));
	}
}
