#include <fstream>
#include "Mesh.h"
#include "Material.h"


#define MESH_ERR(x) \
	ERROR("File \"" << filename << "\": " << x)


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
bool Mesh::load(const char* filename)
{
	fstream file(filename, fstream::in | fstream::binary);

	if(!file.is_open())
	{
		ERROR("Cannot open file \"" << filename << "\"");
		return false;
	}

	// Magic word
	char magic[8];
	file.readsome(magic, 8);
	if(file.fail() || memcmp(magic, "ANKIMESH", 8))
	{
		MESH_ERR("Incorrect file type");
		return false;
	}

	// Mesh name
	string meshName;
	if(!Util::readStringFromBinaryStream(file, meshName))
	{
		MESH_ERR("Cannot read mesh name");
		return false;
	}

	// Material
	string materialName;
	if(!Util::readStringFromBinaryStream(file, materialName))
	{
		MESH_ERR("Cannot read material name");
		return false;
	}
	if(materialName.length() > 0)
	{
		material.loadRsrc(materialName.c_str());
	}

	// Verts num
	uint vertsNum;
	if(!Util::readUintFromBinaryStream(file, vertsNum))
	{
		MESH_ERR("Cannot read verts num");
		return false;
	}
	vertCoords.resize(vertsNum);

	// Verts
	for(uint i=0; i<vertCoords.size(); i++)
	{
		for(uint j=0; j<3; j++)
		{
			if(!Util::readFloatFromBinaryStream(file, vertCoords[i][j]))
			{
				MESH_ERR("Cannot read vert coord");
				return false;
			}
		}
	}

	// Faces num
	uint facesNum;
	if(!Util::readUintFromBinaryStream(file, facesNum))
	{
		MESH_ERR("Cannot read faces num");
		return false;
	}
	tris.resize(facesNum);

	// read the faces
	for(uint i=0; i<tris.size(); i++)
	{
		for(uint j=0; j<3; j++)
		{
			if(!Util::readUintFromBinaryStream(file, tris[i].vertIds[j]))
			{
				MESH_ERR("Cannot read vert index");
				return false;
			}

			if(tris[i].vertIds[j] >= vertCoords.size())
			{
				MESH_ERR("Incorrect vert index " << tris[i].vertIds[j] << " (" << i << ", " << j << ")");
				return false;
			}
		}
	}

	// Tex coords num
	uint texCoordsNum;
	if(!Util::readUintFromBinaryStream(file, texCoordsNum))
	{
		MESH_ERR("Cannot read tex coords num");
		return false;
	}
	texCoords.resize(texCoordsNum);

	// Tex coords
	for(uint i=0; i<texCoords.size(); i++)
	{
		for(uint j=0; j<2; j++)
		{
			if(!Util::readFloatFromBinaryStream(file, texCoords[i][j]))
			{
				MESH_ERR("Cannot read tex coord");
				return false;
			}
		}
	}

	// Vert weights num
	uint vertWeightsNum;
	if(!Util::readUintFromBinaryStream(file, vertWeightsNum))
	{
		MESH_ERR("Cannot read vert weights num");
		return false;
	}
	vertWeights.resize(vertWeightsNum);

	// Vert weights
	for(uint i=0; i<vertWeights.size(); i++)
	{
		// get the bone connections num
		uint boneConnections;
		if(!Util::readUintFromBinaryStream(file, boneConnections))
		{
			MESH_ERR("Cannot read vert weight bone connections num");
			return false;
		}

		// we treat as error if one vert doesnt have a bone
		if(boneConnections < 1)
		{
			MESH_ERR("Vert \"" << i << "\" sould have have at least one bone");
			return false;
		}

		// and here is another possible error
		if(boneConnections > VertexWeight::MAX_BONES_PER_VERT)
		{
			MESH_ERR("Cannot have more than " << VertexWeight::MAX_BONES_PER_VERT << " bones per vertex");
			return false;
		}
		vertWeights[i].bonesNum = boneConnections;

		// for all the weights of the current vertes
		for(uint j=0; j<vertWeights[i].bonesNum; j++)
		{
			// read bone id
			uint boneId;
			if(!Util::readUintFromBinaryStream(file, boneId))
			{
				MESH_ERR("Cannot read bone ID");
				return false;
			}
			vertWeights[i].boneIds[j] = boneId;

			// read the weight of that bone
			float weight;
			if(!Util::readFloatFromBinaryStream(file, weight))
			{
				MESH_ERR("Cannot read weight");
				return false;
			}
			vertWeights[i].weights[j] = weight;
		}
	}

	// Sanity checks
	if(vertCoords.size()<1 || tris.size()<1)
	{
		ERROR("Vert coords and tris must be filled \"" << filename << "\"");
		return false;
	}
	if(texCoords.size()!=0 && texCoords.size()!=vertCoords.size())
	{
		ERROR("Tex coords num must be zero or equal to vertex coords num \"" << filename << "\"");
		return false;
	}
	if(vertWeights.size()!=0 && vertWeights.size()!=vertCoords.size())
	{
		ERROR("Vert weights num must be zero or equal to vertex coords num \"" << filename << "\"");
		return false;
	}

	if(isRenderable())
	{
		createAllNormals();
		if(texCoords.size() > 0)
			createVertTangents();
		createVertIndeces();
		createVbos();
		calcBSphere();

		/*
		 * Sanity checks continued
		 */
		if(material->stdAttribVars[Material::SAV_TEX_COORDS] != NULL && !vbos.texCoords.isCreated())
		{
			ERROR("The shader program (\"" << material->shaderProg->getRsrcName() <<
						 "\") needs texture coord information that the mesh (\"" <<
						 getRsrcName() << "\") doesn't have");
		}

		if(material->hasHWSkinning() && !vbos.vertWeights.isCreated())
		{
			ERROR("The shader program (\"" << material->shaderProg->getRsrcName() <<
						 "\") needs vertex weights that the mesh (\"" <<
						 getRsrcName() << "\") doesn't have");
		}
	}

	return true;
}


//======================================================================================================================
// createFaceNormals                                                                                                   =
//======================================================================================================================
void Mesh::createVertIndeces()
{
	DEBUG_ERR(vertIndeces.size() > 0);

	vertIndeces.resize(tris.size() * 3);
	for(uint i=0; i<tris.size(); i++)
	{
		vertIndeces[i*3+0] = tris[i].vertIds[0];
		vertIndeces[i*3+1] = tris[i].vertIds[1];
		vertIndeces[i*3+2] = tris[i].vertIds[2];
	}
}


//======================================================================================================================
// createFaceNormals                                                                                                   =
//======================================================================================================================
void Mesh::createFaceNormals()
{
	for(uint i=0; i<tris.size(); i++)
	{
		Triangle& tri = tris[i];
		const Vec3& v0 = vertCoords[tri.vertIds[0]];
		const Vec3& v1 = vertCoords[tri.vertIds[1]];
		const Vec3& v2 = vertCoords[tri.vertIds[2]];

		tri.normal = (v1 - v0).cross(v2 - v0);

		tri.normal.normalize();
	}
}


//======================================================================================================================
// createVertNormals                                                                                                   =
//======================================================================================================================
void Mesh::createVertNormals()
{
	vertNormals.resize(vertCoords.size()); // alloc

	for(uint i=0; i<vertCoords.size(); i++)
		vertNormals[i] = Vec3(0.0, 0.0, 0.0);

	for(uint i=0; i<tris.size(); i++)
	{
		const Triangle& tri = tris[i];
		vertNormals[tri.vertIds[0]] += tri.normal;
		vertNormals[tri.vertIds[1]] += tri.normal;
		vertNormals[tri.vertIds[2]] += tri.normal;
	}

	for(uint i=0; i<vertNormals.size(); i++)
		vertNormals[i].normalize();
}


//======================================================================================================================
// createVertTangents                                                                                                  =
//======================================================================================================================
void Mesh::createVertTangents()
{
	vertTangents.resize(vertCoords.size()); // alloc

	Vec<Vec3> bitagents(vertCoords.size());

	for(uint i=0; i<vertTangents.size(); i++)
	{
		vertTangents[i] = Vec4(0.0);
		bitagents[i] = Vec3(0.0);
	}

	for(uint i=0; i<tris.size(); i++)
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


		float det = (uvedge01.y * uvedge02.x) - (uvedge01.x * uvedge02.y);
		if(isZero(det))
		{
			//WARNING(getRsrcName() << ": det == " << fixed << det);
			det = 0.0001;
		}
		else
			det = 1.0 / det;

		Vec3 t = (edge02 * uvedge01.y - edge01 * uvedge02.y) * det;
		Vec3 b = (edge02 * uvedge01.x - edge01 * uvedge02.x) * det;
		t.normalize();
		b.normalize();

		vertTangents[i0] += Vec4(t, 1.0);
		vertTangents[i1] += Vec4(t, 1.0);
		vertTangents[i2] += Vec4(t, 1.0);

		bitagents[i0] += b;
		bitagents[i1] += b;
		bitagents[i2] += b;
	}

	for(uint i=0; i<vertTangents.size(); i++)
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


//======================================================================================================================
// createVbos                                                                                                          =
//======================================================================================================================
void Mesh::createVbos()
{
	vbos.vertIndeces.create(GL_ELEMENT_ARRAY_BUFFER, vertIndeces.getSizeInBytes(), &vertIndeces[0], GL_STATIC_DRAW);
	vbos.vertCoords.create(GL_ARRAY_BUFFER, vertCoords.getSizeInBytes(), &vertCoords[0], GL_STATIC_DRAW);
	vbos.vertNormals.create(GL_ARRAY_BUFFER, vertNormals.getSizeInBytes(), &vertNormals[0], GL_STATIC_DRAW);
	if(vertTangents.size() > 1)
		vbos.vertTangents.create(GL_ARRAY_BUFFER, vertTangents.getSizeInBytes(), &vertTangents[0], GL_STATIC_DRAW);
	if(texCoords.size() > 1)
		vbos.texCoords.create(GL_ARRAY_BUFFER, texCoords.getSizeInBytes(), &texCoords[0], GL_STATIC_DRAW);
	if(vertWeights.size() > 1)
		vbos.vertWeights.create(GL_ARRAY_BUFFER, vertWeights.getSizeInBytes(), &vertWeights[0], GL_STATIC_DRAW);
}


//======================================================================================================================
// calcBSphere                                                                                                         =
//======================================================================================================================
void Mesh::calcBSphere()
{
	bsphere.Set(&vertCoords[0], 0, vertCoords.size());
}


//======================================================================================================================
// isRenderable                                                                                                        =
//======================================================================================================================
bool Mesh::isRenderable() const
{
	return material.get() != NULL;
}
