#include "anki/resource/Model.h"
#include "anki/resource/Material.h"
#include "anki/resource/Mesh.h"
#include "anki/resource/SkelAnim.h"
#include "anki/resource/MeshLoader.h"
#include "anki/resource/Skeleton.h"
#include "anki/resource/ShaderProgramResource.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace anki {

//==============================================================================
// ModelPatchBase                                                              =
//==============================================================================

//==============================================================================
Vao* ModelPatchBase::createNewVao(const Material& mtl,
	const MeshBase& meshb,
	const PassLevelKey& key)
{
	Vao* vao = new Vao;
	vao->create();

	const ShaderProgramAttributeVariable* attrib;
	const ShaderProgram& prog = mtl.findShaderProgram(key);

	attrib = prog.findAttributeVariableByName("position");
	if(attrib)
	{
		const Vbo* vbo = meshb.getVbo(Mesh::VBO_POSITIONS);
		ANKI_ASSERT(vbo != nullptr);

		vao->attachArrayBufferVbo(*vbo, *attrib, 3, GL_FLOAT, GL_FALSE, 0,
			nullptr);
	}

	attrib = prog.findAttributeVariableByName("normal");
	if(attrib)
	{
		const Vbo* vbo = meshb.getVbo(Mesh::VBO_NORMALS);
		ANKI_ASSERT(vbo != nullptr);

		vao->attachArrayBufferVbo(*vbo, *attrib, 3, GL_FLOAT, GL_FALSE, 0,
			nullptr);
	}

	attrib = prog.findAttributeVariableByName("tangent");
	if(attrib)
	{
		const Vbo* vbo = meshb.getVbo(Mesh::VBO_TANGENTS);
		ANKI_ASSERT(vbo != nullptr);

		vao->attachArrayBufferVbo(*vbo, *attrib, 4, GL_FLOAT, GL_FALSE, 0,
			nullptr);
	}

	attrib = prog.findAttributeVariableByName("texCoords");
	if(attrib)
	{
		const Vbo* vbo = meshb.getVbo(Mesh::VBO_TEX_COORDS);
		ANKI_ASSERT(vbo != nullptr);

		vao->attachArrayBufferVbo(*vbo, *attrib, 2, GL_FLOAT, GL_FALSE, 0,
			nullptr);
	}

	vao->attachElementArrayBufferVbo(*meshb.getVbo(Mesh::VBO_INDICES));

	return vao;
}

//==============================================================================
void ModelPatchBase::createVaos(const Material& mtl,
	const MeshBase& meshb,
	VaosContainer& vaos,
	PassLevelToVaoMap& vaosMap)
{
	for(uint level = 0; level < mtl.getLevelsOfDetail(); ++level)
	{
		for(uint pass = 0; pass < mtl.getPasses().size(); ++pass)
		{
			PassLevelKey key(pass, level);

			Vao* vao = createNewVao(mtl, meshb, key);

			vaos.push_back(vao);
			vaosMap[key] = vao;
		}
	}
}

//==============================================================================
// ModelPatch                                                                  =
//==============================================================================

//==============================================================================
ModelPatch::ModelPatch(const char* meshFName, const char* mtlFName)
{
	// Load
	mesh.load(meshFName);
	mtl.load(mtlFName);

	/// Create VAOs
	create();
}

//==============================================================================
ModelPatch::~ModelPatch()
{}

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
		for(const ptree::value_type& v : pt.get_child("modelPatches"))
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
		visibilityShape = modelPatches[0].getMeshBase().getBoundingShape();

		for(ModelPatchesContainer::const_iterator it = modelPatches.begin() + 1;
			it != modelPatches.end();
			++it)
		{
			visibilityShape = visibilityShape.getCompoundShape(
				(*it).getMeshBase().getBoundingShape());
		}
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Model loading failed: " + filename) << e;
	}
}


} // end namespace
