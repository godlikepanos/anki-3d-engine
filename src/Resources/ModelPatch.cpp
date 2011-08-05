#include "ModelPatch.h"
#include "Mesh.h"
#include "Resources/Material.h"


//==============================================================================
// Constructors & Destructor                                                   =
//==============================================================================

ModelPatch::ModelPatch()
{}


ModelPatch::~ModelPatch()
{}


//==============================================================================
// load                                                                        =
//==============================================================================
void ModelPatch::load(const char* meshFName, const char* mtlFName)
{
	// Load
	mesh.loadRsrc(meshFName);
	mtl.loadRsrc(mtlFName);
}
