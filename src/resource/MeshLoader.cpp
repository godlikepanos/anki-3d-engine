// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/MeshLoader.h"
#include "anki/util/File.h"
#include "anki/util/Logger.h"
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
:	m_alloc(alloc)
{}

//==============================================================================
MeshLoader::~MeshLoader()
{
	m_positions.destroy(m_alloc);

	m_normals.destroy(m_alloc);
	m_normalsF16.destroy(m_alloc);

	m_tangents.destroy(m_alloc);
	m_tangentsF16.destroy(m_alloc);

	m_texCoords.destroy(m_alloc);
	m_texCoordsF16.destroy(m_alloc);

	m_weights.destroy(m_alloc);

	m_tris.destroy(m_alloc);

	m_vertIndices.destroy(m_alloc);
}

//==============================================================================
Error MeshLoader::load(const CString& filename)
{
	Error err = ErrorCode::NONE;

		// Open the file
	File file;
	ANKI_CHECK(file.open(filename, 
		File::OpenFlag::READ | File::OpenFlag::BINARY 
		| File::OpenFlag::LITTLE_ENDIAN));

	// Magic word
	char magic[8];
	ANKI_CHECK(file.read(magic, sizeof(magic)));
	
	if(std::memcmp(magic, "ANKIMESH", 8))
	{
		ANKI_LOGE("Incorrect magic word");
		return ErrorCode::USER_DATA;
	}

	// Mesh name
	{
		U32 strLen;
		ANKI_CHECK(file.readU32(strLen));
		ANKI_CHECK(file.seek(strLen, File::SeekOrigin::CURRENT));
	}

	// Verts num
	U32 vertsNum;
	ANKI_CHECK(file.readU32(vertsNum));

	ANKI_CHECK(m_positions.create(m_alloc, vertsNum));

	// Vert coords
	for(Vec3& vertCoord : m_positions)
	{
		for(U j = 0; j < 3; j++)
		{
			ANKI_CHECK(file.readF32(vertCoord[j]));
		}
	}

	// Faces num
	U32 facesNum;
	ANKI_CHECK(file.readU32(facesNum));

	ANKI_CHECK(m_tris.create(m_alloc, facesNum));

	// Faces IDs
	for(Triangle& tri : m_tris)
	{
		for(U j = 0; j < 3; j++)
		{
			ANKI_CHECK(file.readU32(tri.m_vertIds[j]));

			// a sanity check
			if(tri.m_vertIds[j] >= m_positions.getSize())
			{
				ANKI_LOGE("Vert index out of bounds");
				return ErrorCode::USER_DATA;
			}
		}
	}

	// Tex coords num
	U32 texCoordsNum;
	ANKI_CHECK(file.readU32(texCoordsNum));
	ANKI_CHECK(m_texCoords.create(m_alloc, texCoordsNum));

	// Tex coords
	for(Vec2& texCoord : m_texCoords)
	{
		for(U32 i = 0; i < 2; i++)
		{
			ANKI_CHECK(file.readF32(texCoord[i]));
		}
	}

	// Vert weights num
	U32 weightsNum;
	ANKI_CHECK(file.readU32(weightsNum));
	ANKI_CHECK(m_weights.create(m_alloc, weightsNum));

	// Vert weights
	for(VertexWeight& vw : m_weights)
	{
		// get the bone connections num
		U32 boneConnections;
		ANKI_CHECK(file.readU32(boneConnections));

		// we treat as error if one vert doesnt have a bone
		if(boneConnections < 1)
		{
			ANKI_LOGE("Vert sould have at least one bone");
			return ErrorCode::USER_DATA;
		}

		// and here is another possible error
		if(boneConnections > VertexWeight::MAX_BONES_PER_VERT)
		{
			U32 tmp = VertexWeight::MAX_BONES_PER_VERT;
			ANKI_LOGE("Cannot have more than %d bones per vertex", tmp);
			return ErrorCode::USER_DATA;
		}

		vw.m_bonesCount = boneConnections;

		// for all the weights of the current vertes
		for(U32 i = 0; i < vw.m_bonesCount; i++)
		{
			// read bone id
			U32 boneId;
			ANKI_CHECK(file.readU32(boneId));

			vw.m_boneIds[i] = boneId;

			// read the weight of that bone
			F32 weight;
			ANKI_CHECK(file.readF32(weight));
			vw.m_weights[i] = weight;
		}
	} // end for all vert weights

	err = doPostLoad();

	return err;
}


//==============================================================================
Error MeshLoader::doPostLoad()
{
	Error err = ErrorCode::NONE;

	// Sanity checks
	if(m_positions.getSize() < 1 || m_tris.getSize() < 1)
	{
		ANKI_LOGE("Vert coords and tris must be filled");
		return ErrorCode::USER_DATA;
	}

	if(m_texCoords.getSize() != 0 
		&& m_texCoords.getSize() != m_positions.getSize())
	{
		ANKI_LOGE("Tex coords num must be "
			"zero or equal to the vertex "
			"coords num");
		return ErrorCode::USER_DATA;
	}

	if(m_weights.getSize() != 0 
		&& m_weights.getSize() != m_positions.getSize())
	{
		ANKI_LOGE("Vert weights num must be zero or equal to the "
			"vertex coords num");
		return ErrorCode::USER_DATA;
	}

	ANKI_CHECK(createAllNormals());
	//fixNormals();

	if(m_texCoords.getSize() > 0)
	{
		ANKI_CHECK(createVertTangents());
	}

	ANKI_CHECK(createVertIndeces());
	ANKI_CHECK(compressBuffers());

	return err;
}

//==============================================================================
Error MeshLoader::createVertIndeces()
{
	Error err = m_vertIndices.create(m_alloc, m_tris.getSize() * 3);
	if(err)
	{
		return err;
	}
	
	U j = 0;
	for(U i = 0; i < m_tris.getSize(); ++i)
	{
		m_vertIndices[j + 0] = m_tris[i].m_vertIds[0];
		m_vertIndices[j + 1] = m_tris[i].m_vertIds[1];
		m_vertIndices[j + 2] = m_tris[i].m_vertIds[2];

		j += 3;
	}

	return err;
}

//==============================================================================
Error MeshLoader::createFaceNormals()
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

	return ErrorCode::NONE;
}

//==============================================================================
Error MeshLoader::createVertNormals()
{
	Error err = m_normals.create(m_alloc, m_positions.getSize());
	if(err)
	{
		return err;
	}

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

	return err;
}

//==============================================================================
Error MeshLoader::createVertTangents()
{
	Error err = m_tangents.create(m_alloc, m_positions.getSize(), Vec4(0.0));
	if(err)
	{
		return err;
	}

	MLDArray<Vec3> bitagents;
	err = bitagents.create(m_alloc, m_positions.getSize(), Vec3(0.0));
	if(err)
	{
		return err;
	}

	for(U32 i = 0; i < m_tris.getSize(); i++)
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

	for(U i = 0; i < m_tangents.getSize(); i++)
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

	bitagents.destroy(m_alloc);
	return err;
}

//==============================================================================
void MeshLoader::fixNormals()
{
	FixNormalsMap map(10, Hasher(), Equal(), m_alloc);

	// For all verts
	for(U i = 1; i < m_positions.getSize(); i++)
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
Error MeshLoader::append(const MeshLoader& other)
{
#if 0
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
#endif
	ANKI_ASSERT(0 && "TODO");
	return ErrorCode::NONE;
}

//==============================================================================
Error MeshLoader::compressBuffers()
{
	ANKI_ASSERT(m_positions.getSize() > 0);

	Error err = ErrorCode::NONE;

	// Normals
	ANKI_CHECK(m_normalsF16.create(m_alloc, m_normals.getSize()));

	for(U i = 0; i < m_normals.getSize(); i++)
	{
		for(U j = 0; j < 3; j++)
		{
			m_normalsF16[i][j] = F16(m_normals[i][j]);
		}
	}

	// Tangents
	ANKI_CHECK(m_tangentsF16.create(m_alloc, m_tangents.getSize()));

	for(U i = 0; i < m_tangents.getSize(); i++)
	{
		for(U j = 0; j < 4; j++)
		{
			m_tangentsF16[i][j] = F16(m_tangents[i][j]);
		}
	}

	// Texture coords
	ANKI_CHECK(m_texCoordsF16.create(m_alloc, m_texCoords.getSize()));

	for(U i = 0; i < m_texCoords.getSize(); i++)
	{
		for(U j = 0; j < 2; j++)
		{
			m_texCoordsF16[i][j] = F16(m_texCoords[i][j]);
		}
	}

	return err;
}

} // end namespace anki
