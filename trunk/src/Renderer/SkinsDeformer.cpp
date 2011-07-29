#include "SkinsDeformer.h"
#include "Resources/ShaderProgram.h"
#include "Scene/SkinPatchNode.h"
#include "Scene/SkinNode.h"


namespace R {


//==============================================================================
// init                                                                        =
//==============================================================================
void SkinsDeformer::init()
{
	//
	// Load the shaders
	//
	tfHwSkinningAllSProg.loadRsrc("shaders/TfHwSkinningPosNormTan.glsl");
	tfHwSkinningPosSProg.loadRsrc("shaders/TfHwSkinningPos.glsl");
}


//==============================================================================
// deform                                                                      =
//==============================================================================
void SkinsDeformer::deform(SkinPatchNode& node)
{
	ASSERT(node.getParent() != NULL); // The SkinPatchNodes always have parent
	ASSERT(static_cast<SceneNode*>(node.getParent())->getSceneNodeType() ==
		SceneNode::SNT_SKIN); // And their parent must be

	SkinNode* skinNode = static_cast<SkinNode*>(node.getParent());

	GlStateMachineSingleton::getInstance().enable(GL_RASTERIZER_DISCARD);

	// Chose sProg
	const ShaderProgram* sProg;

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
	sProg->getUniformVariable("skinningRotations").set(
		&skinNode->getBoneRotations()[0], skinNode->getBoneRotations().size());

	sProg->getUniformVariable("skinningTranslations").set(
		&skinNode->getBoneTranslations()[0],
		skinNode->getBoneTranslations().size());

	node.getTfVao().bind();

	// TF
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0,
		node.getTfVbo(SkinPatchNode::TFV_POSITIONS).getGlId());

	if(sProg == tfHwSkinningAllSProg.get())
	{
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1,
			node.getTfVbo(SkinPatchNode::TFV_NORMALS).getGlId());
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2,
			node.getTfVbo(SkinPatchNode::TFV_TANGENTS).getGlId());
	}

	//glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, this->Query);
	glBeginTransformFeedback(GL_POINTS);
		glDrawArrays(GL_POINTS, 0,
			node.getModelPatchRsrc().getMesh().getVertsNum());
	glEndTransformFeedback();
	//glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

	GlStateMachineSingleton::getInstance().disable(GL_RASTERIZER_DISCARD);
}


} // end namespace







