#include "anki/resource/Model.h"
#include "anki/resource/Material.h"
#include "anki/resource/Mesh.h"
#include "anki/resource/SkelAnim.h"
#include "anki/resource/MeshLoader.h"
#include "anki/resource/Skeleton.h"
#include "anki/resource/ShaderProgramResource.h"
#include "anki/misc/Xml.h"

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

	attrib = prog.tryFindAttributeVariable("position");
	if(attrib)
	{
		const Vbo* vbo = meshb.getVbo(Mesh::VBO_POSITIONS);
		ANKI_ASSERT(vbo != nullptr);

		vao->attachArrayBufferVbo(*vbo, *attrib, 3, GL_FLOAT, GL_FALSE, 0,
			nullptr);
	}

	attrib = prog.tryFindAttributeVariable("normal");
	if(attrib)
	{
		const Vbo* vbo = meshb.getVbo(Mesh::VBO_NORMALS);
		ANKI_ASSERT(vbo != nullptr);

		vao->attachArrayBufferVbo(*vbo, *attrib, 3, GL_FLOAT, GL_FALSE, 0,
			nullptr);
	}

	attrib = prog.tryFindAttributeVariable("tangent");
	if(attrib)
	{
		const Vbo* vbo = meshb.getVbo(Mesh::VBO_TANGENTS);
		ANKI_ASSERT(vbo != nullptr);

		vao->attachArrayBufferVbo(*vbo, *attrib, 4, GL_FLOAT, GL_FALSE, 0,
			nullptr);
	}

	attrib = prog.tryFindAttributeVariable("texCoords");
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
		// Load
		//
		XmlDocument doc;
		doc.loadFile(filename);

		XmlElement rootEl = doc.getChildElement("model");

		// modelPatches
		XmlElement modelPatchesEl =
			rootEl.getChildElement("modelPatches");
		XmlElement modelPatchEl =
			modelPatchesEl.getChildElement("modelPatch");
		do
		{
			XmlElement meshEl = modelPatchEl.getChildElement("mesh");
			XmlElement materialEl =
				modelPatchEl.getChildElement("material");

			ModelPatch* patch = new ModelPatch(meshEl.getText(),
				materialEl.getText());
			modelPatches.push_back(patch);

			modelPatchEl = modelPatchEl.getNextSiblingElement("modelPatch");
		} while(modelPatchEl);

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
