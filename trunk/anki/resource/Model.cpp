#include "anki/resource/Model.h"
#include "anki/resource/Material.h"
#include "anki/resource/Mesh.h"
#include "anki/resource/SkelAnim.h"
#include "anki/resource/MeshData.h"
#include "anki/resource/Skeleton.h"
#include "anki/resource/ShaderProgram.h"
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/assign.hpp>
#include <boost/range/iterator_range.hpp>


namespace anki {


//==============================================================================
// ModelPatch                                                                  =
//==============================================================================

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
	vao->create();

	if(mtl.getShaderProgram(key).attributeVariableExists("position"))
	{
		ANKI_ASSERT(vbos[Mesh::VBO_VERT_POSITIONS] != NULL);

		vao->attachArrayBufferVbo(*vbos[Mesh::VBO_VERT_POSITIONS],
			0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	if(mtl.getShaderProgram(key).attributeVariableExists("normal"))
	{
		ANKI_ASSERT(vbos[Mesh::VBO_VERT_NORMALS] != NULL);

		vao->attachArrayBufferVbo(*vbos[Mesh::VBO_VERT_NORMALS],
			1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	if(mtl.getShaderProgram(key).attributeVariableExists("tangent"))
	{
		ANKI_ASSERT(vbos[Mesh::VBO_VERT_TANGENTS] != NULL);

		vao->attachArrayBufferVbo(*vbos[Mesh::VBO_VERT_TANGENTS],
			2, 4, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	if(mtl.getShaderProgram(key).attributeVariableExists("texCoords"))
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


//==============================================================================
// Model                                                                       =
//==============================================================================

//==============================================================================
void Model::load(const char* filename)
{
	try
	{
		//
		// Load
		//
		using namespace boost::property_tree;
		ptree pt_;
		read_xml(filename, pt_);

		const ptree& pt = pt_.get_child("model");

		// modelPatches
		BOOST_FOREACH(const ptree::value_type& v, pt.get_child("modelPatches"))
		{
			const std::string& mesh = v.second.get<std::string>("mesh");
			const std::string& material = v.second.get<std::string>("material");

			ModelPatch* patch = new ModelPatch(mesh.c_str(), material.c_str());
			modelPatches.push_back(patch);
		}

		if(modelPatches.size() < 1)
		{
			throw ANKI_EXCEPTION("Zero number of model patches");
		}

		// Calculate compound bounding volume
		visibilityShape = modelPatches[0].getMesh().getVisibilityShape();

		for(ModelPatchesContainer::const_iterator it = modelPatches.begin() + 1;
			it != modelPatches.end();
			++it)
		{
			visibilityShape = visibilityShape.getCompoundShape(
				(*it).getMesh().getVisibilityShape());
		}
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Model \"" + filename + "\"") << e;
	}
}


} // end namespace
