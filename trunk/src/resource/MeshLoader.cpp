#include "anki/resource/MeshLoader.h"
#include "anki/util/File.h"
#include <cstring>
#include <unordered_map>

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

/// The hash functor
struct Hasher
{
	size_t operator()(const Vec3& pos) const
	{
		F32 sum = pos.x() * 100.0 + pos.y() * 10.0 + pos.z();
		size_t hash = 0;
		memcpy(&hash, &sum, sizeof(F32));
		return hash;
	}
};

/// The value of the hash map
struct MapValue
{
	U8 indicesCount = 0;
	Array<U32, 16> indices;
};

typedef std::unordered_map<Vec3, MapValue, Hasher> FixNormalsMap;

//==============================================================================
// MeshLoader                                                                  =
//==============================================================================

//==============================================================================
void MeshLoader::load(const char* filename)
{
	// Try
	try
	{
		// Open the file
		File file(filename, 
			File::OF_READ | File::OF_BINARY | File::E_LITTLE_ENDIAN);

		// Magic word
		char magic[8];
		file.read(magic, sizeof(magic));
		if(memcmp(magic, "ANKIMESH", 8))
		{
			throw ANKI_EXCEPTION("Incorrect magic word");
		}

		// Mesh name
		{
			U32 strLen = file.readU32();
			file.seek(strLen, File::SO_CURRENT);
		}

		// Verts num
		U vertsNum = file.readU32();
		positions.resize(vertsNum);

		// Vert coords
		for(Vec3& vertCoord : positions)
		{
			for(U j = 0; j < 3; j++)
			{
				vertCoord[j] = file.readF32();
			}
		}

		// Faces num
		U facesNum = file.readU32();
		tris.resize(facesNum);

		// Faces IDs
		for(Triangle& tri : tris)
		{
			for(U j = 0; j < 3; j++)
			{
				tri.vertIds[j] = file.readU32();

				// a sanity check
				if(tri.vertIds[j] >= positions.size())
				{
					throw ANKI_EXCEPTION("Vert index out of bounds");
				}
			}
		}

		// Tex coords num
		U texCoordsNum = file.readU32();
		texCoords.resize(texCoordsNum);

		// Tex coords
		for(Vec2& texCoord : texCoords)
		{
			for(U32 i = 0; i < 2; i++)
			{
				texCoord[i] = file.readF32();
			}
		}

		// Vert weights num
		U weightsNum = file.readU32();
		weights.resize(weightsNum);

		// Vert weights
		for(VertexWeight& vw : weights)
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
				throw ANKI_EXCEPTION("Cannot have more than "
					+ std::to_string(tmp) + " bones per vertex");
			}
			vw.bonesNum = boneConnections;

			// for all the weights of the current vertes
			for(U32 i = 0; i < vw.bonesNum; i++)
			{
				// read bone id
				U32 boneId = file.readU32();
				vw.boneIds[i] = boneId;

				// read the weight of that bone
				float weight = file.readF32();
				vw.weights[i] = weight;
			}
		} // end for all vert weights

		doPostLoad();
	}
	catch(Exception& e)
	{
		throw ANKI_EXCEPTION("Loading of file failed: " + filename) << e;
	}
}


//==============================================================================
void MeshLoader::doPostLoad()
{
	// Sanity checks
	if(positions.size() < 1 || tris.size() < 1)
	{
		throw ANKI_EXCEPTION("Vert coords and tris must be filled");
	}
	if(texCoords.size() != 0 && texCoords.size() != positions.size())
	{
		throw ANKI_EXCEPTION("Tex coords num must be "
			"zero or equal to the vertex "
			"coords num");
	}
	if(weights.size() != 0 && weights.size() != positions.size())
	{
		throw ANKI_EXCEPTION("Vert weights num must be zero or equal to the "
			"vertex coords num");
	}

	createAllNormals();
	fixNormals();
	if(texCoords.size() > 0)
	{
		createVertTangents();
	}
	createVertIndeces();
	compressBuffers();
}

//==============================================================================
void MeshLoader::createVertIndeces()
{
	vertIndices.resize(tris.size() * 3);
	U j = 0;
	for(U i = 0; i < tris.size(); ++i)
	{
		vertIndices[j + 0] = tris[i].vertIds[0];
		vertIndices[j + 1] = tris[i].vertIds[1];
		vertIndices[j + 2] = tris[i].vertIds[2];

		j += 3;
	}
}

//==============================================================================
void MeshLoader::createFaceNormals()
{
	for(Triangle& tri : tris)
	{
		const Vec3& v0 = positions[tri.vertIds[0]];
		const Vec3& v1 = positions[tri.vertIds[1]];
		const Vec3& v2 = positions[tri.vertIds[2]];

		tri.normal = (v1 - v0).cross(v2 - v0);

		if(tri.normal != Vec3(0.0))
		{
			//tri.normal.normalize();
		}
		else
		{
			tri.normal = Vec3(1.0, 0.0, 0.0);
		}
	}
}

//==============================================================================
void MeshLoader::createVertNormals()
{
	normals.resize(positions.size());

	for(Vec3& vertNormal : normals)
	{
		vertNormal = Vec3(0.0, 0.0, 0.0);
	}

	for(Triangle& tri : tris)
	{
		normals[tri.vertIds[0]] += tri.normal;
		normals[tri.vertIds[1]] += tri.normal;
		normals[tri.vertIds[2]] += tri.normal;
	}

	for(Vec3& vertNormal : normals)
	{
		vertNormal.normalize();
	}
}

//==============================================================================
void MeshLoader::createVertTangents()
{
	tangents.resize(positions.size(), Vec4(0.0)); // alloc
	Vector<Vec3> bitagents(positions.size(), Vec3(0.0));

	for(U32 i = 0; i < tris.size(); i++)
	{
		const Triangle& tri = tris[i];
		const I i0 = tri.vertIds[0];
		const I i1 = tri.vertIds[1];
		const I i2 = tri.vertIds[2];
		const Vec3& v0 = positions[i0];
		const Vec3& v1 = positions[i1];
		const Vec3& v2 = positions[i2];
		Vec3 edge01 = v1 - v0;
		Vec3 edge02 = v2 - v0;
		Vec2 uvedge01 = texCoords[i1] - texCoords[i0];
		Vec2 uvedge02 = texCoords[i2] - texCoords[i0];

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

		tangents[i0] += Vec4(t, 1.0);
		tangents[i1] += Vec4(t, 1.0);
		tangents[i2] += Vec4(t, 1.0);

		bitagents[i0] += b;
		bitagents[i1] += b;
		bitagents[i2] += b;
	}

	for(U i = 0; i < tangents.size(); i++)
	{
		Vec3 t = tangents[i].xyz();
		const Vec3& n = normals[i];
		Vec3& b = bitagents[i];

		t = t - n * n.dot(t);
		t.normalize();

		b.normalize();

		F32 w = ((n.cross(t)).dot(b) < 0.0) ? 1.0 : -1.0;

		tangents[i] = Vec4(t, w);
	}
}

//==============================================================================
void MeshLoader::fixNormals()
{
	FixNormalsMap map;

	// For all verts
	for(U i = 1; i < positions.size(); i++)
	{
		const Vec3& pos = positions[i];
		Vec3& norm = normals[i];

		// Find pos
		FixNormalsMap::iterator it = map.find(pos);

		// Check if found
		if(it == map.end())
		{
			// Not found

			MapValue val;
			val.indices[0] = i;
			val.indicesCount = 1;
			map[pos] = val;
		}
		else
		{
			// Found

			MapValue& mapVal = it->second;
			ANKI_ASSERT(mapVal.indicesCount > 0);

			// Search the verts with the same position
			for(U j = 0; j < mapVal.indicesCount; j++)
			{
				const Vec3& posB = positions[mapVal.indices[j]];
				Vec3& normB = normals[mapVal.indices[j]];

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
			mapVal.indices[mapVal.indicesCount++] = i;
		}
	}
}

//==============================================================================
void MeshLoader::append(const MeshLoader& other)
{
	positions.insert(
		positions.end(), other.positions.begin(), other.positions.end());

	//normals.insert(
	//	normals.end(), other.normals.begin(), other.normals.end());

	normalsF16.insert(
		normalsF16.end(), other.normalsF16.begin(), other.normalsF16.end());

	//tangents.insert(
	//	tangents.end(), other.tangents.begin(), other.tangents.end());

	tangentsF16.insert(
		tangentsF16.end(), other.tangentsF16.begin(), other.tangentsF16.end());

	//texCoords.insert(
	//	texCoords.end(), other.texCoords.begin(), other.texCoords.end());

	texCoordsF16.insert(
		texCoordsF16.end(), other.texCoordsF16.begin(), 
		other.texCoordsF16.end());

	weights.insert(
		weights.end(), other.weights.begin(), other.weights.end());

	U16 bias = positions.size();
	for(U16 index : other.vertIndices)
	{
		vertIndices.push_back(bias + index);
	}
}

//==============================================================================
void MeshLoader::compressBuffers()
{
	ANKI_ASSERT(positions.size() > 0);

	// Normals
	normalsF16.resize(normals.size());

	for(U i = 0; i < normals.size(); i++)
	{
		for(U j = 0; j < 3; j++)
		{
			normalsF16[i][j] = F16(normals[i][j]);
		}
	}

	// Tangents
	tangentsF16.resize(tangents.size());

	for(U i = 0; i < tangents.size(); i++)
	{
		for(U j = 0; j < 4; j++)
		{
			tangentsF16[i][j] = F16(tangents[i][j]);
		}
	}

	// Texture coords
	texCoordsF16.resize(texCoords.size());	

	for(U i = 0; i < texCoords.size(); i++)
	{
		for(U j = 0; j < 2; j++)
		{
			texCoordsF16[i][j] = F16(texCoords[i][j]);
		}
	}
}

} // end namespace anki
