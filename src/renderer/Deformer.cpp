#include "anki/renderer/Deformer.h"
#include "anki/resource/ShaderProgramResource.h"
#include "anki/resource/Material.h"
#include "anki/scene/SkinNode.h"
#include "anki/gl/GlState.h"

namespace anki {

//==============================================================================
Deformer::~Deformer()
{}

//==============================================================================
void Deformer::init()
{
	// Load the shaders
	//
	tfHwSkinningAllSProg.load("shaders/TfHwSkinningPosNormTan.glsl");
	tfHwSkinningPosSProg.load("shaders/TfHwSkinningPos.glsl");
}

//==============================================================================
void Deformer::deform(SkinNode& skinNode, SkinPatchNode& node) const
{
	ANKI_ASSERT(node.getMovable()->getParent() != NULL
		&& "The SkinPatchNode should always have parent");

	GlStateSingleton::get().enable(GL_RASTERIZER_DISCARD);

	// Chose sProg
	//
	const ShaderProgram* sProg;

	/*if(mtl.variableExistsAndInKey("normal", PassLevelKey(0, 0))
		&& mtl.variableExistsAndInKey("tangent", PassLevelKey(0, 0)))
	{*/
		sProg = tfHwSkinningAllSProg.get();
	/*}
	else
	{
		sProg = tfHwSkinningPosSProg.get();
	}*/

	sProg->bind();

	// Uniforms
	//
	sProg->findUniformVariable("skinningRotations").set(
		&skinNode.getBoneRotations()[0], skinNode.getBoneRotations().size());

	sProg->findUniformVariable("skinningTranslations").set(
		&skinNode.getBoneTranslations()[0],
		skinNode.getBoneTranslations().size());

	SkinModelPatch& smp = node.getSkinModelPatch();

	smp.getTransformFeedbackVao().bind();

	// TF
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0,
		smp.getSkinMesh().getTransformFeedbackVbo(
		SkinMesh::VBO_TF_POSITIONS)->getGlId());

	if(sProg == tfHwSkinningAllSProg.get())
	{
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1,
			smp.getSkinMesh().getTransformFeedbackVbo(
			SkinMesh::VBO_TF_NORMALS)->getGlId());
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2,
			smp.getSkinMesh().getTransformFeedbackVbo(
			SkinMesh::VBO_TF_TANGENTS)->getGlId());
	}

	glBeginTransformFeedback(GL_POINTS);
		glDrawArrays(GL_POINTS, 0,
			smp.getSkinMesh().getVerticesNumber(0));
	glEndTransformFeedback();

	GlStateSingleton::get().disable(GL_RASTERIZER_DISCARD);
}

} // end namespace
