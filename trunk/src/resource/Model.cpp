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

		meshb.getVboInfo(attrib.id, vbo, size, type,
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
	meshb.getVboInfo(MeshBase::VA_INDICES, vbo, size, type,
			stride, offset);

	ANKI_ASSERT(vbo != nullptr);
	vao.attachElementArrayBufferVbo(vbo);
}

//==============================================================================
void ModelPatchBase::create()
{
	const Material& mtl = getMaterial();
	U32 meshMaxLod = getMeshesCount() - 1;
	U32 mtlMaxLod = mtl.getLevelsOfDetail() - 1;
	U32 maxLod = std::max(meshLods, mtlLods);
	U i = 0;
	U passesCount = mtl.getPasses().size();

	vaos.resize(maxLod * passesCount);

	for(U32 level = 0; level < maxLod + 1; ++level)
	{
		for(U32 pass = 0; pass < passesCount; ++pass)
		{
			PassLevelKey meshKey(pass, std::min(level, meshMaxLod));
			PassLevelKey mtlKey(pass, std::min(level, mtlMaxLod));

			Vao vao;
			createVao(mtl, getMeshBase(meshKey), mtlKey, vao);

			vaos[i] = std::move(vao);
			vaosMap[key] = &vaos[i];
			++i;
		}
	}
}

//==============================================================================
// ModelPatch                                                                  =
//==============================================================================

//==============================================================================
ModelPatch::ModelPatch(const char *meshFNames[], U32 meshesCount,
	const char* mtlFName)
{
	// Load
	ANKI_ASSERT(meshesCount > 0);
	meshes.resize(meshesCount);
	for(U32 i = 0; i < meshesCount; i++)
	{
		meshes[i].load(meshFNames[i]);
	}
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
			XmlElement meshEl1 = modelPatchEl.getChildElementOptional("mesh1");
			XmlElement meshEl2 = modelPatchEl.getChildElementOptional("mesh2");
			Array<const char*, 3> meshesFnames;
			U meshesCount = 1;

			meshesFnames[0] = meshEl.getText();

			if(meshEl1)
			{
				++meshesCount;
				meshesFnames[1] = meshEl1.getText();
			}

			if(meshEl2)
			{
				++meshesCount;
				meshesFnames[2] = meshEl2.getText();
			}

			XmlElement materialEl =
				modelPatchEl.getChildElement("material");

			ModelPatch* patch = new ModelPatch(
				&meshesFnames[0], meshesCount, materialEl.getText());
			modelPatches.push_back(patch);

			modelPatchEl = modelPatchEl.getNextSiblingElement("modelPatch");
		} while(modelPatchEl);

		// Check number of model patches
		if(modelPatches.size() < 1)
		{
			throw ANKI_EXCEPTION("Zero number of model patches");
		}

		// Calculate compound bounding volume
		PassLevelKey key;
		key.level = 0;
		visibilityShape = modelPatches[0]->getMeshBase(key).getBoundingShape();

		for(ModelPatchesContainer::const_iterator it = modelPatches.begin() + 1;
			it != modelPatches.end();
			++it)
		{
			visibilityShape = visibilityShape.getCompoundShape(
				(*it)->getMeshBase(key).getBoundingShape());
		}
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Model loading failed: " + filename) << e;
	}
}


} // end namespace
