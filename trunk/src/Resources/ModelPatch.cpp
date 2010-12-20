#include "ModelPatch.h"
#include "Mesh.h"
#include "Material.h"


//======================================================================================================================
// supportsHardwareSkinning                                                                                            =
//======================================================================================================================
bool ModelPatch::supportsHwSkinning() const
{
	return mesh->hasVertWeights();
}


//======================================================================================================================
// supportsNormals                                                                                                     =
//======================================================================================================================
bool ModelPatch::supportsNormals() const
{
	return cpMtl->getStdAttribVar(Material::SAV_NORMAL) != NULL;
}


//======================================================================================================================
// supportsTangents                                                                                                    =
//======================================================================================================================
bool ModelPatch::supportsTangents() const
{
	return cpMtl->getStdAttribVar(Material::SAV_TANGENT) != NULL;
}


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
void ModelPatch::load(const char* meshFName, const char* cpMtlFName, const char* dpMtlFName)
{
	//
	// Load
	//
	mesh.loadRsrc(meshFName);
	cpMtl.loadRsrc(cpMtlFName);
	dpMtl.loadRsrc(dpMtlFName);

	//
	// Sanity checks
	//
	#define EXCEPTION_INCOMPATIBLE_RSRCS(x, y) \
		EXCEPTION("Resource \"" + x->getRsrcName() + "\" and \"" + y->getRsrcName() + "\" are incompatible")

	// if mtl needs tex coords then mesh should have
	if(cpMtl->hasTexCoords() && !mesh->hasTexCoords())
	{
		throw EXCEPTION_INCOMPATIBLE_RSRCS(cpMtl, mesh);
	}

	if(dpMtl->hasTexCoords() && !mesh->hasTexCoords())
	{
		throw EXCEPTION_INCOMPATIBLE_RSRCS(dpMtl, mesh);
	}
}
