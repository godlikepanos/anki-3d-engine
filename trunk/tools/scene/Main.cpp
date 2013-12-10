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

//==============================================================================
/// Load the scene
static const void load(
	const std::string& filename,
	Exporter& exporter)
{
	LOGI("Loading file %s\n", filename.c_str());

	const aiScene* scene = exporter.importer.ReadFile(filename, 0
		//| aiProcess_FindInstances
		| aiProcess_Triangulate
		| aiProcess_JoinIdenticalVertices
		//| aiProcess_SortByPType
		| aiProcess_ImproveCacheLocality
		| aiProcess_OptimizeMeshes
		| aiProcess_RemoveRedundantMaterials
		);

	if(!scene)
	{
		ERROR("%s\n", importer.GetErrorString());
	}

	exporter.scene = scene;

	LOGI("File loaded successfully!\n");
	return *scene;
}

//==============================================================================
static void parseCommandLineArgs(int argc, char** argv, Exporter& exporter)
{
	static const char* usage = R"(Usage: %s in_file out_dir [options]
Options:
-rpath <string>    : Append a string to the meshes and materials
-texrpath <string> : Append a string to the textures paths
-flipyz            : Flip y with z (For blender exports)
)";

	// Parse config
	if(argc < 3)
	{
		goto error;
	}

	exporter.inputFname = argv[1];
	exporter.outDir = argv[2] + std::string("/");

	for(int i = 3; i < argc; i++)
	{
		if(strcmp(argv[i], "-texrpath") == 0)
		{
			++i;

			if(i < argc)
			{
				exporter.texrpath = argv[i] + std::string("/");
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
				exporter.rpath = argv[i] + std::string("/");
			}
			else
			{
				goto error;
			}
		}
		else if(strcmp(argv[i], "-flipyz") == 0)
		{
			exporter.flipyz = true;
		}
		else
		{
			goto error;
		}
	}

	if(exporter.rpath.empty())
	{
		exporter.rpath = exporter.outDir;
	}

	if(exporter.texrpath.empty())
	{
		exporter.texrpath = exporter.outDir;
	}

	return;

error:
	printf(usage, argv[0]);
	exit(1);
}

//==============================================================================
static void exportModel(
	const aiScene& scene, 
	const aiNode& node)
{
	if(node.mNumMeshes == 0)
	{
		return;
	}

	std::string name = node.mName.C_Str();

	LOGI("Exporting model %s\n", name.c_str());

	std::fstream file;
	file.open(config.outDir + name + ".ankimdl", std::ios::out);

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
			<< mesh.mName.C_Str() << ".ankimesh</mesh>\n";

		// Write material
		const aiMaterial& mtl = *scene.mMaterials[mesh.mMaterialIndex];
		aiString mtlname;
		mtl.Get(AI_MATKEY_NAME, mtlname);

		file << "\t\t\t<material>" << config.rpath 
			<< mtlname.C_Str() << ".ankimtl</material>\n";

		// end
		file << "\t\t</modelPatch>\n";
	}

	file << "\t</modelPatches>\n";
	file << "</model>\n";
}

//==============================================================================
static void visitNode(const aiNode* node, const aiScene& scene)
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
		if(config.meshes[meshIndex].mtlIndex == INVALID_INDEX)
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
		visitNode(node->mChildren[i], scene);
	}
}

//==============================================================================
static void exportScene(const aiScene& scene)
{
	LOGI("Exporting scene to %s\n", config.outDir.c_str());

	//
	// Open scene file
	//
	std::ofstream file;
	file.open(config.outDir + "master.ankiscene");

	file << xmlHeader << "\n"
		<< "<scene>\n";

	//
	// Get all the data
	//

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

	visitNode(node, scene);

#if 0
	//
	// Export non instanced static meshes
	//
	for(uint32_t i = 0; i < config.meshes.size(); i++)
	{
		// Check if instance is one
		if(config.meshes[i].transforms.size() == 1)
		{
			continue;
		}

		// Export the material
		aiMaterial& aimtl = *scene.mMaterials[mesh.mtlIndex];
		std::string mtlName = getMaterialName(aimtl);
		exportMaterial(scene, aimtl, false, &mtlName);

		// Export mesh
		std::string meshName = std::string(scene.mMeshes[i]->mName.C_Str())
			+ "_static_" + std::to_string(i);
		exportMesh(*scene.mMeshes[i], &meshName, nullptr);

		for(uint32_t t = 0; t < config.meshes[i].transforms.size(); t++)
		{
			std::string nname = name + "_" + std::to_string(t);

			exportMesh(*scene.mMeshes[i], &nname, 
				&config.meshes[i].transforms[t], config);
		}
	}
#endif

	//
	// Write the instanced meshes
	//
	for(uint32_t i = 0; i < config.meshes.size(); i++)
	{
		const Mesh& mesh = config.meshes[i];

		// Skip meshes that are not instance candidates
		if(mesh.transforms.size() == 0)
		{
			continue;
		}

		// Export the material
		aiMaterial& aimtl = *scene.mMaterials[mesh.mtlIndex];
		std::string mtlName = getMaterialName(aimtl) + "_instanced";
		exportMaterial(scene, aimtl, true, &mtlName);

		// Export mesh
		std::string meshName = std::string(scene.mMeshes[i]->mName.C_Str())
			+ "_instanced_" + std::to_string(i);
		exportMesh(*scene.mMeshes[i], &meshName, nullptr);

		// Write model file
		std::string modelName = mtlName + "_" + std::to_string(i);

		{
			std::ofstream file;
			file.open(
				config.outDir + modelName + ".ankimdl");

			file << xmlHeader << "\n"
				<< "<model>\n"
				<< "\t<modelPatches>\n"
				<< "\t\t<modelPatch>\n"
				<< "\t\t\t<mesh>" << config.rpath << meshName 
				<< ".ankimesh</mesh>\n"
				<< "\t\t\t<material>" << config.rpath << mtlName 
				<< ".ankimtl</material>\n"
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
			<< "\t\t<model>" << config.rpath << modelName 
			<< ".ankimdl</model>\n"
			<< "\t\t<instancesCount>" 
			<< mesh.transforms.size() << "</instancesCount>\n";

		for(uint32_t j = 0; j < mesh.transforms.size(); j++)
		{
			file << "\t\t<transform>";

			aiMatrix4x4 trf = toAnkiMatrix(mesh.transforms[j]);

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

	//
	// Animations
	//
	for(unsigned i = 0; i < scene.mNumAnimations; i++)
	{
		exportAnimation(*scene.mAnimations[i], i, scene);
	}

	file << "</scene>\n";

	LOGI("Done exporting scene!\n");
}

//==============================================================================
int main(int argc, char** argv)
{
	try
	{
		Exporter exporter;

		parseCommandLineArgs(argc, argv, exporter);

		// Load file
		load(exporter.inputFname, exporter);

		// Export
		exportScene(exporter);
	}
	catch(std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}
}
