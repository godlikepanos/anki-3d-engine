#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <stdexcept>
#include <cstdarg>
#include <fstream>
#include <cstdint>

//==============================================================================
void parseCmdLineArgs(int argc, char** argv, std::string& inputFname, 
	std::string& outDir)
{
	// Parse args
	if(argc != 3)
	{
		throw std::runtime_error("Usage:\n" 
			+ std::string(argv[0]) + " input_file output_dir");
	}

	inputFname = argv[1];
	outDir = argv[2];
}

//==============================================================================
#define STR(s) #s
#define XSTR(s) STR(s)
#define LOGI(...) \
	printf("[I] (" __FILE__ ":" XSTR(__LINE__) ") " __VA_ARGS__)

//==============================================================================
/// Load the scene
const aiScene& load(const std::string& filename, Assimp::Importer& importer)
{
	LOGI("Loading file %s\n", filename.c_str());

	const aiScene* scene = importer.ReadFile(filename,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	if(!scene)
	{
		throw std::runtime_error(importer.GetErrorString());
	}

	LOGI("File loaded successfully!\n");
	return *scene;
}

//==============================================================================
void exportMesh(const aiMesh& mesh, const std::string& outDir)
{
	std::string name = mesh.mName.C_Str();
	LOGI("Exporting mesh %s\n", name.c_str());

	std::fstream file;

	// Open file
	file.open(outDir + "/" + name + ".mesh",
		std::ios::out | std::ios::binary);

	// Write magic word
	file.write("ANKIMESH", 8);

	// Write the name
	uint32_t size = name.size();
	file.write((char*)&size, sizeof(uint32_t));
	file.write(&name[0], size);

	// Write the positions
	file.write((char*)&mesh.mNumVertices, sizeof(uint32_t));
	for(uint32_t i = 0; i < mesh.mNumVertices; i++)
	{
		aiVector3D pos = mesh.mVertices[i];

		for(uint32_t j = 0; j < 3; j++)
		{
			file.write((char*)&pos[j], sizeof(float));
		}
	}

	// Write the indices
	file.write((char*)&mesh.mNumFaces, sizeof(uint32_t));
	for(uint32_t i = 0; i < mesh.mNumFaces; i++)
	{
		const aiFace& face = *mesh.mFaces[i];
		
		if(face.mNumIndices != 3)
		{
			throw std::runtime_error("For some reason the assimp didn't "
				"triangulate");
		}

		for(uint32_t j = 0; j < 3; j++)
		{
			uint32_t index = face.mIndices[j];
			file.write((char*)&index, sizeof(uint32_t));
		}
	}

	// Write tex coords
}

//==============================================================================
void exportScene(const aiScene& scene, const std::string& outDir)
{
	LOGI("Exporting scene to %s\n", outDir.c_str());

	// Meshes
	for(unsigned i = 0; i < scene.mNumMeshes; i++)
	{
		exportMesh(*scene.mMeshes[i], outDir);
	}

	LOGI("Done exporting scene!\n");
}

//==============================================================================
int main(int argc, char** argv)
{
	try
	{
		std::string inputFname, outDir;
		parseCmdLineArgs(argc, argv, inputFname, outDir);

		// Load
		Assimp::Importer importer;
		const aiScene& scene = load(inputFname, importer);

		// Export
		exportScene(scene, outDir);
	}
	catch(std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}
}
