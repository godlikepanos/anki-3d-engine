// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/MeshLoader.h"
#include "anki/util/File.h"
#include <cstring>
#include <unordered_map>

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

/// The hash functor
class Hasher
{
public:
	PtrSize operator()(const Vec3& pos) const
	{
		F32 sum = pos.x() * 100.0 + pos.y() * 10.0 + pos.z();
		PtrSize hash = 0;
		std::memcpy(&hash, &sum, sizeof(F32));
		return hash;
	}
};

/// The collision evaluation function
class Equal
{
public:
	Bool operator()(const Vec3& a, const Vec3& b) const
	{
		return a == b;
	}
};

/// The value of the hash map
class MapValue
{
public:
	U8 m_indicesCount = 0;
	Array<U32, 16> m_indices;
};

using FixNormalsMap = std::unordered_map<
	Vec3, MapValue, Hasher, Equal, 
	TempResourceAllocator<std::pair<Vec3, MapValue>>>;

//==============================================================================
// MeshLoader                                                                  =
//==============================================================================

//==============================================================================
MeshLoader::MeshLoader(TempResourceAllocator<U8>& alloc)
:	m_positions(alloc),
	m_normals(alloc),
	m_normalsF16(alloc),
	m_tangents(alloc),
	m_tangentsF16(alloc),
	m_texCoords(alloc),
	m_texCoordsF16(alloc),
	m_weights(alloc),
	m_tris(alloc),
	m_vertIndices(alloc)
{}

//==============================================================================
MeshLoader::MeshLoader(const char* filename, TempResourceAllocator<U8>& alloc)
:	MeshLoader(filename, alloc)
{
	load(filename);
}

//==============================================================================
void MeshLoader::load(const char* filename)
{
	// Try
	try
	{
		// Open the file
		File file(filename, 
			File::OpenFlag::READ | File::OpenFlag::BINARY 
			| File::OpenFlag::LITTLE_ENDIAN);

		// Magic word
		char magic[8];
		file.read(magic, sizeof(magic));
		if(std::memcmp(magic, "ANKIMESH", 8))
		{
			throw ANKI_EXCEPTION("Incorrect magic word");
		}

		// Mesh name
		{
			U32 strLen = file.readU32();
			file.seek(strLen, File::SeekOrigin::CURRENT);
		}

		// Verts num
		U vertsNum = file.readU32();
		m_positions.resize(vertsNum);

		// Vert coords
		for(Vec3& vertCoord : m_positions)
		{
			for(U j = 0; j < 3; j++)
			{
				vertCoord[j] = file.readF32();
			}
		}

		// Faces num
		U facesNum = file.readU32();
		m_tris.resize(facesNum);

		// Faces IDs
		for(Triangle& tri : m_tris)
		{
			for(U j = 0; j < 3; j++)
			{
				tri.m_vertIds[j] = file.readU32();

				// a sanity check
				if(tri.m_vertIds[j] >= m_positions.size())
				{
					throw ANKI_EXCEPTION("Vert index out of bounds");
				}
			}
		}

		// Tex coords num
		U texCoordsNum = file.readU32();
		m_texCoords.resize(texCoordsNum);

		// Tex coords
		for(Vec2& texCoord : m_texCoords)
		{
			for(U32 i = 0; i < 2; i++)
			{
				texCoord[i] = file.readF32();
			}
		}

		// Vert weights num
		U weightsNum = file.readU32();
		m_weights.resize(weightsNum);

		// Vert weights
		for(VertexWeight& vw : m_weights)
		{
			// get the bone connections num
			U32 boneConnections = file.readU32();

			// we treat as error if one vert doesnt have a bone
			if(boneConnections < 1)
			{
				throw ANKI_EXCEPTION("Vert sould have at least one bone");
			}

			// and here is another possible error
			if(boneConnections > VertexWeight::MAX_BONES_PER_VERT)
			{
				U32 tmp = VertexWeight::MAX_BONES_PER_VERT;
				throw ANKI_EXCEPTION("Cannot have more than %d "
					"bones per vertex", tmp);
			}
			vw.m_bonesCount = boneConnections;

			// for all the weights of the current vertes
			for(U32 i = 0; i < vw.m_bonesCount; i++)
			{
				// read bone id
				U32 boneId = file.readU32();
				vw.m_boneIds[i] = boneId;

				// read the weight of that bone
				float weight = file.readF32();
				vw.m_weights[i] = weight;
			}
		} // end for all vert weights

		doPostLoad();
	}
	catch(Exception& e)
	{
		throw ANKI_EXCEPTION("Loading of mesh failed") << e;
	}
}


//==============================================================================
void MeshLoader::doPostLoad()
{
	// Sanity checks
	if(m_positions.size() < 1 || m_tris.size() < 1)
	{
		throw ANKI_EXCEPTION("Vert coords and tris must be filled");
	}

	if(m_texCoords.size() != 0 && m_texCoords.size() != m_positions.size())
	{
		throw ANKI_EXCEPTION("Tex coords num must be "
			"zero or equal to the vertex "
			"coords num");
	}

	if(m_weights.size() != 0 && m_weights.size() != m_positions.size())
	{
		throw ANKI_EXCEPTION("Vert weights num must be zero or equal to the "
			"vertex coords num");
	}

	createAllNormals();
	fixNormals();

	if(m_texCoords.size() > 0)
	{
		createVertTangents();
	}

	createVertIndeces();
	compressBuffers();
}

//==============================================================================
void MeshLoader::createVertIndeces()
{
	m_vertIndices.resize(m_tris.size() * 3);
	U j = 0;
	for(U i = 0; i < m_tris.size(); ++i)
	{
		m_vertIndices[j + 0] = m_tris[i].m_vertIds[0];
		m_vertIndices[j + 1] = m_tris[i].m_vertIds[1];
		m_vertIndices[j + 2] = m_tris[i].m_vertIds[2];

		j += 3;
	}
}

//==============================================================================
void MeshLoader::createFaceNormals()
{
	for(Triangle& tri : m_tris)
	{
		const Vec3& v0 = m_positions[tri.m_vertIds[0]];
		const Vec3& v1 = m_positions[tri.m_vertIds[1]];
		const Vec3& v2 = m_positions[tri.m_vertIds[2]];

		tri.m_normal = (v1 - v0).cross(v2 - v0);

		if(tri.m_normal != Vec3(0.0))
		{
			//tri.normal.normalize();
		}
		else
		{
			tri.m_normal = Vec3(1.0, 0.0, 0.0);
		}
	}
}

//==============================================================================
void MeshLoader::createVertNormals()
{
	m_normals.resize(m_positions.size());

	for(Vec3& vertNormal : m_normals)
	{
		vertNormal = Vec3(0.0);
	}

	for(Triangle& tri : m_tris)
	{
		m_normals[tri.m_vertIds[0]] += tri.m_normal;
		m_normals[tri.m_vertIds[1]] += tri.m_normal;
		m_normals[tri.m_vertIds[2]] += tri.m_normal;
	}

	for(Vec3& vertNormal : m_normals)
	{
		if(vertNormal != Vec3(0.0))
		{
			vertNormal.normalize();
		}
	}
}

//==============================================================================
void MeshLoader::createVertTangents()
{
	m_tangents.resize(m_positions.size(), Vec4(0.0)); // alloc
	MLVector<Vec3> bitagents(
		m_positions.size(), Vec3(0.0), m_tangents.get_allocator());

	for(U32 i = 0; i < m_tris.size(); i++)
	{
		const Triangle& tri = m_tris[i];
		const I i0 = tri.m_vertIds[0];
		const I i1 = tri.m_vertIds[1];
		const I i2 = tri.m_vertIds[2];
		const Vec3& v0 = m_positions[i0];
		const Vec3& v1 = m_positions[i1];
		const Vec3& v2 = m_positions[i2];
		Vec3 edge01 = v1 - v0;
		Vec3 edge02 = v2 - v0;
		Vec2 uvedge01 = m_texCoords[i1] - m_texCoords[i0];
		Vec2 uvedge02 = m_texCoords[i2] - m_texCoords[i0];

		F32 det = (uvedge01.y() * uvedge02.x()) -
			(uvedge01.x() * uvedge02.y());

		if(isZero(det))
		{
			det = 0.0001;
		}
		else
		{
			// Calc the det
			det = 1.0 / det;

			// Add a noise to the det to avoid zero tangents on mirrored cases
			// Add 1 to the rand so that it's not zero
			det *= ((rand() % 10 + 1) * getEpsilon<F32>());
		}

		Vec3 t = (edge02 * uvedge01.y() - edge01 * uvedge02.y()) * det;
		Vec3 b = (edge02 * uvedge01.x() - edge01 * uvedge02.x()) * det;
		//t.normalize();
		//b.normalize();

		m_tangents[i0] += Vec4(t, 1.0);
		m_tangents[i1] += Vec4(t, 1.0);
		m_tangents[i2] += Vec4(t, 1.0);

		bitagents[i0] += b;
		bitagents[i1] += b;
		bitagents[i2] += b;
	}

	for(U i = 0; i < m_tangents.size(); i++)
	{
		Vec3 t = m_tangents[i].xyz();
		const Vec3& n = m_normals[i];
		Vec3& b = bitagents[i];

		t = t - n * n.dot(t);

		if(t != Vec3(0.0))
		{
			t.normalize();
		}

		if(b != Vec3(0.0))
		{
			b.normalize();
		}

		F32 w = ((n.cross(t)).dot(b) < 0.0) ? 1.0 : -1.0;

		m_tangents[i] = Vec4(t, w);
	}
}

//==============================================================================
void MeshLoader::fixNormals()
{
	FixNormalsMap map(10, Hasher(), Equal(), m_positions.get_allocator());

	// For all verts
	for(U i = 1; i < m_positions.size(); i++)
	{
		const Vec3& pos = m_positions[i];
		Vec3& norm = m_normals[i];

		// Find pos
		FixNormalsMap::iterator it = map.find(pos);

		// Check if found
		if(it == map.end())
		{
			// Not found

			MapValue val;
			val.m_indices[0] = i;
			val.m_indicesCount = 1;
			map[pos] = val;
		}
		else
		{
			// Found

			MapValue& mapVal = it->second;
			ANKI_ASSERT(mapVal.m_indicesCount > 0);

			// Search the verts with the same position
			for(U j = 0; j < mapVal.m_indicesCount; j++)
			{
				const Vec3& posB = m_positions[mapVal.m_indices[j]];
				Vec3& normB = m_normals[mapVal.m_indices[j]];

				ANKI_ASSERT(posB == pos);
				(void)posB;

				F32 dot = norm.dot(normB);
				F32 ang = acos(dot);

				if(ang <= NORMALS_ANGLE_MERGE)
				{
					Vec3 newNormal = (norm + normB) * 0.5;
					newNormal.normalize();

					norm = newNormal;
					normB = newNormal;
				}
			}

			// Update the map
			mapVal.m_indices[mapVal.m_indicesCount++] = i;
		}
	}
}

//==============================================================================
void MeshLoader::append(const MeshLoader& other)
{
	m_positions.insert(
		m_positions.end(), other.m_positions.begin(), other.m_positions.end());

	//normals.insert(
	//	normals.end(), other.normals.begin(), other.normals.end());

	m_normalsF16.insert(m_normalsF16.end(), other.m_normalsF16.begin(), 
		other.m_normalsF16.end());

	//tangents.insert(
	//	tangents.end(), other.tangents.begin(), other.tangents.end());

	m_tangentsF16.insert(m_tangentsF16.end(), 
		other.m_tangentsF16.begin(), other.m_tangentsF16.end());

	//texCoords.insert(
	//	texCoords.end(), other.texCoords.begin(), other.texCoords.end());

	m_texCoordsF16.insert(
		m_texCoordsF16.end(), other.m_texCoordsF16.begin(), 
		other.m_texCoordsF16.end());

	m_weights.insert(
		m_weights.end(), other.m_weights.begin(), other.m_weights.end());

	U16 bias = m_positions.size();
	for(U16 index : other.m_vertIndices)
	{
		m_vertIndices.push_back(bias + index);
	}
}

//==============================================================================
void MeshLoader::compressBuffers()
{
	ANKI_ASSERT(m_positions.size() > 0);

	// Normals
	m_normalsF16.resize(m_normals.size());

	for(U i = 0; i < m_normals.size(); i++)
	{
		for(U j = 0; j < 3; j++)
		{
			m_normalsF16[i][j] = F16(m_normals[i][j]);
		}
	}

	// Tangents
	m_tangentsF16.resize(m_tangents.size());

	for(U i = 0; i < m_tangents.size(); i++)
	{
		for(U j = 0; j < 4; j++)
		{
			m_tangentsF16[i][j] = F16(m_tangents[i][j]);
		}
	}

	// Texture coords
	m_texCoordsF16.resize(m_texCoords.size());	

	for(U i = 0; i < m_texCoords.size(); i++)
	{
		for(U j = 0; j < 2; j++)
		{
			m_texCoordsF16[i][j] = F16(m_texCoords[i][j]);
		}
	}
}

} // end namespace anki
