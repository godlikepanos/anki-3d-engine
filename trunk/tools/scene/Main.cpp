// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

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
#include "Common.h"

//==============================================================================
/// Load the scene
static void load(
	Exporter& exporter,
	const std::string& filename)
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
		ERROR("%s\n", exporter.importer.GetErrorString());
	}

	exporter.scene = scene;

	LOGI("File loaded successfully!\n");
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
static void visitNode(Exporter& exporter, const aiNode* ainode)
{
	if(ainode == nullptr)
	{
		return;
	}

	// For every mesh of this node
	for(unsigned i = 0; i < ainode->mNumMeshes; i++)
	{
		unsigned meshIndex = ainode->mMeshes[i];
		unsigned mtlIndex =  exporter.scene->mMeshes[meshIndex]->mMaterialIndex;

		// Find if there is another node with the same model
		std::vector<Node>::iterator it;
		for(it = exporter.nodes.begin(); it != exporter.nodes.end(); it++)
		{
			const Node& node = *it;
			const Model& model = exporter.models[node.modelIndex];

			if(model.meshIndex == meshIndex && model.mtlIndex == mtlIndex)
			{
				break;
			}
		}

		if(it != exporter.nodes.end())
		{
			// A node with the same model exists. It's instanced

			Node& node = *it;
			Model& model = exporter.models[node.modelIndex];

			assert(node.transforms.size() > 0);
			node.transforms.push_back(ainode->mTransformation);
			
			model.instanced = true;
			break;
		}

		// Create new model
		Model mdl;
		mdl.meshIndex = meshIndex;
		mdl.mtlIndex = mtlIndex;
		exporter.models.push_back(mdl);

		// Create new node
		Node node;
		node.modelIndex = exporter.models.size() - 1;
		node.transforms.push_back(ainode->mTransformation);
		exporter.nodes.push_back(node);
	}

	// Go to children
	for(uint32_t i = 0; i < ainode->mNumChildren; i++)
	{
		visitNode(exporter, ainode->mChildren[i]);
	}
}

//==============================================================================
static void writeNodeTransform(const Exporter& exporter, std::ofstream& file, 
	const std::string& node, const aiMatrix4x4& mat)
{
	aiMatrix4x4 m = toAnkiMatrix(mat, exporter.flipyz);

	float pos[3];
	pos[0] = m[0][3];
	pos[1] = m[1][3];
	pos[2] = m[2][3];

	file << "pos = Vec3.new()\n";
	file << "pos:setX(" << pos[0] << ")\n";
	file << "pos:setY(" << pos[1] << ")\n";
	file << "pos:setZ(" << pos[2] << ")\n";
	file << node 
		<< ":getSceneNodeBase():getMoveComponent():setLocalOrigin(pos)\n";

	file << "rot = Mat3.new()\n";
	for(unsigned j = 0; j < 3; j++)
	{
		for(unsigned i = 0; i < 3; i++)
		{
			file << "rot:setAt(" << j << ", " << i << ", " << m[j][i] << ")\n";
		}
	}
	file << node 
		<< ":getSceneNodeBase():getMoveComponent():setLocalRotation(rot)\n";
}

//==============================================================================
static void exportScene(Exporter& exporter)
{
	LOGI("Exporting scene to %s\n", exporter.outDir.c_str());

	//
	// Open scene file
	//
	std::ofstream file;
	file.open(exporter.outDir + "scene.lua");

	file << "scene = SceneGraphSingleton.get()\n";

	//
	// Get all the data
	//

	const aiNode* node = exporter.scene->mRootNode;
	visitNode(exporter, node);

	//
	// Export nodes
	//
	for(uint32_t i = 0; i < exporter.nodes.size(); i++)
	{
		Node& node = exporter.nodes[i];

		Model& model = exporter.models[node.modelIndex];

		exportMesh(exporter, 
			*exporter.scene->mMeshes[model.meshIndex], nullptr);

		exportMaterial(exporter, 
			*exporter.scene->mMaterials[model.mtlIndex], 
			model.instanced);

		exportModel(exporter, model);
		std::string name = getModelName(exporter, model);

		// Write the main node
		file << "\nnode = scene:newModelNode(\"" << name << "\", \"" 
			<< exporter.rpath << name << ".ankimdl" << "\")\n"; 
		writeNodeTransform(exporter, file, "node", node.transforms[0]);

		// Write instance nodes
		for(unsigned j = 1; j < node.transforms.size(); j++)
		{
			file << "inst = scene:newInstanceNode(\"" 
				<< name << "_inst" << (j - 1) << "\")\n"
				<< "node:getSceneNodeBase():addChild("
				<< "inst:getSceneNodeBase())\n";

			writeNodeTransform(exporter, file, "inst", node.transforms[j]);
		}
	}

	//
	// Animations
	//
	for(unsigned i = 0; i < exporter.scene->mNumAnimations; i++)
	{
		exportAnimation(exporter, *exporter.scene->mAnimations[i], i);
	}

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
		load(exporter, exporter.inputFname);

		// Export
		exportScene(exporter);
	}
	catch(std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}
}
