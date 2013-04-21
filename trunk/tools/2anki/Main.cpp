#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <stdexcept>
#include <cstdarg>
#include <fstream>
#include <cstdint>
#include <sstream>
#include <cassert>

static const char* xmlHeader = R"(<?xml version="1.0" encoding="UTF-8" ?>)";

//==============================================================================
// Log and errors

#define STR(s) #s
#define XSTR(s) STR(s)
#define LOGI(...) \
	printf("[I] (" __FILE__ ":" XSTR(__LINE__) ") " __VA_ARGS__)

#define ERROR(...) \
	do { \
		fprintf(stderr, "[E] (" __FILE__ ":" XSTR(__LINE__) ") " __VA_ARGS__); \
		exit(0); \
	} while(0)

#define LOGW(...) \
	fprintf(stderr, "[W] (" __FILE__ ":" XSTR(__LINE__) ") " __VA_ARGS__)

//==============================================================================
std::string replaceAllString(const std::string& str, const std::string& from, 
	const std::string& to)
{
	if(from.empty())
	{
		return str;
	}

	std::string out = str;
	size_t start_pos = 0;
	while((start_pos = out.find(from, start_pos)) != std::string::npos) 
	{
		out.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}

	return out;
}

//==============================================================================
struct Config
{
	std::string inputFname;
	std::string outDir;
	std::string texturesAppend;
	std::string rpath;
	bool flipyz = false;
};

//==============================================================================
void parseConfig(int argc, char** argv, Config& config)
{
	static const char* usage = R"(Usage: 2anki in_file out_dir [options]
Options:
-texpath <string>: Append a string to the paths of textures
-rpath <string>:   Append a string to the meshes and materials
-flipyz:           Flip y with z
)";

	// Parse config
	if(argc < 3)
	{
		goto error;
	}

	config.inputFname = argv[1];
	config.outDir = argv[2] + std::string("/");

	for(int i = 3; i < argc; i++)
	{
		if(strcmp(argv[i], "-texpath") == 0)
		{
			++i;

			if(i < argc)
			{
				config.texturesAppend = argv[i] + std::string("/");
			}
			else
			{
				goto error;
			}
		}
		else if(strcmp(argv[i], "-rpath") == 0)
		{
			++i;

			if(i < argc)
			{
				config.rpath = argv[i] + std::string("/");
			}
			else
			{
				goto error;
			}
		}
		else if(strcmp(argv[i], "-flipyz") == 0)
		{
			config.flipyz = true;
		}
		else
		{
			goto error;
		}
	}

	return;

error:
	printf("%s", usage);
	exit(0);
}

//==============================================================================
/// Load the scene
const aiScene& load(const std::string& filename, Assimp::Importer& importer)
{
	LOGI("Loading file %s\n", filename.c_str());

	const aiScene* scene = importer.ReadFile(filename,
		aiProcess_Triangulate
		| aiProcess_JoinIdenticalVertices
		| aiProcess_SortByPType
		| aiProcess_ImproveCacheLocality
		| aiProcess_OptimizeMeshes
		//| aiProcess_CalcTangentSpace
		);

	if(!scene)
	{
		ERROR("%s\n", importer.GetErrorString());
	}

	LOGI("File loaded successfully!\n");
	return *scene;
}

//==============================================================================
static const uint32_t MAX_BONES_PER_VERTEX = 4;

/// Bone/weight info per vertex
struct Vw
{
	uint32_t boneIds[MAX_BONES_PER_VERTEX];
	float weigths[MAX_BONES_PER_VERTEX];
	uint32_t bonesCount;
};

//==============================================================================
void exportMesh(const aiMesh& mesh, const Config& config)
{
	std::string name = mesh.mName.C_Str();
	std::fstream file;
	LOGI("Exporting mesh %s\n", name.c_str());

	uint32_t vertsCount = mesh.mNumVertices;

	// Open file
	file.open(config.outDir + name + ".mesh",
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

		// flip
		if(config.flipyz)
		{
			float tmp = pos[1];
			pos[1] = pos[2];
			pos[2] = -tmp;
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
			ERROR("For some reason the assimp didn't triangulate");
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
			ERROR("Incorrect number of UV components");
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
void exportMaterial(const aiMaterial& mtl, const Config& config, 
	uint32_t id)
{
	std::string diffTex;
	std::string normTex;

	// Find the name
	aiString ainame;
	std::string name;
	if(mtl.Get(AI_MATKEY_NAME, ainame) == AI_SUCCESS)
	{
		name = ainame.C_Str();
	}
	else
	{
		ERROR("Material's name is missing\n");
	}
	

	LOGI("Exporting material %s\n", name.c_str());

	// Diffuse texture
	if(mtl.GetTextureCount(aiTextureType_DIFFUSE) < 1)
	{
		ERROR("Material has no diffuse textures");
	}

	aiString path;
	if(mtl.GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
	{
		diffTex = path.C_Str();
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
			normTex = path.C_Str();
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
	file.open(config.outDir + name + ".mtl", std::ios::out);

	// Chose the correct template
	if(normTex.size() == 0)
	{
		file << replaceAllString(diffMtlStr, "%diffuseMap%", 
			config.texturesAppend + diffTex);
	}
	else
	{
		std::string str;
		
		str = replaceAllString(diffNormMtlStr, "%diffuseMap%", 
			config.texturesAppend + diffTex);
		str = replaceAllString(str, "%normalMap%", 
			config.texturesAppend + normTex);

		file << str;
	}
}

//==============================================================================
void exportLight(const aiLight& light, const Config& config, std::fstream& file)
{
	if(light.mType != aiLightSource_POINT || light.mType != aiLightSource_SPOT)
	{
		LOGW("Skipping light %s. Unsupported type\n", light.mName.C_Str());
		return;
	}

	file << "\t<light>\n";

	file << "\t\t<name>" << light.mName.C_Str() << "</name>\n";

	file << "\t\t<diffuseColor>" 
		<< light.mColorDiffuse[0] << " " 
		<< light.mColorDiffuse[1] << " " 
		<< light.mColorDiffuse[2] << " " 
		<< light.mColorDiffuse[3]
		<< "</diffuseColor>\n";

	file << "\t\t<specularColor>" 
		<< light.mColorSpecular[0] << " " 
		<< light.mColorSpecular[1] << " " 
		<< light.mColorSpecular[2] << " " 
		<< light.mColorSpecular[3]
		<< "</specularColor>\n";

	aiMatrix4x4 trf;
	aiMatrix4x4::Translation(light.mPosition, trf);

	switch(light.mType)
	{
	case aiLightSource_POINT:
		{
			file << "\t\t<type>point</type>\n";

			// At this point I want the radius and have the attenuation factors
			// att = Ac + Al*d + Aq*d^2. When d = r then att = 0.0. Also if we 
			// assume that Ac is 0 then:
			// 0 = Al*r + Aq*r^2. Solving by r is easy
			float r = -light.mAttenuationLinear / light.mAttenuationQuadratic;
			file << "\t\t<radius>" << r << "</radius>\n";
			break;
		}
	case aiLightSource_SPOT:
		file << "\t\t<type>spot</type>\n";
		break;
	default:
		assert(0);
		break;
	}

	// <transform>
	file << "\t\t<transform>";
	for(uint32_t i = 0; i < 16; i++)
	{
		file << trf[i] << " ";
	}
	file << "</transform>\n";

	file << "\t</light>\n";
}

//==============================================================================
void exportModel(const aiScene& scene, const aiNode& node, const Config& config)
{
	if(node.mNumMeshes == 0)
	{
		return;
	}

	std::string name = node.mName.C_Str();

	LOGI("Exporting model %s\n", name.c_str());

	std::fstream file;
	file.open(config.outDir + name + ".mdl", std::ios::out);

	file << xmlHeader << '\n';
	file << "<model>\n";
	file << "\t<modelPatches>\n";

	for(uint32_t i = 0; i < node.mNumMeshes; i++)
	{
		uint32_t meshIndex = node.mMeshes[i];
		const aiMesh& mesh = *scene.mMeshes[meshIndex];
	
		// start
		file << "\t\t<modelPatch>\n";

		// Write mesh
		file << "\t\t\t<mesh>" << config.rpath 
			<< mesh.mName.C_Str() << ".mesh</mesh>\n";

		// Write material
		const aiMaterial& mtl = *scene.mMaterials[mesh.mMaterialIndex];
		aiString mtlname;
		mtl.Get(AI_MATKEY_NAME, mtlname);

		file << "\t\t\t<material>" << config.rpath 
			<< mtlname.C_Str() << ".mtl</material>\n";

		// end
		file << "\t\t</modelPatch>\n";
	}

	file << "\t</modelPatches>\n";
	file << "</model>\n";
}

//==============================================================================
void exportNode(const aiScene& scene, const aiNode* node, const Config& config)
{
	if(node == nullptr)
	{
		return;
	}

	// Write the .mdl
	exportModel(scene, *node, config);
	
	// Go to children
	for(uint32_t i = 0; i < node->mNumChildren; i++)
	{
		exportNode(scene, node->mChildren[i], config);
	}
}

//==============================================================================
void exportScene(const aiScene& scene, const Config& config)
{
	LOGI("Exporting scene to %s\n", config.outDir.c_str());

	// Meshes
	for(uint32_t i = 0; i < scene.mNumMeshes; i++)
	{
		exportMesh(*scene.mMeshes[i], config);
	}

	// Materials
	for(uint32_t i = 0; i < scene.mNumMaterials; i++)
	{
		exportMaterial(*scene.mMaterials[i], config, i);
	}

	// The nodes
	exportNode(scene, scene.mRootNode, config);

	LOGI("Done exporting scene!\n");
}

//==============================================================================
int main(int argc, char** argv)
{
	try
	{
		Config config;
		parseConfig(argc, argv, config);

		// Load
		Assimp::Importer importer;
		const aiScene& scene = load(config.inputFname, importer);

		// Export
		exportScene(scene, config);
	}
	catch(std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}
}
