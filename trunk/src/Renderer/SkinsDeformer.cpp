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
	std::string all = ShaderProg::createSrcCodeToCache("shaders/TfHwSkinningGeneric.glsl",
	                                                   "#define NORMAL_ENABLED\n#define TANGENT_ENABLED\n",
	                                                   "pnt");

	std::string p = ShaderProg::createSrcCodeToCache("shaders/TfHwSkinningGeneric.glsl",
	                                                 "",
	                                                 "p");

	tfHwSkinningAllSProg.loadRsrc(all.c_str());
	tfHwSkinningPosSProg.loadRsrc(p.c_str());

	const char* vars[] = {"vPosition", "vNormal", "vTangent"};

	// All
	tfHwSkinningAllSProg->bind();
	glTransformFeedbackVaryings(tfHwSkinningAllSProg->getGlId(), 3, vars, GL_SEPARATE_ATTRIBS);
	tfHwSkinningAllSProg->relink();

	// Pos
	tfHwSkinningPosSProg->bind();
	glTransformFeedbackVaryings(tfHwSkinningAllSProg->getGlId(), 1, vars, GL_SEPARATE_ATTRIBS);
	tfHwSkinningPosSProg->relink();
}


//======================================================================================================================
// deform                                                                                                              =
//======================================================================================================================
void SkinsDeformer::deform(SkinPatchNode& node)
{
	ASSERT(node.getParent() == NULL);
	ASSERT(static_cast<SceneNode*>(node.getParent())->getSceneNodeType() == SceneNode::SNT_SKIN);

	SkinNode* skinNode = static_cast<SkinNode*>(node.getParent());

	glEnable(GL_RASTERIZER_DISCARD);

	tfHwSkinningAllSProg->bind();

	// Uniforms
	tfHwSkinningAllSProg->findUniVar("skinningRotations")->setMat3(&skinNode->getBoneRotations()[0],
	                                                               skinNode->getBoneRotations().size());

	tfHwSkinningAllSProg->findUniVar("skinningTranslations")->setVec3(&skinNode->getBoneTranslations()[0],
	                                                                  skinNode->getBoneTranslations().size());

	// TF
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, node.getTfVbo(SkinPatchNode::TFV_POSITIONS).getGlId());
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, node.getTfVbo(SkinPatchNode::TFV_NORMALS).getGlId());
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, node.getTfVbo(SkinPatchNode::TFV_TANGENTS).getGlId());

	node.getTfVao().bind();

	//glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, this->Query);
	glBeginTransformFeedback(GL_TRIANGLES);
		glDrawElements(GL_TRIANGLES, node.getVertIdsNum(), GL_UNSIGNED_SHORT, 0);
	glEndTransformFeedback();
	//glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

	glDisable(GL_RASTERIZER_DISCARD);
}
