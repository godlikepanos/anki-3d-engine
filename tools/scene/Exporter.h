// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_TOOLS_SCENE_EXPORTER_H
#define ANKI_TOOLS_SCENE_EXPORTER_H

#include <string>
#include <array>
#include <cstdint>
#include <fstream>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Common.h"

const uint32_t INVALID_INDEX = 0xFFFFFFFF;

/// Thin mesh wrapper
struct Model
{
	uint32_t m_meshIndex = INVALID_INDEX; ///< Mesh index in the scene
	uint32_t m_materialIndex = INVALID_INDEX;
	bool m_instanced = false;
};

/// Scene node.
struct Node
{
	uint32_t m_modelIndex; ///< Index inside Exporter::m_models
	std::vector<aiMatrix4x4> m_transforms;
};

const uint32_t MAX_BONES_PER_VERTEX = 4;

/// Bone/weight info for a single vertex
struct VertexWeight
{
	std::array<uint32_t, MAX_BONES_PER_VERTEX> m_boneIndices;
	std::array<float, MAX_BONES_PER_VERTEX> m_weigths;
	uint32_t m_bonesCount;
};

/// AnKi exporter.
class Exporter
{
public:
	std::string m_inputFilename;
	std::string m_outputDirectory;
	std::string m_rpath;
	std::string m_texrpath;
	
	bool m_flipyz = false;

	const aiScene* m_scene = nullptr;
	Assimp::Importer m_importer;

	std::vector<Model> m_models;
	std::vector<Node> m_nodes;

	std::ofstream m_sceneFile;

	/// Load the scene.
	void load();

	/// Export.
	void exportAll();

private:
	/// @name Helpers
	/// @{

	/// Convert one 4x4 matrix to AnKi friendly matrix.
	aiMatrix4x4 toAnkiMatrix(const aiMatrix4x4& in) const;

	/// Convert one 3x3 matrix to AnKi friendly matrix.
	aiMatrix3x3 toAnkiMatrix(const aiMatrix3x3& in) const;

	/// Write transformation of a node
	void writeNodeTransform(
		const std::string& node, 
		const aiMatrix4x4& mat);

	const aiMesh& getMeshAt(unsigned index) const;
	const aiMaterial& getMaterialAt(unsigned index) const;
	std::string getModelName(const Model& model) const;

	/// Visits the node hierarchy and gathers models and nodes.
	void visitNode(const aiNode* ainode);
	/// @}

	/// Export a mesh.
	/// @param transform If not nullptr then transform the vertices using that.
	void exportMesh(
		const aiMesh& mesh, 
		const aiMatrix4x4* transform) const;

	/// Export a skeleton.
	void exportSkeleton(const aiMesh& mesh) const;

	/// Export a material.
	void exportMaterial(
		const aiMaterial& mtl, 
		bool instanced) const;

	/// Export a model.
	void exportModel(const Model& model) const;

	/// Export a light.
	void exportLight(const aiLight& light);

	/// Export an animation.
	void exportAnimation(
		const aiAnimation& anim,
		unsigned index);
};

#endif

