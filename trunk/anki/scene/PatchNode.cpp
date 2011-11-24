#include "anki/scene/PatchNode.h"
#include "anki/resource/Material.h"
#include "anki/scene/MaterialRuntime.h"


namespace anki {


//==============================================================================
// Constructor                                                                 =
//==============================================================================
PatchNode::PatchNode(const ModelPatch& modelPatch, ulong flags,
	SceneNode& parent)
:	RenderableNode(parent.getScene(), flags | SNF_INHERIT_PARENT_TRANSFORM,
		&parent),
	rsrc(modelPatch),
	mtlRun(new MaterialRuntime(rsrc.getMaterial()))
{}


} // end namespace
