#include "Mesh.h"
#include "Renderer.h"
#include "Resource.h"
#include "Scanner.h"
#include "Parser.h"
#include "Material.h"


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
bool Mesh::load(const char* filename)
{
	Scanner scanner;
	if(!scanner.loadFile(filename))
		return false;

	const Scanner::Token* token;

	/*
	 * Material
	 */
	token = &scanner.getNextToken();
	if(token->getCode() != Scanner::TC_STRING)
	{
		PARSE_ERR_EXPECTED("string");
		return false;
	}
	if(strlen(token->getValue().getString()) > 0)
		material.loadRsrc(token->getValue().getString());

	/*
	 * Verts
	 */
	// verts num
	token = &scanner.getNextToken();
	if(token->getCode() != Scanner::TC_NUMBER || token->getDataType() != Scanner::DT_INT)
	{
		PARSE_ERR_EXPECTED("integer");
		return false;
	}
	vertCoords.resize(token->getValue().getInt());

	// read the verts
	for(uint i=0; i<vertCoords.size(); i++)
	{
		if(!Parser::parseArrOfNumbers<float>(scanner, false, true, 3, &vertCoords[i][0]))
			return false;
	}

	/*
	 * Faces
	 */
	// faces num
	token = &scanner.getNextToken();
	if(token->getCode() != Scanner::TC_NUMBER || token->getDataType() != Scanner::DT_INT)
	{
		PARSE_ERR_EXPECTED("integer");
		return false;
	}
	tris.resize(token->getValue().getInt());
	// read the faces
	for(uint i=0; i<tris.size(); i++)
	{
		if(!Parser::parseArrOfNumbers<uint>(scanner, false, true, 3, tris[i].vertIds))
			return false;
	}

	/*
	 * Tex coords
	 */
	// UVs num
	token = &scanner.getNextToken();
	if(token->getCode() != Scanner::TC_NUMBER || token->getDataType() != Scanner::DT_INT)
	{
		PARSE_ERR_EXPECTED("integer");
		return false;
	}
	texCoords.resize(token->getValue().getInt());
	// read the texCoords
	for(uint i=0; i<texCoords.size(); i++)
	{
		if(!Parser::parseArrOfNumbers(scanner, false, true, 2, &texCoords[i][0]))
			return false;
	}

	/*
	 * Vert weights
	 */
	token = &scanner.getNextToken();
	if(token->getCode() != Scanner::TC_NUMBER || token->getDataType() != Scanner::DT_INT)
	{
		PARSE_ERR_EXPECTED("integer");
		return false;
	}
	vertWeights.resize(token->getValue().getInt());
	for(uint i=0; i<vertWeights.size(); i++)
	{
		// get the bone connections num
		token = &scanner.getNextToken();
		if(token->getCode() != Scanner::TC_NUMBER || token->getDataType() != Scanner::DT_INT)
		{
			PARSE_ERR_EXPECTED("integer");
			return false;
		}

		// we treat as error if one vert doesnt have a bone
		if(token->getValue().getInt() < 1)
		{
			ERROR("Vert \"" << i << "\" doesnt have at least one bone");
			return false;
		}

		// and here is another possible error
		if(token->getValue().getInt() > VertexWeight::MAX_BONES_PER_VERT)
		{
			ERROR("Cannot have more than " << VertexWeight::MAX_BONES_PER_VERT << " bones per vertex");
			return false;
		}
		vertWeights[i].bonesNum = token->getValue().getInt();

		// for all the weights of the current vertes
		for(uint j=0; j<vertWeights[i].bonesNum; j++)
		{
			// read bone id
			token = &scanner.getNextToken();
			if(token->getCode() != Scanner::TC_NUMBER || token->getDataType() != Scanner::DT_INT)
			{
				PARSE_ERR_EXPECTED("integer");
				return false;
			}
			vertWeights[i].boneIds[j] = token->getValue().getInt();

			// read the weight of that bone
			token = &scanner.getNextToken();
			if(token->getCode() != Scanner::TC_NUMBER || token->getDataType() != Scanner::DT_FLOAT)
			{
				PARSE_ERR_EXPECTED("float");
				return false;
			}
			vertWeights[i].weights[j] = token->getValue().getFloat();
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
			WARNING(getRsrcName() << ": det == " << fixed << det);
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
