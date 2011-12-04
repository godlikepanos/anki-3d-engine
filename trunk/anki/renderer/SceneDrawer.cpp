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
#include <boost/foreach.hpp>


namespace anki {


//==============================================================================
// Constructor                                                                 =
//==============================================================================
SceneDrawer::UsrDefVarVisitor::UsrDefVarVisitor(
	const MaterialRuntimeVariable& udvr_,
	const Renderer& r_, const PassLevelKey& pt_, uint& texUnit_)
:	udvr(udvr_),
	r(r_),
	key(pt_),
	texUnit(texUnit_)
{}


//==============================================================================
// Visitor functors                                                            =
//==============================================================================

template<typename Type>
void SceneDrawer::UsrDefVarVisitor::operator()(const Type& x) const
{
	static_cast<const ShaderProgramUniformVariable&>(udvr.getMaterialVariable().
		getShaderProgramVariable(key)).set(x);
}


void SceneDrawer::UsrDefVarVisitor::operator()(
	const TextureResourcePointer* x) const
{
	const TextureResourcePointer& texPtr = *x;
	texPtr->setRepeat(true);

	static_cast<const ShaderProgramUniformVariable&>(udvr.getMaterialVariable().
		getShaderProgramVariable(key)).set(*texPtr, texUnit);

	++texUnit;
}


//==============================================================================
// setupShaderProg                                                             =
//==============================================================================
void SceneDrawer::setupShaderProg(
	const MaterialRuntime& mtlr,
	const PassLevelKey& pt,
	const Transform& nodeWorldTransform,
	const Camera& cam,
	const Renderer& r,
	float blurring)
{
	typedef MaterialVariable Mvb; // Short name
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
	if(mtl.variableExistsAndInKey("modelViewMat", pt) ||
		mtl.variableExistsAndInKey("modeViewProjectionMat", pt) ||
		mtl.variableExistsAndInKey("normalMat", pt))
	{
		// Optimization
		modelViewMat = (modelMat == Mat4::getIdentity()) ? viewMat :
			Mat4::combineTransformations(viewMat, modelMat);
	}

	// set matrices
	if(mtl.variableExistsAndInKey("modelMat", pt))
	{
		static_cast<const ShaderProgramUniformVariable&>(mtl.findVariableByName("modelMat").
			getShaderProgramVariable(pt)).set(modelMat);
	}

	if(mtl.variableExistsAndInKey("viewMat", pt))
	{
		static_cast<const ShaderProgramUniformVariable&>(mtl.findVariableByName("viewMat").
			getShaderProgramVariable(pt)).set(viewMat);
	}

	if(mtl.variableExistsAndInKey("projectionMat", pt))
	{
		static_cast<const ShaderProgramUniformVariable&>(mtl.findVariableByName("projectionMat").
			getShaderProgramVariable(pt)).set(projectionMat);
	}

	if(mtl.variableExistsAndInKey("modelViewMat", pt))
	{
		static_cast<const ShaderProgramUniformVariable&>(mtl.findVariableByName("modelMat").
			getShaderProgramVariable(pt)).set(modelViewMat);
	}

	if(mtl.variableExistsAndInKey("viewProjectionMat", pt))
	{
		static_cast<const ShaderProgramUniformVariable&>(mtl.findVariableByName("viewProjectionMat").
			getShaderProgramVariable(pt)).set(r.getViewProjectionMat());
	}

	if(mtl.variableExistsAndInKey("normalMat", pt))
	{
		normalMat = modelViewMat.getRotationPart();

		static_cast<const ShaderProgramUniformVariable&>(mtl.findVariableByName("normalMat").
			getShaderProgramVariable(pt)).set(normalMat);
	}

	if(mtl.variableExistsAndInKey("modelViewProjectionMat", pt))
	{
		modelViewProjectionMat = projectionMat * modelViewMat;

		static_cast<const ShaderProgramUniformVariable&>(mtl.findVariableByName("modelViewProjectionMat").
			getShaderProgramVariable(pt)).set(modelViewProjectionMat);
	}


	//
	// FAis
	//
	/*if(mtl.variableExistsAndInKey(Mvb::MV_MS_NORMAL_FAI, pt))
	{
		mtl.getBuildinVariable(Mvb::MV_MS_NORMAL_FAI).
			getShaderProgramUniformVariable(pt).set(
			r.getMs().getNormalFai(), textureUnit++);
	}

	if(mtl.variableExistsAndInKey(Mvb::MV_MS_DIFFUSE_FAI, pt))
	{
		mtl.getBuildinVariable(Mvb::MV_MS_DIFFUSE_FAI).
			getShaderProgramUniformVariable(pt).set(
			r.getMs().getDiffuseFai(), textureUnit++);
	}

	if(mtl.variableExistsAndInKey(Mvb::MV_MS_SPECULAR_FAI, pt))
	{
		mtl.getBuildinVariable(Mvb::MV_MS_SPECULAR_FAI).
			getShaderProgramUniformVariable(pt).set(
			r.getMs().getSpecularFai(), textureUnit++);
	}

	if(mtl.variableExistsAndInKey(Mvb::MV_MS_DEPTH_FAI, pt))
	{
		mtl.getBuildinVariable(Mvb::MV_MS_DEPTH_FAI).
			getShaderProgramUniformVariable(pt).set(
			r.getMs().getDepthFai(), textureUnit++);
	}

	if(mtl.variableExistsAndInKey(Mvb::MV_IS_FAI, pt))
	{
		mtl.getBuildinVariable(Mvb::MV_IS_FAI).
			getShaderProgramUniformVariable(pt).set(
			r.getIs().getFai(), textureUnit++);
	}

	if(mtl.variableExistsAndInKey(Mvb::MV_PPS_PRE_PASS_FAI, pt))
	{
		mtl.getBuildinVariable(Mvb::MV_PPS_PRE_PASS_FAI).
			getShaderProgramUniformVariable(pt).set(
			r.getPps().getPrePassFai(), textureUnit++);
	}

	if(mtl.variableExistsAndInKey(Mvb::MV_PPS_POST_PASS_FAI, pt))
	{
		mtl.getBuildinVariable(Mvb::MV_PPS_POST_PASS_FAI).
			getShaderProgramUniformVariable(pt).set(
			r.getPps().getPostPassFai(), textureUnit++);
	}


	//
	// Other
	//
	if(mtl.variableExistsAndInKey(Mvb::MV_RENDERER_SIZE, pt))
	{
		Vec2 v(r.getWidth(), r.getHeight());
		mtl.getBuildinVariable(Mvb::MV_RENDERER_SIZE).
			getShaderProgramUniformVariable(pt).set(&v);
	}

	if(mtl.variableExistsAndInKey(Mvb::MV_SCENE_AMBIENT_COLOR, pt))
	{
		Vec3 col(SceneSingleton::get().getAmbientColor());
		mtl.getBuildinVariable(Mvb::MV_SCENE_AMBIENT_COLOR).
			getShaderProgramUniformVariable(pt).set(&col);
	}

	if(mtl.variableExistsAndInKey(Mvb::MV_BLURRING, pt))
	{
		blurring *= 10.0;
		ANKI_INFO(blurring);
		float b = blurring;
		mtl.getBuildinVariable(Mvb::MV_BLURRING).
			getShaderProgramUniformVariable(pt).set(&b);
	}*/


	//
	// set user defined vars
	//
	BOOST_FOREACH(const MaterialRuntimeVariable& udvr, mtlr.getVariables())
	{
		if(!udvr.getMaterialVariable().inPass(pt) ||
			udvr.getMaterialVariable().getShaderProgramVariableType() ==
				ShaderProgramVariable::T_ATTRIBUTE)
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
	const Camera& cam, const PassLevelKey& key) const
{
	float blurring = 0.0;
	const MaterialRuntime& mtlr = node.getMaterialRuntime();
	const Material& mtl = mtlr.getMaterial();

	// Calc the blur if needed
	if(mtl.variableExistsAndInKey("blurring", key))
	{
		float prev = (node.getPrevWorldTransform().getOrigin() -
			cam.getPrevWorldTransform().getOrigin()).getLength();

		float crnt = (node.getWorldTransform().getOrigin() -
			cam.getWorldTransform().getOrigin()).getLength();

		blurring = abs(crnt - prev);
	}

	setupShaderProg(mtlr, key, node.getWorldTransform(), cam, r, blurring);
	node.getVao(key).bind();
	glDrawElements(GL_TRIANGLES, node.getVertIdsNum(key), GL_UNSIGNED_SHORT, 0);
	node.getVao(key).unbind();
}


} // end namespace
