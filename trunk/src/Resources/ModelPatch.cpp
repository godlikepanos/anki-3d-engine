#include "ModelPatch.h"
#include "Mesh.h"
#include "Material.h"


//======================================================================================================================
// supportsHardwareSkinning                                                                                            =
//======================================================================================================================
bool ModelPatch::supportsHardwareSkinning() const
{
	return mesh->hasVertWeights();
}


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
void ModelPatch::load(const char* meshFName, const char* mtlFName, const char* dpMtlFName)
{
	//
	// Load
	//
	mesh.loadRsrc(meshFName);
	material.loadRsrc(mtlFName);
	dpMaterial.loadRsrc(dpMtlFName);

	//
	// Sanity checks
	//
	#define EXCEPTION_INCOMPATIBLE_RSRCS(x, y) \
		EXCEPTION("Resource \"" + x->getRsrcName() + "\" and \"" + y->getRsrcName() + "\" are incompatible")

	// if mtl needs tex coords then mesh should have
	if(material->hasTexCoords() && !mesh->hasTexCoords())
	{
		throw EXCEPTION_INCOMPATIBLE_RSRCS(material, mesh);
	}

	if(dpMaterial->hasTexCoords() && !mesh->hasTexCoords())
	{
		throw EXCEPTION_INCOMPATIBLE_RSRCS(dpMaterial, mesh);
	}
}
