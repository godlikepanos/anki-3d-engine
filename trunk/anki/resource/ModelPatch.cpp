#include "anki/resource/ModelPatch.h"
#include "anki/resource/Mesh.h"
#include "anki/resource/Material.h"


namespace anki {


//==============================================================================
ModelPatch::ModelPatch(const char* meshFName, const char* mtlFName)
{
	// Load
	mesh.load(meshFName);
	mtl.load(mtlFName);

	// Create VAOs
	VboArray vboArr;
	for(uint i = 0; i < Mesh::VBOS_NUM; i++)
	{
		vboArr[i] = &mesh->getVbo((Mesh::Vbos)i);
	}

	createVaos(*mtl, vboArr, vaos, vaosHashMap);
}


//==============================================================================
ModelPatch::~ModelPatch()
{}


//==============================================================================
bool ModelPatch::supportsHwSkinning() const
{
	return mesh->hasVertWeights();
}


//==============================================================================
Vao* ModelPatch::createVao(const Material& mtl,
	const VboArray& vbos,
	const PassLevelKey& key)
{
	Vao* vao = new Vao;

	if(mtl.variableExistsAndInKey("position", key))
	{
		ANKI_ASSERT(vbos[Mesh::VBO_VERT_POSITIONS] != NULL);

		vao->attachArrayBufferVbo(*vbos[Mesh::VBO_VERT_POSITIONS],
			0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	if(mtl.variableExistsAndInKey("normal", key))
	{
		ANKI_ASSERT(vbos[Mesh::VBO_VERT_NORMALS] != NULL);

		vao->attachArrayBufferVbo(*vbos[Mesh::VBO_VERT_NORMALS],
			1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	if(mtl.variableExistsAndInKey("tangent", key))
	{
		ANKI_ASSERT(vbos[Mesh::VBO_VERT_TANGENTS] != NULL);

		vao->attachArrayBufferVbo(*vbos[Mesh::VBO_VERT_TANGENTS],
			2, 4, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	if(mtl.variableExistsAndInKey("texCoords", key))
	{
		vao->attachArrayBufferVbo(*vbos[Mesh::VBO_TEX_COORDS],
			3, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	vao->attachElementArrayBufferVbo(*vbos[Mesh::VBO_VERT_INDECES]);

	return vao;
}


//==============================================================================
void ModelPatch::createVaos(const Material& mtl,
	const VboArray& vbos,
	VaosContainer& vaos,
	PassLevelToVaoHashMap& vaosHashMap)
{
	for(uint level = 0; level < mtl.getLevelsOfDetail(); ++level)
	{
		for(uint pass = 0; pass < mtl.getPasses().size(); ++pass)
		{
			PassLevelKey key(pass, level);

			Vao* vao = createVao(mtl, vbos, key);

			vaos.push_back(vao);
			vaosHashMap[key] = vao;
		}
	}


}


} // end namespace
