#include "anki/renderer/SceneDrawer.h"
#include "anki/math/Math.h"
#include "anki/resource/Material.h"
#include "anki/scene/RenderableNode.h"
#include "anki/scene/Camera.h"
#include "anki/renderer/Renderer.h"
#include "anki/core/App.h"
#include "anki/scene/Scene.h"
#include "anki/scene/MaterialRuntime.h"
#include "anki/scene/MaterialRuntimeVariable.h"
#include "anki/gl/GlStateMachine.h"


namespace anki {


//==============================================================================
// Constructor                                                                 =
//==============================================================================
SceneDrawer::UsrDefVarVisitor::UsrDefVarVisitor(
	const MaterialRuntimeVariable& udvr_,
	const Renderer& r_, PassType pt_, uint& texUnit_)
:	udvr(udvr_),
	r(r_),
	pt(pt_),
	texUnit(texUnit_)
{}


//==============================================================================
// Visitor functors                                                            =
//==============================================================================

template<typename Type>
void SceneDrawer::UsrDefVarVisitor::operator()(const Type& x) const
{
	udvr.getMaterialUserVariable().getShaderProgramUniformVariable(pt).set(&x);
}


void SceneDrawer::UsrDefVarVisitor::operator()(const RsrcPtr<Texture>* x) const
{
	const RsrcPtr<Texture>& texPtr = *x;
	texPtr->setRepeat(true);
	udvr.getMaterialUserVariable().getShaderProgramUniformVariable(pt).
		set(*texPtr, texUnit);
	++texUnit;
}


//==============================================================================
// setupShaderProg                                                             =
//==============================================================================
void SceneDrawer::setupShaderProg(
	const MaterialRuntime& mtlr,
	PassType pt,
	const Transform& nodeWorldTransform,
	const Camera& cam,
	const Renderer& r,
	float blurring)
{
	typedef MaterialBuildinVariable Mvb; // Short name
	uint textureUnit = 0;
	GlStateMachine& gl = GlStateMachineSingleton::get();
	const Material& mtl = mtlr.getMaterial();
	const ShaderProgram& sprog = mtl.getShaderProgram(pt);

	sprog.bind();

	//
	// FFP stuff
	//
	gl.enable(GL_BLEND, mtlr.isBlendingEnabled());
	if(mtlr.isBlendingEnabled())
	{
		glBlendFunc(mtlr.getBlendingSFactor(), mtlr.getBlendingDFactor());
	}

	gl.enable(GL_DEPTH_TEST, mtlr.getDepthTesting());

	if(mtlr.getWireframe())
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}


	//
	// Calc needed matrices
	//
	Mat4 modelMat(nodeWorldTransform);
	const Mat4& projectionMat = cam.getProjectionMatrix();
	const Mat4& viewMat = cam.getViewMatrix();
	Mat4 modelViewMat;
	Mat3 normalMat;
	Mat4 modelViewProjectionMat;

	// should I calculate the modelViewMat ?
	if(mtl.buildinVariableExits(Mvb::MV_MODELVIEW_MAT, pt) ||
		mtl.buildinVariableExits(Mvb::MV_MODELVIEWPROJECTION_MAT, pt) ||
		mtl.buildinVariableExits(Mvb::MV_NORMAL_MAT, pt))
	{
		// Optimization
		modelViewMat = (modelMat == Mat4::getIdentity()) ? viewMat :
			Mat4::combineTransformations(viewMat, modelMat);
	}

	// set matrices
	if(mtl.buildinVariableExits(Mvb::MV_MODEL_MAT, pt))
	{
		mtl.getBuildinVariable(Mvb::MV_MODEL_MAT).
			getShaderProgramUniformVariable(pt).set(&modelMat);
	}

	if(mtl.buildinVariableExits(Mvb::MV_VIEW_MAT, pt))
	{
		mtl.getBuildinVariable(Mvb::MV_VIEW_MAT).
			getShaderProgramUniformVariable(pt).set(&viewMat);
	}

	if(mtl.buildinVariableExits(Mvb::MV_PROJECTION_MAT, pt))
	{
		mtl.getBuildinVariable(Mvb::MV_PROJECTION_MAT).
			getShaderProgramUniformVariable(pt).set(&projectionMat);
	}

	if(mtl.buildinVariableExits(Mvb::MV_MODELVIEW_MAT, pt))
	{
		mtl.getBuildinVariable(Mvb::MV_MODELVIEW_MAT).
			getShaderProgramUniformVariable(pt).set(&modelViewMat);
	}

	if(mtl.buildinVariableExits(Mvb::MV_VIEWPROJECTION_MAT, pt))
	{
		mtl.getBuildinVariable(Mvb::MV_VIEWPROJECTION_MAT).
			getShaderProgramUniformVariable(pt).set(&r.getViewProjectionMat());
	}

	if(mtl.buildinVariableExits(Mvb::MV_NORMAL_MAT, pt))
	{
		normalMat = modelViewMat.getRotationPart();

		mtl.getBuildinVariable(Mvb::MV_NORMAL_MAT).
			getShaderProgramUniformVariable(pt).set(&normalMat);
	}

	if(mtl.buildinVariableExits(Mvb::MV_MODELVIEWPROJECTION_MAT, pt))
	{
		modelViewProjectionMat = projectionMat * modelViewMat;

		mtl.getBuildinVariable(Mvb::MV_MODELVIEWPROJECTION_MAT).
			getShaderProgramUniformVariable(pt).set(&modelViewProjectionMat);
	}


	//
	// FAis
	//
	if(mtl.buildinVariableExits(Mvb::MV_MS_NORMAL_FAI, pt))
	{
		mtl.getBuildinVariable(Mvb::MV_MS_NORMAL_FAI).
			getShaderProgramUniformVariable(pt).set(
			r.getMs().getNormalFai(), textureUnit++);
	}

	if(mtl.buildinVariableExits(Mvb::MV_MS_DIFFUSE_FAI, pt))
	{
		mtl.getBuildinVariable(Mvb::MV_MS_DIFFUSE_FAI).
			getShaderProgramUniformVariable(pt).set(
			r.getMs().getDiffuseFai(), textureUnit++);
	}

	if(mtl.buildinVariableExits(Mvb::MV_MS_SPECULAR_FAI, pt))
	{
		mtl.getBuildinVariable(Mvb::MV_MS_SPECULAR_FAI).
			getShaderProgramUniformVariable(pt).set(
			r.getMs().getSpecularFai(), textureUnit++);
	}

	if(mtl.buildinVariableExits(Mvb::MV_MS_DEPTH_FAI, pt))
	{
		mtl.getBuildinVariable(Mvb::MV_MS_DEPTH_FAI).
			getShaderProgramUniformVariable(pt).set(
			r.getMs().getDepthFai(), textureUnit++);
	}

	if(mtl.buildinVariableExits(Mvb::MV_IS_FAI, pt))
	{
		mtl.getBuildinVariable(Mvb::MV_IS_FAI).
			getShaderProgramUniformVariable(pt).set(
			r.getIs().getFai(), textureUnit++);
	}

	if(mtl.buildinVariableExits(Mvb::MV_PPS_PRE_PASS_FAI, pt))
	{
		mtl.getBuildinVariable(Mvb::MV_PPS_PRE_PASS_FAI).
			getShaderProgramUniformVariable(pt).set(
			r.getPps().getPrePassFai(), textureUnit++);
	}

	if(mtl.buildinVariableExits(Mvb::MV_PPS_POST_PASS_FAI, pt))
	{
		mtl.getBuildinVariable(Mvb::MV_PPS_POST_PASS_FAI).
			getShaderProgramUniformVariable(pt).set(
			r.getPps().getPostPassFai(), textureUnit++);
	}


	//
	// Other
	//
	if(mtl.buildinVariableExits(Mvb::MV_RENDERER_SIZE, pt))
	{
		Vec2 v(r.getWidth(), r.getHeight());
		mtl.getBuildinVariable(Mvb::MV_RENDERER_SIZE).
			getShaderProgramUniformVariable(pt).set(&v);
	}

	if(mtl.buildinVariableExits(Mvb::MV_SCENE_AMBIENT_COLOR, pt))
	{
		Vec3 col(SceneSingleton::get().getAmbientColor());
		mtl.getBuildinVariable(Mvb::MV_SCENE_AMBIENT_COLOR).
			getShaderProgramUniformVariable(pt).set(&col);
	}

	if(mtl.buildinVariableExits(Mvb::MV_BLURRING, pt))
	{
		/*blurring *= 10.0;
		ANKI_INFO(blurring);*/
		float b = blurring;
		mtl.getBuildinVariable(Mvb::MV_BLURRING).
			getShaderProgramUniformVariable(pt).set(&b);
	}


	//
	// set user defined vars
	//
	BOOST_FOREACH(const MaterialRuntimeVariable& udvr, mtlr.getVariables())
	{
		if(!udvr.getMaterialUserVariable().inPass(pt))
		{
			continue;
		}

		boost::apply_visitor(UsrDefVarVisitor(udvr, r, pt, textureUnit),
			udvr.getDataVariant());
	}

	ANKI_CHECK_GL_ERROR();
}


//==============================================================================
// renderRenderableNode                                                        =
//==============================================================================
void SceneDrawer::renderRenderableNode(const RenderableNode& node,
	const Camera& cam, PassType pt) const
{
	float blurring = 0.0;
	const MaterialRuntime& mtlr = node.getMaterialRuntime();
	const Material& mtl = mtlr.getMaterial();

	// Calc the blur if needed
	if(mtl.buildinVariableExits(MaterialBuildinVariable::MV_BLURRING, pt))
	{
		float prev = (node.getPrevWorldTransform().getOrigin() -
			cam.getPrevWorldTransform().getOrigin()).getLength();

		float crnt = (node.getWorldTransform().getOrigin() -
			cam.getWorldTransform().getOrigin()).getLength();

		blurring = abs(crnt - prev);
	}

	setupShaderProg(mtlr, pt, node.getWorldTransform(), cam, r, blurring);
	node.getVao(pt).bind();
	glDrawElements(GL_TRIANGLES, node.getVertIdsNum(), GL_UNSIGNED_SHORT, 0);
	node.getVao(pt).unbind();
}


} // end namespace
