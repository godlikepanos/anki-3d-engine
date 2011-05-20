#include "SkinsDeformer.h"
#include "ShaderProg.h"
#include "SkinPatchNode.h"
#include "SkinNode.h"


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void SkinsDeformer::init()
{
	//
	// Load the shaders
	//
	tfHwSkinningAllSProg.loadRsrc("shaders/TfHwSkinningPosNormTan.glsl");
	tfHwSkinningPosSProg.loadRsrc("shaders/TfHwSkinningPos.glsl");
}


//======================================================================================================================
// deform                                                                                                              =
//======================================================================================================================
void SkinsDeformer::deform(SkinPatchNode& node)
{
	ASSERT(node.getParent() != NULL); // The SkinPatchNode are always have parent
	ASSERT(static_cast<SceneNode*>(node.getParent())->getSceneNodeType() == SceneNode::SNT_SKIN);

	SkinNode* skinNode = static_cast<SkinNode*>(node.getParent());

	glEnable(GL_RASTERIZER_DISCARD);

	const ShaderProg* sProg;

	if(node.getModelPatchRsrc().supportsNormals() &&
	   node.getModelPatchRsrc().supportsTangents())
	{
		sProg = tfHwSkinningAllSProg.get();
	}
	else
	{
		sProg = tfHwSkinningPosSProg.get();
	}

	sProg->bind();

	// Uniforms
	sProg->findUniVar("skinningRotations")->set(&skinNode->getBoneRotations()[0],
	                                                           skinNode->getBoneRotations().size());

	sProg->findUniVar("skinningTranslations")->set(&skinNode->getBoneTranslations()[0],
	                                                              skinNode->getBoneTranslations().size());

	node.getTfVao().bind();

	// TF
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, node.getTfVbo(SkinPatchNode::TFV_POSITIONS).getGlId());

	if(sProg == tfHwSkinningAllSProg.get())
	{
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, node.getTfVbo(SkinPatchNode::TFV_NORMALS).getGlId());
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, node.getTfVbo(SkinPatchNode::TFV_TANGENTS).getGlId());
	}

	//glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, this->Query);
	glBeginTransformFeedback(GL_POINTS);
		glDrawArrays(GL_POINTS, 0, node.getModelPatchRsrc().getMesh().getVertsNum());
	glEndTransformFeedback();
	//glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

	glDisable(GL_RASTERIZER_DISCARD);
}








