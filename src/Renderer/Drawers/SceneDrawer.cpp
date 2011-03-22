#include "SceneDrawer.h"
#include "Math.h"
#include "Material.h"
#include "RenderableNode.h"
#include "Camera.h"
#include "Renderer.h"
#include "App.h"
#include "Scene.h"
#include "MaterialRuntime.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
SceneDrawer::UsrDefVarVisitor::UsrDefVarVisitor(const MtlUserDefinedVarRuntime& udvr_,
                                                const Renderer& r_, uint& texUnit_):
	udvr(udvr_),
	r(r_),
	texUnit(texUnit_)
{}


//======================================================================================================================
// Visitor functors                                                                                                    =
//======================================================================================================================

void SceneDrawer::UsrDefVarVisitor::operator()(float x) const
{
	udvr.getUniVar().set(&x);
}

void SceneDrawer::UsrDefVarVisitor::operator()(const Vec2& x) const
{
	udvr.getUniVar().set(&x);
}

void SceneDrawer::UsrDefVarVisitor::operator()(const Vec3& x) const
{
	udvr.getUniVar().set(&x);
}

void SceneDrawer::UsrDefVarVisitor::operator()(const Vec4& x) const
{
	udvr.getUniVar().set(&x);
}

void SceneDrawer::UsrDefVarVisitor::operator()(const RsrcPtr<Texture>* x) const
{
	const RsrcPtr<Texture>& texPtr = *x;
	texPtr->setRepeat(true);
	udvr.getUniVar().set(*texPtr, texUnit);
	++texUnit;
}

void SceneDrawer::UsrDefVarVisitor::operator()(MtlUserDefinedVar::Fai x) const
{
	switch(x)
	{
		case MtlUserDefinedVar::MS_DEPTH_FAI:
			udvr.getUniVar().set(r.getMs().getDepthFai(), texUnit);
			break;
		case MtlUserDefinedVar::IS_FAI:
			udvr.getUniVar().set(r.getIs().getFai(), texUnit);
			break;
		case MtlUserDefinedVar::PPS_PRE_PASS_FAI:
			udvr.getUniVar().set(r.getPps().getPrePassFai(), texUnit);
			break;
		case MtlUserDefinedVar::PPS_POST_PASS_FAI:
			udvr.getUniVar().set(r.getPps().getPostPassFai(), texUnit);
			break;
		default:
			ASSERT(0);
	}
	++texUnit;
}



//======================================================================================================================
// setupShaderProg                                                                                                     =
//======================================================================================================================
void SceneDrawer::setupShaderProg(const MaterialRuntime& mtlr, const Transform& nodeWorldTransform, const Camera& cam,
                                  const Renderer& r)
{
	uint textureUnit = 0;

	const Material& mtl = mtlr.getMaterial();
	mtl.getShaderProg().bind();

	//
	// FFP stuff
	//
	if(mtl.isBlendingEnabled())
	{
		glEnable(GL_BLEND);
		//glDisable(GL_BLEND);
		glBlendFunc(mtl.getBlendingSfactor(), mtl.getBlendingDfactor());
	}
	else
	{
		glDisable(GL_BLEND);
	}


	if(mtl.isDepthTestingEnabled())
	{
		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}

	if(mtl.isWireframeEnabled())
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
	if(mtl.getStdUniVar(Material::SUV_MODELVIEW_MAT) ||
	   mtl.getStdUniVar(Material::SUV_MODELVIEWPROJECTION_MAT) ||
	   mtl.getStdUniVar(Material::SUV_NORMAL_MAT))
	{
		modelViewMat = Mat4::combineTransformations(viewMat, modelMat);
	}

	// set matrices
	if(mtl.getStdUniVar(Material::SUV_MODEL_MAT))
	{
		mtl.getStdUniVar(Material::SUV_MODEL_MAT)->set(&modelMat);
	}

	if(mtl.getStdUniVar(Material::SUV_VIEW_MAT))
	{
		mtl.getStdUniVar(Material::SUV_VIEW_MAT)->set(&viewMat);
	}

	if(mtl.getStdUniVar(Material::SUV_PROJECTION_MAT))
	{
		mtl.getStdUniVar(Material::SUV_PROJECTION_MAT)->set(&projectionMat);
	}

	if(mtl.getStdUniVar(Material::SUV_MODELVIEW_MAT))
	{
		mtl.getStdUniVar(Material::SUV_MODELVIEW_MAT)->set(&modelViewMat);
	}

	if(mtl.getStdUniVar(Material::SUV_VIEWPROJECTION_MAT))
	{
		mtl.getStdUniVar(Material::SUV_VIEWPROJECTION_MAT)->set(&r.getViewProjectionMat());
	}

	if(mtl.getStdUniVar(Material::SUV_NORMAL_MAT))
	{
		normalMat = modelViewMat.getRotationPart();
		mtl.getStdUniVar(Material::SUV_NORMAL_MAT)->set(&normalMat);
	}

	if(mtl.getStdUniVar(Material::SUV_MODELVIEWPROJECTION_MAT))
	{
		modelViewProjectionMat = projectionMat * modelViewMat;
		mtl.getStdUniVar(Material::SUV_MODELVIEWPROJECTION_MAT)->set(&modelViewProjectionMat);
	}


	//
	// FAis
	//
	if(mtl.getStdUniVar(Material::SUV_MS_NORMAL_FAI))
	{
		mtl.getStdUniVar(Material::SUV_MS_NORMAL_FAI)->set(r.getMs().getNormalFai(), textureUnit++);
	}

	if(mtl.getStdUniVar(Material::SUV_MS_DIFFUSE_FAI))
	{
		mtl.getStdUniVar(Material::SUV_MS_DIFFUSE_FAI)->set(r.getMs().getDiffuseFai(), textureUnit++);
	}

	if(mtl.getStdUniVar(Material::SUV_MS_SPECULAR_FAI))
	{
		mtl.getStdUniVar(Material::SUV_MS_SPECULAR_FAI)->set(r.getMs().getSpecularFai(), textureUnit++);
	}

	if(mtl.getStdUniVar(Material::SUV_MS_DEPTH_FAI))
	{
		mtl.getStdUniVar(Material::SUV_MS_DEPTH_FAI)->set(r.getMs().getDepthFai(), textureUnit++);
	}

	if(mtl.getStdUniVar(Material::SUV_IS_FAI))
	{
		mtl.getStdUniVar(Material::SUV_IS_FAI)->set(r.getIs().getFai(), textureUnit++);
	}

	if(mtl.getStdUniVar(Material::SUV_PPS_PRE_PASS_FAI))
	{
		mtl.getStdUniVar(Material::SUV_PPS_PRE_PASS_FAI)->set(r.getPps().getPrePassFai(), textureUnit++);
	}

	if(mtl.getStdUniVar(Material::SUV_PPS_POST_PASS_FAI))
	{
		mtl.getStdUniVar(Material::SUV_PPS_POST_PASS_FAI)->set(r.getPps().getPostPassFai(), textureUnit++);
	}


	//
	// Other
	//
	if(mtl.getStdUniVar(Material::SUV_RENDERER_SIZE))
	{
		Vec2 v(r.getWidth(), r.getHeight());
		mtl.getStdUniVar(Material::SUV_RENDERER_SIZE)->set(&v);
	}

	if(mtl.getStdUniVar(Material::SUV_SCENE_AMBIENT_COLOR))
	{
		Vec3 col(SceneSingleton::getInstance().getAmbientCol());
		mtl.getStdUniVar(Material::SUV_SCENE_AMBIENT_COLOR)->set(&col);
	}


	//
	// set user defined vars
	//
	BOOST_FOREACH(const MtlUserDefinedVarRuntime& udvr, mtlr.getUserDefinedVars())
	{
		boost::apply_visitor(UsrDefVarVisitor(udvr, r, textureUnit), udvr.getDataVariant());
	}

	ON_GL_FAIL_THROW_EXCEPTION();
}


//======================================================================================================================
// renderRenderableNode                                                                                                =
//======================================================================================================================
void SceneDrawer::renderRenderableNode(const RenderableNode& renderable, const Camera& cam,
                                       RenderingPassType rtype) const
{
	const MaterialRuntime* mtlr;
	const Vao* vao;

	switch(rtype)
	{
		case RPT_COLOR:
			mtlr = &renderable.getCpMtlRun();
			vao = &renderable.getCpVao();
			break;

		case RPT_DEPTH:
			mtlr = &renderable.getDpMtlRun();
			vao = &renderable.getDpVao();
			break;
	}

	setupShaderProg(*mtlr, renderable.getWorldTransform(), cam, r);

	vao->bind();
	glDrawElements(GL_TRIANGLES, renderable.getVertIdsNum(), GL_UNSIGNED_SHORT, 0);
	vao->unbind();
}
