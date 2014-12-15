// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

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
std::string getModelName(const Exporter& exporter, const Model& model)
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
	std::string specColTex;
	std::string shininessTex;
	std::string dispTex;

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

	// Specular color
	if(mtl.GetTextureCount(aiTextureType_SPECULAR) > 0)
	{
		if(mtl.GetTexture(aiTextureType_SPECULAR, 0, &path) == AI_SUCCESS)
		{
			specColTex = getFilename(path.C_Str());
		}
		else
		{
			ERROR("Failed to retrieve texture\n");
		}
	}

	// Shininess color
	if(mtl.GetTextureCount(aiTextureType_SHININESS) > 0)
	{
		if(mtl.GetTexture(aiTextureType_SHININESS, 0, &path) == AI_SUCCESS)
		{
			shininessTex = getFilename(path.C_Str());
		}
		else
		{
			ERROR("Failed to retrieve texture\n");
		}
	}

	// Height texture
	if(mtl.GetTextureCount(aiTextureType_EMISSIVE) > 0)
	{	
		if(mtl.GetTexture(aiTextureType_EMISSIVE, 0, &path) == AI_SUCCESS)
		{
			dispTex = getFilename(path.C_Str());
		}
		else
		{
			ERROR("Failed to retrieve texture\n");
		}
	}

	// Write file
	static const char* diffFragTemplate = 
#include "templates/diffFrag.h"
		;
	static const char* diffNormFragTemplate = 
#include "templates/diffNormFrag.h"
		;
	static const char* diffNormSpecFragTemplate = 
#include "templates/diffNormSpecFrag.h"
		;
	static const char* simpleVertTemplate = 
#include "templates/simpleVert.h"
		;
	static const char* tessVertTemplate = 
#include "templates/tessVert.h"
		;
	static const char* readRFromTextureTemplate = R"(
				<operation>
					<id>%id%</id>
					<returnType>float</returnType>
					<function>readRFromTexture</function>
					<arguments>
						<argument>%map%</argument>
						<argument>out2</argument>
					</arguments>
				</operation>)";

	// Compose full template
	// First geometry part
	std::string materialStr;
	materialStr = R"(<?xml version="1.0" encoding="UTF-8" ?>)";
	materialStr += "\n<material>\n\t<programs>\n";
	if(dispTex.empty())
	{
		materialStr += simpleVertTemplate;
	}
	else
	{
		materialStr += tessVertTemplate;
	}

	materialStr += "\n";

	// Then fragment part
	if(normTex.empty())
	{
		materialStr += diffFragTemplate;
	}
	else
	{
		materialStr += diffNormSpecFragTemplate;
	}

	materialStr += "\n\t</programs>\t</material>";

	// Replace strings
	if(!normTex.empty())
	{
		materialStr = replaceAllString(materialStr, "%normalMap%", 
			exporter.texrpath + normTex);
	}

	if(!dispTex.empty())
	{
		materialStr = replaceAllString(materialStr, "%dispMap%", 
			exporter.texrpath + dispTex);
	}

	if(!specColTex.empty())
	{
		materialStr = replaceAllString(materialStr, "%specularColorInput%", 
			R"(<input><type>sampler2D</type><name>uSpecularColor</name><value>)"
			+ exporter.texrpath + specColTex
			+ R"(</value></input>)");

		materialStr = replaceAllString(materialStr, "%specularColorFunc%", 
			readRFromTextureTemplate);

		materialStr = replaceAllString(materialStr, "%id%", 
			"50");

		materialStr = replaceAllString(materialStr, "%map%", 
			"uSpecularColor");

		materialStr = replaceAllString(materialStr, "%specularColorArg%", 
			"out50");
	}
	else
	{
		aiColor3D specCol = {0.0, 0.0, 0.0};
		mtl.Get(AI_MATKEY_COLOR_SPECULAR, specCol);

		materialStr = replaceAllString(materialStr, "%specularColorInput%", 
			R"(<input><type>float</type><name>uSpecularColor</name><value>)"
			+ std::to_string((specCol[0] + specCol[1] + specCol[2]) / 3.0)
			+ R"(</value></input>)");

		materialStr = replaceAllString(materialStr, "%specularColorFunc%", 
			"");

		materialStr = replaceAllString(materialStr, "%specularColorArg%", 
			"uSpecularColor");
	}

	if(!shininessTex.empty())
	{
		materialStr = replaceAllString(materialStr, "%specularPowerInput%", 
			R"(<input><type>sampler2D</type><name>uSpecularPower</name><value>)"
			+ exporter.texrpath + shininessTex
			+ R"(</value></input>)" + "\n"
			+ "\t\t\t\t" 
			+ R"(<input><type>float</type><name>MAX_SPECULAR_POWER</name>)"
			+ R"(<value>120.0</value><const>1</const></input>)");

		materialStr = replaceAllString(materialStr, "%specularPowerValue%", 
			exporter.texrpath + shininessTex);

			static const char* multiply = R"(
				<operation>
					<id>61</id>
					<returnType>float</returnType>
					<function>mul</function>
					<arguments>
						<argument>MAX_SPECULAR_POWER</argument>
						<argument>out60</argument>
					</arguments>
				</operation>)";

		std::string func = std::string(readRFromTextureTemplate) + multiply;

		materialStr = replaceAllString(materialStr, "%specularPowerFunc%", 
			func);

		materialStr = replaceAllString(materialStr, "%id%", 
			"60");

		materialStr = replaceAllString(materialStr, "%map%", 
			"uSpecularPower");

		materialStr = replaceAllString(materialStr, "%specularPowerArg%", 
			"out61");
	}
	else
	{
		float shininess = 0.0;
		mtl.Get(AI_MATKEY_SHININESS, shininess);
		materialStr = replaceAllString(materialStr, "%specularPowerInput%", 
			R"(<input><type>float</type><name>uSpecularPower</name><value>)"
			+ std::to_string(shininess)
			+ R"(</value></input>)");

		materialStr = replaceAllString(materialStr, "%specularPowerFunc%", 
			"");

		materialStr = replaceAllString(materialStr, "%specularPowerArg%", 
			"uSpecularPower");
	}

	materialStr = replaceAllString(materialStr, "%maxSpecularPower%", " ");

	materialStr = replaceAllString(materialStr, "%instanced%", 
		(instanced) ? "1" : "0");
	materialStr = replaceAllString(materialStr, "%diffuseMap%", 
		exporter.texrpath + diffTex);

	// Replace texture extensions with .anki
	materialStr = replaceAllString(materialStr, ".tga", ".ankitex");
	materialStr = replaceAllString(materialStr, ".png", ".ankitex");

	// Open and write file
	std::fstream file;
	file.open(exporter.outDir + name + ".ankimtl", std::ios::out);
	file << materialStr;
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
