#include "anki/resource/ModelPatch.h"
#include "anki/resource/Mesh.h"
#include "anki/resource/Material.h"


namespace anki {


//==============================================================================
// Constructors & Destructor                                                   =
//==============================================================================

ModelPatch::ModelPatch(const char* meshFName, const char* mtlFName)
{
	load(meshFName, mtlFName);
}


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


//==============================================================================
// supportsHwSkinning                                                          =
//==============================================================================
bool ModelPatch::supportsHwSkinning() const
{
	return mesh->hasVertWeights();
}


} // end namespace
