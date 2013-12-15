#include "Common.h"
#include <cassert>

//==============================================================================
static const aiMesh& getMesh(const Exporter& exporter, unsigned index)
{
	assert(index < exporter.scene->mNumMeshes);
	return *exporter.scene->mMeshes[index];
}

//==============================================================================
static const aiMaterial& getMaterial(const Exporter& exporter, unsigned index)
{
	assert(index < exporter.scene->mNumMaterials);
	return *exporter.scene->mMaterials[index];
}

//==============================================================================
static std::string getMeshName(const aiMesh& mesh)
{
	return std::string(mesh.mName.C_Str());
}

//==============================================================================
static std::string getMaterialName(const aiMaterial& mtl, bool instanced)
{
	aiString ainame;
	std::string name;
	if(mtl.Get(AI_MATKEY_NAME, ainame) == AI_SUCCESS)
	{
		name = ainame.C_Str();

		if(instanced)
		{
			name += "_inst";
		}
	}
	else
	{
		ERROR("Material's name is missing\n");
	}

	return name;
}

//==============================================================================
static std::string getModelName(const Exporter& exporter, const Model& model)
{
	std::string name = 
		getMeshName(getMesh(exporter, model.meshIndex)) + "_"
		+ getMaterialName(getMaterial(exporter, model.mtlIndex), 
			model.instanced);

	return name;
}

//==============================================================================
void exportMesh(
	const Exporter& exporter,
	const aiMesh& mesh, 
	const aiMatrix4x4* transform)
{
	std::string name = getMeshName(mesh);
	std::fstream file;
	LOGI("Exporting mesh %s\n", name.c_str());

	uint32_t vertsCount = mesh.mNumVertices;

	// Open file
	file.open(exporter.outDir + name + ".ankimesh",
		std::ios::out | std::ios::binary);

	// Write magic word
	file.write("ANKIMESH", 8);

	// Write the name
	uint32_t size = name.size();
	file.write((char*)&size, sizeof(uint32_t));
	file.write(&name[0], size);

	// Write positions
	file.write((char*)&vertsCount, sizeof(uint32_t));
	for(uint32_t i = 0; i < mesh.mNumVertices; i++)
	{
		aiVector3D pos = mesh.mVertices[i];

		// Transform
		if(transform)
		{
			pos = (*transform) * pos;
		}

		// flip 
		if(exporter.flipyz)
		{
			static const aiMatrix4x4 toLefthanded(
				1, 0, 0, 0, 
				0, 0, 1, 0, 
				0, -1, 0, 0, 
				0, 0, 0, 1);

			pos = toLefthanded * pos;
		}

		for(uint32_t j = 0; j < 3; j++)
		{
			file.write((char*)&pos[j], sizeof(float));
		}
	}

	// Write the indices
	file.write((char*)&mesh.mNumFaces, sizeof(uint32_t));
	for(uint32_t i = 0; i < mesh.mNumFaces; i++)
	{
		const aiFace& face = mesh.mFaces[i];
		
		if(face.mNumIndices != 3)
		{
			ERROR("For some reason the assimp didn't triangulate\n");
		}

		for(uint32_t j = 0; j < 3; j++)
		{
			uint32_t index = face.mIndices[j];
			file.write((char*)&index, sizeof(uint32_t));
		}
	}

	// Write the tex coords
	file.write((char*)&vertsCount, sizeof(uint32_t));

	// For all channels
	for(uint32_t ch = 0; ch < mesh.GetNumUVChannels(); ch++)
	{
		if(mesh.mNumUVComponents[ch] != 2)
		{
			ERROR("Incorrect number of UV components\n");
		}

		// For all tex coords of this channel
		for(uint32_t i = 0; i < vertsCount; i++)
		{
			aiVector3D texCoord = mesh.mTextureCoords[ch][i];

			for(uint32_t j = 0; j < 2; j++)
			{
				file.write((char*)&texCoord[j], sizeof(float));
			}
		}
	}

	// Write bone weigths count
	if(mesh.HasBones())
	{
#if 0
		// Write file
		file.write((char*)&vertsCount, sizeof(uint32_t));

		// Gather info for each vertex
		std::vector<Vw> vw;
		vw.resize(vertsCount);
		memset(&vw[0], 0, sizeof(Vw) * vertsCount);

		// For all bones
		for(uint32_t i = 0; i < mesh.mNumBones; i++)
		{
			const aiBone& bone = *mesh.mBones[i];

			// for every weights of the bone
			for(uint32_t j = 0; j < bone.mWeightsCount; j++)
			{
				const aiVertexWeight& weigth = bone.mWeights[j];

				// Sanity check
				if(weight.mVertexId >= vertCount)
				{
					ERROR("Out of bounds vert ID");
				}

				Vm& a = vm[weight.mVertexId];

				// Check out of bounds
				if(a.bonesCount >= MAX_BONES_PER_VERTEX)
				{
					LOGW("Too many bones for vertex %d\n", weigth.mVertexId);
					continue;
				}

				// Write to vertex
				a.boneIds[a.bonesCount] = i;
				a.weigths[a.bonesCount] = weigth.mWeigth;
				++a.bonesCount;
			}

			// Now write the file
		}
#endif
	}
	else
	{
		uint32_t num = 0;
		file.write((char*)&num, sizeof(uint32_t));
	}
}

//==============================================================================
void exportSkeleton(const Exporter& exporter, const aiMesh& mesh)
{
	assert(mesh.HasBones());
	std::string name = mesh.mName.C_Str();
	std::fstream file;
	LOGI("Exporting skeleton %s\n", name.c_str());

	// Open file
	file.open(exporter.outDir + name + ".skel", std::ios::out);
	
	file << XML_HEADER << "\n";
	file << "<skeleton>\n";
	file << "\t<bones>\n";

	bool rootBoneFound = false;

	for(uint32_t i = 0; i < mesh.mNumBones; i++)
	{
		const aiBone& bone = *mesh.mBones[i];

		file << "\t\t<bone>\n";

		// <name>
		file << "\t\t\t<name>" << bone.mName.C_Str() << "</name>\n";

		if(strcmp(bone.mName.C_Str(), "root") == 0)
		{
			rootBoneFound = true;
		}

		// <transform>
		file << "\t\t\t<transform>";
		for(uint32_t j = 0; j < 16; j++)
		{
			file << bone.mOffsetMatrix[j] << " ";
		}
		file << "</transform>\n";

		file << "\t\t</bone>\n";
	}

	if(!rootBoneFound)
	{
		ERROR("There should be one bone named \"root\"\n");
	}

	file << "\t</bones>\n";
	file << "</skeleton>\n";
}

//==============================================================================
void exportMaterial(
	const Exporter& exporter, 
	const aiMaterial& mtl, 
	bool instanced)
{
	std::string diffTex;
	std::string normTex;

	std::string name = getMaterialName(mtl, instanced);
	LOGI("Exporting material %s\n", name.c_str());

	// Diffuse texture
	if(mtl.GetTextureCount(aiTextureType_DIFFUSE) < 1)
	{
		ERROR("Material has no diffuse textures\n");
	}

	aiString path;
	if(mtl.GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
	{
		diffTex = getFilename(path.C_Str());
	}
	else
	{
		ERROR("Failed to retrieve texture\n");
	}

	// Normal texture
	if(mtl.GetTextureCount(aiTextureType_NORMALS) > 0)
	{	
		if(mtl.GetTexture(aiTextureType_NORMALS, 0, &path) == AI_SUCCESS)
		{
			normTex = getFilename(path.C_Str());
		}
		else
		{
			ERROR("Failed to retrieve texture\n");
		}
	}

	// Write file
	static const char* diffMtlStr = 
#include "diffTemplateMtl.h"
		;
	static const char* diffNormMtlStr = 
#include "diffNormTemplateMtl.h"
		;

	std::fstream file;
	file.open(exporter.outDir + name + ".ankimtl", std::ios::out);

	// Chose the correct template
	std::string str;
	if(normTex.size() == 0)
	{
		str = diffMtlStr;
	}
	else
	{
		str = replaceAllString(diffNormMtlStr, "%normalMap%", 
			exporter.texrpath + normTex);
	}

	str = replaceAllString(str, "%instanced%", (instanced) ? "1" : "0");
	str = replaceAllString(str, "%diffuseMap%", exporter.texrpath + diffTex);

	file << str;
}

//==============================================================================
void exportModel(const Exporter& exporter, const Model& model)
{
	std::string name = getModelName(exporter, model);
	LOGI("Exporting model %s\n", name.c_str());

	std::fstream file;
	file.open(exporter.outDir + name + ".ankimdl", std::ios::out);

	file << XML_HEADER << '\n';
	file << "<model>\n";
	file << "\t<modelPatches>\n";
	
	// start
	file << "\t\t<modelPatch>\n";

	// Write mesh
	file << "\t\t\t<mesh>" << exporter.rpath 
		<< getMeshName(getMesh(exporter, model.meshIndex)) 
		<< ".ankimesh</mesh>\n";

	// Write material
	file << "\t\t\t<material>" << exporter.rpath 
		<< getMaterialName(getMaterial(exporter, model.mtlIndex),
			model.instanced) 
		<< ".ankimtl</material>\n";

	// end
	file << "\t\t</modelPatch>\n";

	file << "\t</modelPatches>\n";
	file << "</model>\n";
}
