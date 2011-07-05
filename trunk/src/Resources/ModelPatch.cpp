#include "ModelPatch.h"
#include "Mesh.h"
#include "Resources/Material.h"


//==============================================================================
// supportsHardwareSkinning                                                    =
//==============================================================================
bool ModelPatch::supportsHwSkinning() const
{
	return mesh->hasVertWeights();
}


//==============================================================================
// supportsNormals                                                             =
//==============================================================================
bool ModelPatch::supportsNormals() const
{
	return cpMtl->getStdAttribVar(Material::SAV_NORMAL) != NULL;
}


//==============================================================================
// supportsTangents                                                            =
//==============================================================================
bool ModelPatch::supportsTangents() const
{
	return cpMtl->getStdAttribVar(Material::SAV_TANGENT) != NULL;
}


//==============================================================================
// load                                                                        =
//==============================================================================
void ModelPatch::load(const char* meshFName, const char* cpMtlFName,
	const char* dpMtlFName)
{
	// Load
	mesh.loadRsrc(meshFName);
	cpMtl.loadRsrc(cpMtlFName);
	dpMtl.loadRsrc(dpMtlFName);

	// Sanity checks
	doMeshAndMtlSanityChecks(*mesh, *cpMtl);
	doMeshAndMtlSanityChecks(*mesh, *dpMtl);
}


//==============================================================================
// doMeshAndMtlSanityChecks                                                    =
//==============================================================================
void ModelPatch::doMeshAndMtlSanityChecks(const Mesh& mesh, const Material& mtl)
{
	try
	{
		// if mtl needs tex coords then mesh should have
		if(mtl.hasTexCoords() && !mesh.hasTexCoords())
		{
			throw EXCEPTION("Texture coords");
		}

		// Normals
		if(mtl.getStdAttribVar(Material::SAV_NORMAL) != NULL &&
			!mesh.hasNormalsAndTangents())
		{
			throw EXCEPTION("Normals");
		}

		// Tangents
		if(mtl.getStdAttribVar(Material::SAV_TANGENT) != NULL &&
			!mesh.hasNormalsAndTangents())
		{
			throw EXCEPTION("Tangents");
		}
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("Mesh and material are incompatible: " + e.what());
	}
}
