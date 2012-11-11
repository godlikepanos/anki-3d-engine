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

struct Attrib
{
	const char* name;
	MeshBase::VertexAttribute id;
};

static const Array<Attrib, MeshBase::VA_COUNT - 1> attribs = {{
	{"position", MeshBase::VA_POSITION},
	{"normal", MeshBase::VA_NORMAL},
	{"tangent", MeshBase::VA_TANGENT},
	{"texCoords", MeshBase::VA_TEXTURE_COORDS},
	{"texCoords1", MeshBase::VA_TEXTURE_COORDS_1},
	{"bonesCount", MeshBase::VA_BONE_COUNT},
	{"boneIds", MeshBase::VA_BONE_IDS},
	{"boneWeights", MeshBase::VA_BONE_WEIGHTS}
}};

//==============================================================================
void ModelPatchBase::createVao(const Material& mtl,const MeshBase& meshb,
	const PassLevelKey& key, Vao& vao)
{
	vao.create();

	const ShaderProgram& prog = mtl.findShaderProgram(key);

	const Vbo* vbo;
	U32 size;
	GLenum type;
	U32 stride;
	U32 offset;

	for(const Attrib& attrib : attribs)
	{
		const ShaderProgramAttributeVariable* attr = 
			prog.tryFindAttributeVariable(attrib.name);

		if(attr == nullptr)
		{
			continue;
		}

		meshb.getVboInfo(attrib.id, (U32)key.level, vbo, size, type,
			stride, offset);

		if(vbo == nullptr)
		{
			throw ANKI_EXCEPTION("Material asks for attribute that the mesh "
				"does not have: " + attrib.name);
		}

		vao.attachArrayBufferVbo(vbo, *attr, size, type, false, stride,
			offset);
	}

	// The indices VBO
	meshb.getVboInfo(MeshBase::VA_INDICES, key.level, vbo, size, type,
			stride, offset);

	if(vbo == nullptr)
	{
		// The desired LOD was not found. Use the 0
		meshb.getVboInfo(MeshBase::VA_INDICES, 0, vbo, size, type,
			stride, offset);
	}

	ANKI_ASSERT(vbo != nullptr);
	vao.attachElementArrayBufferVbo(vbo);
}

//==============================================================================
void ModelPatchBase::createVaos(const Material& mtl,
	const MeshBase& meshb,
	VaosContainer& vaos,
	PassLevelToVaoMap& vaosMap)
{
	vaos.resize(mtl.getLevelsOfDetail() * mtl.getPasses().size());

	for(U32 level = 0; level < mtl.getLevelsOfDetail(); ++level)
	{
		for(U32 pass = 0; pass < mtl.getPasses().size(); ++pass)
		{
			PassLevelKey key(pass, level);

			Vao vao;
			createVao(mtl, meshb, key, vao);

			U index = level * mtl.getPasses().size() + pass;
			vaos[index] = std::move(vao);
			vaosMap[key] = &vaos[index];
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
		visibilityShape = modelPatches[0]->getMeshBase().getBoundingShape();

		for(ModelPatchesContainer::const_iterator it = modelPatches.begin() + 1;
			it != modelPatches.end();
			++it)
		{
			visibilityShape = visibilityShape.getCompoundShape(
				(*it)->getMeshBase().getBoundingShape());
		}
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Model loading failed: " + filename) << e;
	}
}


} // end namespace
