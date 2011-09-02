#include <fstream>
#include <cstring>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#include "MeshData.h"
#include "util/BinaryStream.h"


//==============================================================================
// load                                                                        =
//==============================================================================
void MeshData::load(const char* filename)
{
	// Try
	try
	{
		// Open the file
		std::fstream file(filename, std::fstream::in | std::fstream::binary);

		if(!file.is_open())
		{
			throw EXCEPTION("Cannot open file \"" + filename + "\"");
		}

		BinaryStream bs(file.rdbuf());

		// Magic word
		char magic[8];
		bs.read(magic, 8);
		if(bs.fail() || memcmp(magic, "ANKIMESH", 8))
		{
			throw EXCEPTION("Incorrect magic word");
		}

		// Mesh name
		std::string meshName = bs.readString();

		// Verts num
		uint vertsNum = bs.readUint();
		vertCoords.resize(vertsNum);

		// Vert coords
		BOOST_FOREACH(Vec3& vertCoord, vertCoords)
		{
			for(uint j = 0; j < 3; j++)
			{
				vertCoord[j] = bs.readFloat();
			}
		}

		// Faces num
		uint facesNum = bs.readUint();
		tris.resize(facesNum);

		// Faces IDs
		BOOST_FOREACH(Triangle& tri, tris)
		{
			for(uint j = 0; j < 3; j++)
			{
				tri.vertIds[j] = bs.readUint();

				// a sanity check
				if(tri.vertIds[j] >= vertCoords.size())
				{
					throw EXCEPTION("Vert index out of bounds");
				}
			}
		}

		// Tex coords num
		uint texCoordsNum = bs.readUint();
		texCoords.resize(texCoordsNum);

		// Tex coords
		BOOST_FOREACH(Vec2& texCoord, texCoords)
		{
			for(uint i = 0; i < 2; i++)
			{
				texCoord[i] = bs.readFloat();
			}
		}

		// Vert weights num
		uint vertWeightsNum = bs.readUint();
		vertWeights.resize(vertWeightsNum);

		// Vert weights
		BOOST_FOREACH(VertexWeight& vw, vertWeights)
		{
			// get the bone connections num
			uint boneConnections = bs.readUint();

			// we treat as error if one vert doesnt have a bone
			if(boneConnections < 1)
			{
				throw EXCEPTION("Vert sould have at least one bone");
			}

			// and here is another possible error
			if(boneConnections > VertexWeight::MAX_BONES_PER_VERT)
			{
				uint tmp = VertexWeight::MAX_BONES_PER_VERT;
				throw EXCEPTION("Cannot have more than " +
					boost::lexical_cast<std::string>(tmp) +
					" bones per vertex");
			}
			vw.bonesNum = boneConnections;

			// for all the weights of the current vertes
			for(uint i = 0; i < vw.bonesNum; i++)
			{
				// read bone id
				uint boneId = bs.readUint();
				vw.boneIds[i] = boneId;

				// read the weight of that bone
				float weight = bs.readFloat();
				vw.weights[i] = weight;
			}
		} // end for all vert weights

		doPostLoad();
	}
	catch(Exception& e)
	{
		throw EXCEPTION("File \"" + filename + "\": " + e.what());
	}
}


//==============================================================================
// doPostLoad                                                                  =
//==============================================================================
void MeshData::doPostLoad()
{
	// Sanity checks
	if(vertCoords.size() < 1 || tris.size() < 1)
	{
		throw EXCEPTION("Vert coords and tris must be filled");
	}
	if(texCoords.size() != 0 && texCoords.size() != vertCoords.size())
	{
		throw EXCEPTION("Tex coords num must be zero or equal to the vertex "
			"coords num");
	}
	if(vertWeights.size() != 0 && vertWeights.size() != vertCoords.size())
	{
		throw EXCEPTION("Vert weights num must be zero or equal to the "
			"vertex coords num");
	}

	createAllNormals();
	if(texCoords.size() > 0)
	{
		createVertTangents();
	}
	createVertIndeces();
}


//==============================================================================
// createFaceNormals                                                           =
//==============================================================================
void MeshData::createVertIndeces()
{
	vertIndeces.resize(tris.size() * 3);
	for(uint i = 0; i < tris.size(); i++)
	{
		vertIndeces[i * 3 + 0] = tris[i].vertIds[0];
		vertIndeces[i * 3 + 1] = tris[i].vertIds[1];
		vertIndeces[i * 3 + 2] = tris[i].vertIds[2];
	}
}


//==============================================================================
// createFaceNormals                                                           =
//==============================================================================
void MeshData::createFaceNormals()
{
	BOOST_FOREACH(Triangle& tri, tris)
	{
		const Vec3& v0 = vertCoords[tri.vertIds[0]];
		const Vec3& v1 = vertCoords[tri.vertIds[1]];
		const Vec3& v2 = vertCoords[tri.vertIds[2]];

		/*std::cout << v0 << std::endl;
		std::cout << v1 << std::endl;
		std::cout << v2 << std::endl;
		std::cout << std::endl;*/

		tri.normal = (v1 - v0).cross(v2 - v0);

		tri.normal.normalize();
	}
}


//==============================================================================
// createVertNormals                                                           =
//==============================================================================
void MeshData::createVertNormals()
{
	vertNormals.resize(vertCoords.size());

	BOOST_FOREACH(Vec3& vertNormal, vertNormals)
	{
		vertNormal = Vec3(0.0, 0.0, 0.0);
	}

	BOOST_FOREACH(Triangle& tri, tris)
	{
		vertNormals[tri.vertIds[0]] += tri.normal;
		vertNormals[tri.vertIds[1]] += tri.normal;
		vertNormals[tri.vertIds[2]] += tri.normal;
	}

	BOOST_FOREACH(Vec3& vertNormal, vertNormals)
	{
		vertNormal.normalize();
	}
}


//==============================================================================
// createVertTangents                                                          =
//==============================================================================
void MeshData::createVertTangents()
{
	vertTangents.resize(vertCoords.size(), Vec4(0.0)); // alloc
	Vec<Vec3> bitagents(vertCoords.size(), Vec3(0.0));

	for(uint i = 0; i < tris.size(); i++)
	{
		const Triangle& tri = tris[i];
		const int i0 = tri.vertIds[0];
		const int i1 = tri.vertIds[1];
		const int i2 = tri.vertIds[2];
		const Vec3& v0 = vertCoords[i0];
		const Vec3& v1 = vertCoords[i1];
		const Vec3& v2 = vertCoords[i2];
		Vec3 edge01 = v1 - v0;
		Vec3 edge02 = v2 - v0;
		Vec2 uvedge01 = texCoords[i1] - texCoords[i0];
		Vec2 uvedge02 = texCoords[i2] - texCoords[i0];


		float det = (uvedge01.y() * uvedge02.x()) -
			(uvedge01.x() * uvedge02.y());
		if(Math::isZero(det))
		{
			//WARNING(getRsrcName() << ": det == " << fixed << det);
			det = 0.0001;
		}
		else
		{
			det = 1.0 / det;
		}

		Vec3 t = (edge02 * uvedge01.y() - edge01 * uvedge02.y()) * det;
		Vec3 b = (edge02 * uvedge01.x() - edge01 * uvedge02.x()) * det;
		t.normalize();
		b.normalize();

		vertTangents[i0] += Vec4(t, 1.0);
		vertTangents[i1] += Vec4(t, 1.0);
		vertTangents[i2] += Vec4(t, 1.0);

		bitagents[i0] += b;
		bitagents[i1] += b;
		bitagents[i2] += b;
	}

	for(uint i = 0; i < vertTangents.size(); i++)
	{
		Vec3 t = Vec3(vertTangents[i]);
		const Vec3& n = vertNormals[i];
		Vec3& b = bitagents[i];

		//t = t - n * n.dot(t);
		t.normalize();

		b.normalize();

		float w = ((n.cross(t)).dot(b) < 0.0) ? 1.0 : -1.0;

		vertTangents[i] = Vec4(t, w);
	}
}
