#include "SceneDrawer.h"
#include "Math/Math.h"
#include "Resources/Material.h"
#include "Scene/RenderableNode.h"
#include "Scene/Camera.h"
#include "../Renderer.h"
#include "Core/App.h"
#include "Scene/Scene.h"
#include "Scene/MaterialRuntime.h"
#include "GfxApi/GlStateMachine.h"


namespace R {


//==============================================================================
// Constructor                                                                 =
//==============================================================================
SceneDrawer::UsrDefVarVisitor::UsrDefVarVisitor(
	const MaterialRuntimeUserDefinedVar& udvr_,
	const Renderer& r_, uint& texUnit_)
:	udvr(udvr_),
	r(r_),
	texUnit(texUnit_)
{}


//==============================================================================
// Visitor functors                                                            =
//==============================================================================

template<typename Type>
void SceneDrawer::UsrDefVarVisitor::operator()(const Type& x) const
{
	udvr.getUniVar().set(&x);
}


template<>
void SceneDrawer::UsrDefVarVisitor::operator()(const RsrcPtr<Texture>* x) const
{
	const RsrcPtr<Texture>& texPtr = *x;
	texPtr->setRepeat(true);
	udvr.getUniVar().set(*texPtr, texUnit);
	++texUnit;
}


//==============================================================================
// setupShaderProg                                                             =
//==============================================================================
void SceneDrawer::setupShaderProg(const MaterialRuntime& mtlr,
	PassType passType,
	const Transform& nodeWorldTransform, const Camera& cam,
	const Renderer& r, float blurring)
{
	typedef MaterialBuildinVariable Mvb; // Short name
	uint textureUnit = 0;

	const Material& mtl = mtlr.getMaterial();
	const ShaderProgram& sprog = mtl.getShaderProgram(passType);

	sprog.bind();

	//
	// FFP stuff
	//
	GlStateMachineSingleton::getInstance().enable(GL_BLEND,
		mtlr.isBlendingEnabled());
	if(mtlr.isBlendingEnabled())
	{
		glBlendFunc(mtlr.getBlendingSfactor(), mtlr.getBlendingDfactor());
	}

	GlStateMachineSingleton::getInstance().enable(GL_DEPTH_TEST,
		mtlr.isDepthTestingEnabled());

	if(mtlr.isWireframeEnabled())
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}


	//
	// calc needed matrices
	//
	Mat4 modelMat(nodeWorldTransform);
	const Mat4& projectionMat = cam.getProjectionMatrix();
	const Mat4& viewMat = cam.getViewMatrix();
	Mat4 modelViewMat;
	Mat3 normalMat;
	Mat4 modelViewProjectionMat;

	// should I calculate the modelViewMat ?
	if(mtl.buildinVariableExits(Mvb::MODELVIEW_MAT, passType) ||
		mtl.buildinVariableExits(Mvb::MODELVIEWPROJECTION_MAT, passType) ||
		mtl.buildinVariableExits(Mvb::NORMAL_MAT, passType))
	{
		// Optimization
		if(modelMat == Mat4::getIdentity())
		{
			modelViewMat = viewMat;
		}
		else
		{
			modelViewMat = Mat4::combineTransformations(viewMat, modelMat);
		}
	}

	// set matrices
	if(mtl.buildinVariableExits(Mvb::MODEL_MAT, passType))
	{
		mtl.getBuildinVariable(Mvb::MODEL_MAT).
			getShaderProgramUniformVariable(passType).set(&modelMat);
	}

	/// XXX

	if(mtlr.getStdUniVar(Mvb::VIEW_MAT))
	{
		mtlr.getStdUniVar(Mvb::VIEW_MAT)->set(&viewMat);
	}

	if(mtlr.getStdUniVar(Mvb::PROJECTION_MAT))
	{
		mtlr.getStdUniVar(Mvb::PROJECTION_MAT)->set(&projectionMat);
	}

	if(mtlr.getStdUniVar(Mvb::MODELVIEW_MAT))
	{
		mtlr.getStdUniVar(Mvb::MODELVIEW_MAT)->set(&modelViewMat);
	}

	if(mtlr.getStdUniVar(Mvb::VIEWPROJECTION_MAT))
	{
		mtlr.getStdUniVar(Mvb::VIEWPROJECTION_MAT)->set(
			&r.getViewProjectionMat());
	}

	if(mtlr.getStdUniVar(Mvb::NORMAL_MAT))
	{
		normalMat = modelViewMat.getRotationPart();
		mtlr.getStdUniVar(Mvb::NORMAL_MAT)->set(&normalMat);
	}

	if(mtlr.getStdUniVar(Mvb::MODELVIEWPROJECTION_MAT))
	{
		modelViewProjectionMat = projectionMat * modelViewMat;
		mtlr.getStdUniVar(Mvb::MODELVIEWPROJECTION_MAT)->set(
			&modelViewProjectionMat);
	}


	//
	// FAis
	//
	if(mtlr.getStdUniVar(Mvb::MS_NORMAL_FAI))
	{
		mtlr.getStdUniVar(Mvb::MS_NORMAL_FAI)->set(
			r.getMs().getNormalFai(), textureUnit++);
	}

	if(mtlr.getStdUniVar(Mvb::MS_DIFFUSE_FAI))
	{
		mtlr.getStdUniVar(Mvb::MS_DIFFUSE_FAI)->set(
			r.getMs().getDiffuseFai(), textureUnit++);
	}

	if(mtlr.getStdUniVar(Mvb::MS_SPECULAR_FAI))
	{
		mtlr.getStdUniVar(Mvb::MS_SPECULAR_FAI)->set(
			r.getMs().getSpecularFai(), textureUnit++);
	}

	if(mtlr.getStdUniVar(Mvb::MS_DEPTH_FAI))
	{
		mtlr.getStdUniVar(Mvb::MS_DEPTH_FAI)->set(
			r.getMs().getDepthFai(), textureUnit++);
	}

	if(mtlr.getStdUniVar(Mvb::IS_FAI))
	{
		mtlr.getStdUniVar(Mvb::IS_FAI)->set(r.getIs().getFai(),
			textureUnit++);
	}

	if(mtlr.getStdUniVar(Mvb::PPS_PRE_PASS_FAI))
	{
		mtlr.getStdUniVar(Mvb::PPS_PRE_PASS_FAI)->set(
			r.getPps().getPrePassFai(), textureUnit++);
	}

	if(mtlr.getStdUniVar(Mvb::PPS_POST_PASS_FAI))
	{
		mtlr.getStdUniVar(Mvb::PPS_POST_PASS_FAI)->set(
			r.getPps().getPostPassFai(), textureUnit++);
	}


	//
	// Other
	//
	if(mtlr.getStdUniVar(Mvb::RENDERER_SIZE))
	{
		Vec2 v(r.getWidth(), r.getHeight());
		mtlr.getStdUniVar(Mvb::RENDERER_SIZE)->set(&v);
	}

	if(mtlr.getStdUniVar(Mvb::SCENE_AMBIENT_COLOR))
	{
		Vec3 col(SceneSingleton::getInstance().getAmbientCol());
		mtlr.getStdUniVar(Mvb::SCENE_AMBIENT_COLOR)->set(&col);
	}

	if(mtlr.getStdUniVar(Mvb::BLURRING))
	{
		/*blurring *= 10.0;
		INFO(blurring);*/
		float b = blurring;
		mtlr.getStdUniVar(Mvb::BLURRING)->set(&b);
	}


	//
	// set user defined vars
	//
	BOOST_FOREACH(const MaterialRuntimeUserDefinedVar& udvr,
		mtlr.getUserDefinedVars())
	{
		boost::apply_visitor(UsrDefVarVisitor(udvr, r, textureUnit),
			udvr.getDataVariant());
	}

	ON_GL_FAIL_THROW_EXCEPTION();
}


//==============================================================================
// renderRenderableNode                                                        =
//==============================================================================
void SceneDrawer::renderRenderableNode(const RenderableNode& renderable,
	const Camera& cam, RenderingPassType rtype) const
{
	const MaterialRuntime* mtlr;
	const Vao* vao;
	float blurring = 0.0;

	switch(rtype)
	{
		case RPT_COLOR:
			mtlr = &renderable.getCpMtlRun();
			vao = &renderable.getCpVao();

			blurring = (renderable.getWorldTransform().getOrigin() -
				renderable.getPrevWorldTransform().getOrigin()).getLength();

			break;

		case RPT_DEPTH:
			mtlr = &renderable.getDpMtlRun();
			vao = &renderable.getDpVao();
			break;
	}

	setupShaderProg(*mtlr, renderable.getWorldTransform(), cam, r, blurring);

	vao->bind();
	glDrawElements(GL_TRIANGLES, renderable.getVertIdsNum(),
		GL_UNSIGNED_SHORT, 0);
	vao->unbind();
}


} // end namespace

