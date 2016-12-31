// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
	std::string m_lod1MeshName;
};

/// Scene node.
struct Node
{
	uint32_t m_modelIndex; ///< Index inside Exporter::m_models
	aiMatrix4x4 m_transform;
	std::string m_group;
	std::string m_collisionMesh;
};

const uint32_t MAX_BONES_PER_VERTEX = 4;

/// Bone/weight info for a single vertex
struct VertexWeight
{
	std::array<uint32_t, MAX_BONES_PER_VERTEX> m_boneIndices;
	std::array<float, MAX_BONES_PER_VERTEX> m_weigths;
	uint32_t m_bonesCount;
};

class Portal
{
public:
	uint32_t m_meshIndex;
	aiMatrix4x4 m_transform;
};

using Sector = Portal;

class ParticleEmitter
{
public:
	std::string m_filename;
	aiMatrix4x4 m_transform;
};

class StaticCollisionNode
{
public:
	uint32_t m_meshIndex;
	aiMatrix4x4 m_transform;
};

class ReflectionProbe
{
public:
	aiVector3D m_position;
	float m_radius;
};

class ReflectionProxy
{
public:
	aiMatrix4x4 m_transform;
	uint32_t m_meshIndex; ///< Points to the scene that is not triangulated.
};

class OccluderNode
{
public:
	aiMatrix4x4 m_transform;
	uint32_t m_meshIndex; ///< Points to the scene that is not triangulated.
};

class DecalNode
{
public:
	aiMatrix4x4 m_transform;
	std::string m_diffuseTextureAtlasFilename;
	std::string m_diffuseSubTextureName;
	std::string m_normalRoughnessAtlasFilename;
	std::string m_normalRoughnessSubTextureName;
	aiVector3D m_size;
	std::array<float, 2> m_factors = {{1.0, 1.0}};
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
	const aiScene* m_sceneNoTriangles = nullptr;
	Assimp::Importer m_importer;
	Assimp::Importer m_importerNoTriangles;

	std::vector<Model> m_models;
	std::vector<Node> m_nodes;

	std::ofstream m_sceneFile;

	std::vector<StaticCollisionNode> m_staticCollisionNodes;
	std::vector<Portal> m_portals;
	std::vector<Sector> m_sectors;
	std::vector<ParticleEmitter> m_particleEmitters;
	std::vector<ReflectionProbe> m_reflectionProbes;
	std::vector<ReflectionProxy> m_reflectionProxies;
	std::vector<OccluderNode> m_occluders;
	std::vector<DecalNode> m_decals;

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

	void writeTransform(const aiMatrix4x4& mat);

	/// Write transformation of a node
	void writeNodeTransform(const std::string& node, const aiMatrix4x4& mat);

	const aiMesh& getMeshAt(unsigned index) const;
	const aiMaterial& getMaterialAt(unsigned index) const;
	std::string getModelName(const Model& model) const;

	/// Visits the node hierarchy and gathers models and nodes.
	void visitNode(const aiNode* ainode);
	/// @}

	/// Export a mesh.
	/// @param transform If not nullptr then transform the vertices using that.
	void exportMesh(const aiMesh& mesh, const aiMatrix4x4* transform, unsigned vertCountPerFace) const;

	/// Export a skeleton.
	void exportSkeleton(const aiMesh& mesh) const;

	/// Export a material.
	void exportMaterial(const aiMaterial& mtl) const;

	/// Export a model.
	void exportModel(const Model& model) const;

	/// Export a light.
	void exportLight(const aiLight& light);

	/// Export a camera.
	void exportCamera(const aiCamera& cam);

	/// Export an animation.
	void exportAnimation(const aiAnimation& anim, unsigned index);

	/// Export a static collision mesh.
	void exportCollisionMesh(uint32_t meshIdx);

	/// Helper.
	static std::string getMaterialName(const aiMaterial& mtl);
};

#endif
