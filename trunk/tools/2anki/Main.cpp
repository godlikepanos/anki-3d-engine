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

struct Mesh
{
	uint32_t index = 0xFFFFFFFF; ///< Mesh index in the scene
	std::vector<aiMatrix4x4> transforms;
	uint32_t mtlIndex = 0xFFFFFFFF;
};

struct Material
{
	uint32_t index = 0xFFFFFFFF;
	std::vector<uint32_t> meshIndices;
};

struct Config
{
	std::string inputFname;
	std::string outDir;
	std::string rpath;
	std::string texpath;
	bool flipyz = false;

	std::vector<Mesh> meshes;
	std::vector<Material> materials;
};

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
static std::string replaceAllString(
	const std::string& str, 
	const std::string& from, 
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
static std::string getFilename(const std::string& path)
{
	std::string out;

	const size_t last = path.find_last_of("/");
	if(std::string::npos != last)
	{
		out.insert(out.end(), path.begin() + last + 1, path.end());
	}

	return out;
}

//==============================================================================
static void parseConfig(int argc, char** argv, Config& config)
{
	static const char* usage = R"(Usage: 2anki in_file out_dir [options]
Options:
-rpath <string>   : Append a string to the meshes and materials
-texrpath         : Append a string to the textures paths
-flipyz           : Flip y with z (For blender exports)
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
		if(strcmp(argv[i], "-texrpath") == 0)
		{
			++i;

			if(i < argc)
			{
				config.texpath = argv[i] + std::string("/");
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
static const aiScene& load(
	const std::string& filename, 
	Assimp::Importer& importer)
{
	LOGI("Loading file %s\n", filename.c_str());

	const aiScene* scene = importer.ReadFile(filename, 0
		//| aiProcess_FindInstances
		| aiProcess_Triangulate
		| aiProcess_JoinIdenticalVertices
		//| aiProcess_SortByPType
		| aiProcess_ImproveCacheLocality
		| aiProcess_OptimizeMeshes
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
static void exportMesh(
	const aiMesh& mesh, 
	const std::string* name_,
	const aiMatrix4x4* transform,
	const Config& config)
{
	std::string name = (name_) ? *name_ : mesh.mName.C_Str();
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

		// Transform
		if(transform)
		{
			pos = (*transform) * pos;
		}

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
static void exportSkeleton(const aiMesh& mesh, const Config& config)
{
	assert(mesh.HasBones());
	std::string name = mesh.mName.C_Str();
	std::fstream file;
	LOGI("Exporting skeleton %s\n", name.c_str());

	// Open file
	file.open(config.outDir + name + ".skel", std::ios::out);
	
	file << xmlHeader << "\n";
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
static std::string getMaterialName(const aiMaterial& mtl)
{
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

	return name;
}

//==============================================================================
static void exportMaterial(
	const aiScene& scene, 
	const aiMaterial& mtl, 
	bool instanced,
	const std::string* name_,
	const Config& config)
{
	std::string diffTex;
	std::string normTex;

	std::string name;
	if(name_)
	{
		name = *name_;
	}
	else
	{
		name = getMaterialName(mtl);
	}
	

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
	file.open(config.outDir + name + ".mtl", std::ios::out);

	// Chose the correct template
	std::string str;
	if(normTex.size() == 0)
	{
		str = diffMtlStr;
	}
	else
	{
		str = replaceAllString(diffNormMtlStr, "%normalMap%", 
			config.texpath + normTex);
	}

	str = replaceAllString(str, "%instanced%", (instanced) ? "1" : "0");
	str = replaceAllString(str, "%diffuseMap%", config.texpath + diffTex);

	file << str;
}

//==============================================================================
static void exportLight(
	const aiLight& light, 
	const Config& config, 
	std::fstream& file)
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
static void exportModel(
	const aiScene& scene, 
	const aiNode& node, 
	const Config& config)
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
static void exportAnimation(
	const aiAnimation& anim, 
	uint32_t index, 
	const aiScene& scene, 
	const Config& config)
{
	// Get name
	std::string name = anim.mName.C_Str();
	if(name.size() == 0)
	{
		name = std::string("animation_") + std::to_string(index);
	}

	// Find if it's skeleton animation
	/*bool isSkeletalAnimation = false;
	for(uint32_t i = 0; i < scene.mNumMeshes; i++)
	{
		const aiMesh& mesh = *scene.mMeshes[i];
		if(mesh.HasBones())
		{

		}
	}*/

	std::fstream file;
	LOGI("Exporting animation %s\n", name.c_str());

	file.open(config.outDir + name + ".anim", std::ios::out);

	file << xmlHeader << "\n";
	file << "<animation>\n";

	file << "\t<duration>" << anim.mDuration << "</duration>\n";

	file << "\t<channels>\n";

	for(uint32_t i = 0; i < anim.mNumChannels; i++)
	{
		const aiNodeAnim& nAnim = *anim.mChannels[i];

		file << "\t\t<channel>\n";
		
		// Name
		file << "\t\t\t<name>" << nAnim.mNodeName.C_Str() << "</name>\n";

		// Positions
		file << "\t\t\t<positionKeys>\n";
		for(uint32_t j = 0; j < nAnim.mNumPositionKeys; j++)
		{
			const aiVectorKey& key = nAnim.mPositionKeys[j];

			file << "\t\t\t\t<key><time>" << key.mTime << "</time>"
				<< "<value>" << key.mValue[0] << " " 
				<< key.mValue[1] << " " << key.mValue[2] << "</value></key>\n";
		}
		file << "</positionKeys>\n";

		// Rotations
		file << "\t\t\t<rotationKeys>";
		for(uint32_t j = 0; j < nAnim.mNumRotationKeys; j++)
		{
			const aiQuatKey& key = nAnim.mRotationKeys[j];

			file << "\t\t\t\t<key><time>" << key.mTime << "</time>"
				<< "<value>" << key.mValue.y << " " << key.mValue.z << " "
				<< key.mValue.w << "</value></key>\n";
		}
		file << "</rotationKeys>\n";

		// Scale
		file << "\t\t\t<scalingKeys>";
		for(uint32_t j = 0; j < nAnim.mNumScalingKeys; j++)
		{
			const aiVectorKey& key = nAnim.mScalingKeys[j];

			// Note: only uniform scale
			file << "\t\t\t\t<key><time>" << key.mTime << "</time>"
				<< "<value>" 
				<< ((key.mValue[0] + key.mValue[1] + key.mValue[2]) / 3.0)
				<< "</value></key>\n";
		}
		file << "</scalingKeys>\n";

		file << "\t\t</channel>\n";
	}

	file << "\t</channels>\n";
	file << "</animation>\n";
}

//==============================================================================
static void visitNode(const aiNode* node, const aiScene& scene, Config& config)
{
	if(node == nullptr)
	{
		return;
	}

	// For every mesh of this node
	for(uint32_t i = 0; i < node->mNumMeshes; i++)
	{
		uint32_t meshIndex = node->mMeshes[i];
		const aiMesh& mesh = *scene.mMeshes[meshIndex];

		// Is material set?
		if(config.meshes[meshIndex].mtlIndex == 0xFFFFFFFF)
		{
			// Connect mesh with material
			config.meshes[meshIndex].mtlIndex = mesh.mMaterialIndex;

			// Connect material with mesh
			config.materials[mesh.mMaterialIndex].meshIndices.push_back(
				meshIndex);
		}
		else if(config.meshes[meshIndex].mtlIndex != mesh.mMaterialIndex)
		{
			ERROR("Previous material index conflict\n");
		}

		config.meshes[meshIndex].transforms.push_back(node->mTransformation);
	}

	// Go to children
	for(uint32_t i = 0; i < node->mNumChildren; i++)
	{
		visitNode(node->mChildren[i], scene, config);
	}
}

//==============================================================================
static void exportScene(const aiScene& scene, Config& config)
{
	LOGI("Exporting scene to %s\n", config.outDir.c_str());

	// Open scene file
	std::ofstream file;
	file.open(config.outDir + "master.scene");

	file << xmlHeader << "\n"
		<< "<scene>\n";

	// Get all the data
	config.meshes.resize(scene.mNumMeshes);
	config.materials.resize(scene.mNumMaterials);

	int i = 0;
	for(Mesh& mesh : config.meshes)
	{
		mesh.index = i++;
	}
	i = 0;
	for(Material& mtl : config.materials)
	{
		mtl.index = i++;
	}

	const aiNode* node = scene.mRootNode;

	visitNode(node, scene, config);

#if 0
	// Export all meshes that are used
	for(uint32_t i = 0; i < config.meshes.size(); i++)
	{
		// Check if used
		if(config.meshes[i].transforms.size() < 1)
		{
			continue;
		}

		std::string name = "mesh_" + std::to_string(i);

		for(uint32_t t = 0; t < config.meshes[i].transforms.size(); t++)
		{
			std::string nname = name + "_" + std::to_string(t);

			exportMesh(*scene.mMeshes[i], &nname, 
				&config.meshes[i].transforms[t], config);
		}
	}

	// Export materials
	for(uint32_t i = 0; i < config.materials.size(); i++)
	{
		// Check if used
		if(config.materials[i].meshIndices.size() < 1)
		{
			continue;
		}

		exportMaterial(scene, *scene.mMaterials[i], config);
	}
#endif

	// Write the instanced meshes
	for(uint32_t i = 0; i < config.meshes.size(); i++)
	{
		const Mesh& mesh = config.meshes[i];

		// Skip meshes that are not instance candidates
		if(mesh.transforms.size() < 2)
		{
			continue;
		}

		// Export the material
		aiMaterial& aimtl = *scene.mMaterials[mesh.mtlIndex];
		std::string mtlName = getMaterialName(aimtl) + "_instance";
		exportMaterial(scene, aimtl, true, &mtlName, config);

		// Export mesh
		std::string meshName = mtlName + "_" + std::to_string(i);
		exportMesh(*scene.mMeshes[i], &meshName, nullptr, config);

		// Write model file
		std::string modelName = mtlName + "_" + std::to_string(i);

		{
			std::ofstream file;
			file.open(
				config.outDir + modelName + ".mdl");

			file << xmlHeader << "\n"
				<< "<model>\n"
				<< "\t<modelPatches>\n"
				<< "\t\t<modelPatch>\n"
				<< "\t\t\t<mesh>" << config.outDir << meshName 
				<< ".mesh</mesh>\n"
				<< "\t\t\t<material>" << config.outDir << mtlName 
				<< ".mtl</material>\n"
				<< "\t\t</modelPatch>\n"
				<< "\t</modelPatches>\n"
				<< "</model>\n";
		}

		// Node name
		std::string nodeName = getMaterialName(aimtl) + "_instanced_" 
			+ std::to_string(i);

		// Write the scene file
		file << "\t<modelNode>\n"
			<< "\t\t<name>" << nodeName << "</name>\n"
			<< "\t\t<model>" << config.outDir << modelName << ".mdl</model>\n"
			<< "\t\t<instancesCount>" 
			<< mesh.transforms.size() << "</instancesCount>\n";

		for(uint32_t j = 0; j < mesh.transforms.size(); j++)
		{
			file << "\t\t<transform>";

			aiMatrix4x4 trf = mesh.transforms[j];

			for(uint32_t a = 0; a < 4; a++)
			{
				for(uint32_t b = 0; b < 4; b++)
				{
					file << trf[a][b] << " ";
				}
			}

			file << "</transform>\n";
		}

		file << "\t</modelNode>\n";
	}


#if 0
	// Write bmeshes
	for(uint32_t mtlId = 0; mtlId < config.materials.size(); mtlId++)
	{
		const Material& mtl = config.materials[mtlId];

		// Check if used
		if(mtl.meshIndices.size() < 1)
		{
			continue;
		}

		std::string name = getMaterialName(*scene.mMaterials[mtlId]) + ".bmesh";

		std::fstream file;
		file.open(config.outDir + name, std::ios::out);

		file << xmlHeader << "\n";
		file << "<bucketMesh>\n";
		file << "\t<meshes>\n";
		
		for(uint32_t j = 0; j < mtl.meshIndices.size(); j++)
		{
			uint32_t meshId = mtl.meshIndices[j];
			const Mesh& mesh = config.meshes[meshId];

			for(uint32_t k = 0; k < mesh.transforms.size(); k++)
			{
				file << "\t\t<mesh>" << config.rpath 
					<< "mesh_" + std::to_string(meshId) << "_" 
					<< std::to_string(k)
					<< ".mesh</mesh>\n";
			}
		}

		file << "\t</meshes>\n";
		file << "</bucketMesh>\n";
	}

	// Create the master model
	std::fstream file;
	file.open(config.outDir + "static_geometry.mdl", std::ios::out);

	file << xmlHeader << "\n";
	file << "<model>\n";
	file << "\t<modelPatches>\n";
	for(uint32_t i = 0; i < config.materials.size(); i++)
	{
		// Check if used
		if(config.materials[i].meshIndices.size() < 1)
		{
			continue;
		}

		file << "\t\t<modelPatch>\n";
		file << "\t\t\t<bucketMesh>" << config.rpath 
			<< getMaterialName(*scene.mMaterials[i]) << ".bmesh</bucketMesh>\n";
		file << "\t\t\t<material>" << config.rpath 
			<< getMaterialName(*scene.mMaterials[i]) 
			<< ".mtl</material>\n";
		file << "\t\t</modelPatch>\n";
	}
	file << "\t</modelPatches>\n";
	file << "</model>\n";
#endif

	file << "</scene>\n";

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
