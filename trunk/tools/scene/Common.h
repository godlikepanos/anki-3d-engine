#ifndef ANKI_TOOLS_SCENE_MISC_H
#define ANKI_TOOLS_SCENE_MISC_H

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>
#include <fstream>

//==============================================================================
// Log and errors

#define STR(s) #s
#define XSTR(s) STR(s)
#define LOGI(...) \
	printf("[I] (" __FILE__ ":" XSTR(__LINE__) ") " __VA_ARGS__)

#define ERROR(...) \
	do { \
		fprintf(stderr, "[E] (" __FILE__ ":" XSTR(__LINE__) ") " __VA_ARGS__); \
		exit(1); \
	} while(0)

#define LOGW(...) \
	fprintf(stderr, "[W] (" __FILE__ ":" XSTR(__LINE__) ") " __VA_ARGS__)

/// A string
extern const char* XML_HEADER;

const uint32_t INVALID_INDEX = 0xFFFFFFFF;

/// Thin mesh wrapper
struct Mesh
{
	uint32_t index = INVALID_INDEX; ///< Mesh index in the scene
	std::vector<aiMatrix4x4> transforms;
	uint32_t mtlIndex = INVALID_INDEX;
};

/// Thin material wrapper
struct Material
{
	uint32_t index = INVALID_INDEX;
	std::vector<uint32_t> meshIndices;
};

const uint32_t MAX_BONES_PER_VERTEX = 4;

/// Bone/weight info for a single vertex
struct VertexWeight
{
	uint32_t boneIds[MAX_BONES_PER_VERTEX];
	float weigths[MAX_BONES_PER_VERTEX];
	uint32_t bonesCount;
};

/// Exporter class. This holds all the globals
struct Exporter
{
	std::string inputFname;
	std::string outDir;
	std::string rpath;
	std::string texrpath;
	bool flipyz = false; ///< Handle blender's annoying coordinate system

	aiScene* scene = nullptr;
	Assimp::Importer* importer;

	std::vector<Mesh> meshes;
	std::vector<Material> materials;
};

/// Replace all @a from substrings in @a str to @a to
extern std::string replaceAllString(
	const std::string& str, 
	const std::string& from, 
	const std::string& to);

/// From a path return only the filename
extern std::string getFilename(const std::string& path);

/// Convert one 4x4 matrix to AnKi friendly matrix
extern aiMatrix4x4 toAnkiMatrix(const aiMatrix4x4& in, bool flipyz);

/// Convert one 3x3 matrix to AnKi friendly matrix
extern aiMatrix3x3 toAnkiMatrix(const aiMatrix3x3& in, bool flipyz);

/// Export animation
extern void exportAnimation(
	const Exporter& exporter,
	const aiAnimation& anim,
	uint32_t index);

/// Export a mesh
extern void exportMesh(
	const Exporter& exporter,
	const aiMesh& mesh, 
	const std::string* name,
	const aiMatrix4x4* transform);

/// Export a skeleton
extern void exportSkeleton(const Exporter& exporter, const aiMesh& mesh);

/// Helper function
extern std::string getMaterialName(const aiMaterial& mtl);

/// Export material
extern void exportMaterial(
	const Exporter& exporter, 
	const aiMaterial& mtl, 
	bool instanced,
	const std::string* name);

/// Write light to the scene file
extern void exportLight(
	const aiLight& light, 
	std::fstream& file);

#endif
