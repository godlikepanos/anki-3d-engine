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
void ModelPatchBase::createVao(const ShaderProgram& prog,
	const MeshBase& meshb, Vao& vao)
{
	vao.create();

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
void ModelPatchBase::getRenderingData(const PassLevelKey& key, const Vao*& vao,
	const ShaderProgram*& prog, U32& indicesCount) const
{
	const U meshLods = getMeshesCount();
	ANKI_ASSERT(meshLods > 0);
	const U mtlLods = getMaterial().getLevelsOfDetail();
	ANKI_ASSERT(mtlLods > 0);

	// VAO
	U lodsCount = std::max(meshLods, mtlLods);

	U index = key.pass + std::min((U)key.level, lodsCount - 1) * lodsCount;

	ANKI_ASSERT(index < modelPatchProtected.vaos.size());
	vao = &modelPatchProtected.vaos[index];

	// Mesh and indices
	PassLevelKey meshKey;
	meshKey.pass = key.pass;
	meshKey.level = std::min(key.level, (U8)(meshLods - 1));

	const MeshBase& meshBase = getMeshBase(meshKey);
	indicesCount = meshBase.getIndicesCount();

	// Prog
	PassLevelKey mtlKey;
	mtlKey.pass = key.pass;
	mtlKey.level = std::min(key.level, (U8)(mtlLods - 1));

	prog = &getMaterial().findShaderProgram(mtlKey);
}

//==============================================================================
void ModelPatchBase::getRenderingDataSub(const PassLevelKey& key,
	U64 subMeshesMask, const Vao*& vao, const ShaderProgram*& prog,
	U32* indicesCountArray, U32* indicesOffsetArray, U32& primcount) const
{
	const U meshLods = getMeshesCount();
	ANKI_ASSERT(meshLods > 0);
	const U mtlLods = getMaterial().getLevelsOfDetail();
	ANKI_ASSERT(mtlLods > 0);

	// VAO
	U lodsCount = std::max(meshLods, mtlLods);

	U index = key.pass + std::min((U)key.level, lodsCount - 1) * lodsCount;

	ANKI_ASSERT(index < modelPatchProtected.vaos.size());
	vao = &modelPatchProtected.vaos[index];

	// Prog
	PassLevelKey mtlKey;
	mtlKey.pass = key.pass;
	mtlKey.level = std::min(key.level, (U8)(mtlLods - 1));

	prog = &getMaterial().findShaderProgram(mtlKey);

	// Mesh and indices
	PassLevelKey meshKey;
	meshKey.pass = key.pass;
	meshKey.level = std::min(key.level, (U8)(meshLods - 1));

	const MeshBase& meshBase = getMeshBase(meshKey);

	U subMeshesCount = meshBase.getSubMeshesCount();
	primcount = 0;
	for(U i = 0; i < subMeshesCount; i++)
	{
		if(subMeshesMask & (1 << i))
		{
			indicesCountArray[primcount] =
				meshBase.getIndicesCountSub(i, indicesOffsetArray[primcount]);
			++primcount;
		}
	}
}

//==============================================================================
void ModelPatchBase::create()
{
	U i = 0;
	const Material& mtl = getMaterial();
	U lodsCount = std::max(getMeshesCount(), mtl.getLevelsOfDetail());
	U passesCount = mtl.getPasses().size();

	modelPatchProtected.vaos.resize(lodsCount * passesCount);

	for(U lod = 0; lod < lodsCount; ++lod)
	{
		for(U pass = 0; pass < passesCount; ++pass)
		{
			PassLevelKey key(pass, lod);
			const ShaderProgram* prog;
			const MeshBase* mesh;

			// Get mesh
			ANKI_ASSERT(getMeshesCount() > 0);
			PassLevelKey meshKey = key;
			meshKey.level = std::min(key.level, (U8)(getMeshesCount() - 1));
			mesh = &getMeshBase(meshKey);

			// Get shader prog
			ANKI_ASSERT(getMaterial().getLevelsOfDetail() > 0);
			PassLevelKey shaderKey = key;
			shaderKey.level = std::min(key.level,
				(U8)(getMaterial().getLevelsOfDetail() - 1));
			prog = &getMaterial().findShaderProgram(shaderKey);

			Vao vao;
			createVao(*prog, *mesh, vao);

			modelPatchProtected.vaos[i] = std::move(vao);
			++i;
		}
	}
}

//==============================================================================
// Model                                                                       =
//==============================================================================

//==============================================================================
Model::~Model()
{
	for(ModelPatchBase* patch : modelPatches)
	{
		delete patch;
	}
}

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
			XmlElement materialEl =
			modelPatchEl.getChildElement("material");

			Array<const char*, 3> meshesFnames;
			U meshesCount = 1;
			ModelPatchBase* patch;

			// Try mesh
			XmlElement meshEl = modelPatchEl.getChildElementOptional("mesh");
			if(meshEl)
			{
				XmlElement meshEl1 =
					modelPatchEl.getChildElementOptional("mesh1");
				XmlElement meshEl2 =
					modelPatchEl.getChildElementOptional("mesh2");

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

				patch = new ModelPatch<MeshResourcePointer>(
					&meshesFnames[0], meshesCount, materialEl.getText());
			}
			else
			{
				XmlElement bmeshEl =
					modelPatchEl.getChildElement("bucketMesh");
				XmlElement bmeshEl1 =
					modelPatchEl.getChildElementOptional("bucketMesh1");
				XmlElement bmeshEl2 =
					modelPatchEl.getChildElementOptional("bucketMesh2");

				meshesFnames[0] = bmeshEl.getText();

				if(bmeshEl1)
				{
					++meshesCount;
					meshesFnames[1] = bmeshEl1.getText();
				}

				if(bmeshEl2)
				{
					++meshesCount;
					meshesFnames[2] = bmeshEl2.getText();
				}

				patch = new ModelPatch<BucketMeshResourcePointer>(
					&meshesFnames[0], meshesCount, materialEl.getText());
			}

			modelPatches.push_back(patch);

			// Move to next
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
