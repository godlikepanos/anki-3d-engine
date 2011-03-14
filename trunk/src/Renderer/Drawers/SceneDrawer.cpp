#include "SceneDrawer.h"
#include "Math.h"
#include "Material.h"
#include "RenderableNode.h"
#include "Camera.h"
#include "Renderer.h"
#include "App.h"
#include "Scene.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
SceneDrawer::UsrDefVarVisitor::UsrDefVarVisitor(const MtlUserDefinedVar& udv_, const Renderer& r_, uint& texUnit_):
	udv(udv_),
	r(r_),
	texUnit(texUnit_)
{}


//======================================================================================================================
// Visitor functors                                                                                                    =
//======================================================================================================================

void SceneDrawer::UsrDefVarVisitor::operator()(float x) const
{
	udv.getUniVar().setFloat(x);
}

void SceneDrawer::UsrDefVarVisitor::operator()(const Vec2& x) const
{
	udv.getUniVar().setVec2(&x);
}

void SceneDrawer::UsrDefVarVisitor::operator()(const Vec3& x) const
{
	udv.getUniVar().setVec3(&x);
}

void SceneDrawer::UsrDefVarVisitor::operator()(const Vec4& x) const
{
	udv.getUniVar().setVec4(&x);
}

void SceneDrawer::UsrDefVarVisitor::operator()(const RsrcPtr<Texture>& x) const
{
	x->setRepeat(true);
	udv.getUniVar().setTexture(*x, texUnit);
	++texUnit;
}

void SceneDrawer::UsrDefVarVisitor::operator()(MtlUserDefinedVar::Fai x) const
{
	switch(x)
	{
		case MtlUserDefinedVar::MS_DEPTH_FAI:
			udv.getUniVar().setTexture(r.getMs().getDepthFai(), texUnit);
			break;
		case MtlUserDefinedVar::IS_FAI:
			udv.getUniVar().setTexture(r.getIs().getFai(), texUnit);
			break;
		case MtlUserDefinedVar::PPS_PRE_PASS_FAI:
			udv.getUniVar().setTexture(r.getPps().getPrePassFai(), texUnit);
			break;
		case MtlUserDefinedVar::PPS_POST_PASS_FAI:
			udv.getUniVar().setTexture(r.getPps().getPostPassFai(), texUnit);
			break;
		default:
			ASSERT(0);
	}
	++texUnit;
}



//======================================================================================================================
// setupShaderProg                                                                                                     =
//======================================================================================================================
void SceneDrawer::setupShaderProg(const Material& mtl, const Transform& nodeWorldTransform, const Camera& cam,
                                  const Renderer& r)
{
	uint textureUnit = 0;

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
		mtl.getStdUniVar(Material::SUV_MODEL_MAT)->setMat4(&modelMat);
	}

	if(mtl.getStdUniVar(Material::SUV_VIEW_MAT))
	{
		mtl.getStdUniVar(Material::SUV_VIEW_MAT)->setMat4(&viewMat);
	}

	if(mtl.getStdUniVar(Material::SUV_PROJECTION_MAT))
	{
		mtl.getStdUniVar(Material::SUV_PROJECTION_MAT)->setMat4(&projectionMat);
	}

	if(mtl.getStdUniVar(Material::SUV_MODELVIEW_MAT))
	{
		mtl.getStdUniVar(Material::SUV_MODELVIEW_MAT)->setMat4(&modelViewMat);
	}

	if(mtl.getStdUniVar(Material::SUV_VIEWPROJECTION_MAT))
	{
		mtl.getStdUniVar(Material::SUV_VIEWPROJECTION_MAT)->setMat4(&r.getViewProjectionMat());
	}

	if(mtl.getStdUniVar(Material::SUV_NORMAL_MAT))
	{
		normalMat = modelViewMat.getRotationPart();
		mtl.getStdUniVar(Material::SUV_NORMAL_MAT)->setMat3(&normalMat);
	}

	if(mtl.getStdUniVar(Material::SUV_MODELVIEWPROJECTION_MAT))
	{
		modelViewProjectionMat = projectionMat * modelViewMat;
		mtl.getStdUniVar(Material::SUV_MODELVIEWPROJECTION_MAT)->setMat4(&modelViewProjectionMat);
	}


	//
	// FAis
	//
	if(mtl.getStdUniVar(Material::SUV_MS_NORMAL_FAI))
	{
		mtl.getStdUniVar(Material::SUV_MS_NORMAL_FAI)->setTexture(r.getMs().getNormalFai(), textureUnit++);
	}

	if(mtl.getStdUniVar(Material::SUV_MS_DIFFUSE_FAI))
	{
		mtl.getStdUniVar(Material::SUV_MS_DIFFUSE_FAI)->setTexture(r.getMs().getDiffuseFai(), textureUnit++);
	}

	if(mtl.getStdUniVar(Material::SUV_MS_SPECULAR_FAI))
	{
		mtl.getStdUniVar(Material::SUV_MS_SPECULAR_FAI)->setTexture(r.getMs().getSpecularFai(), textureUnit++);
	}

	if(mtl.getStdUniVar(Material::SUV_MS_DEPTH_FAI))
	{
		mtl.getStdUniVar(Material::SUV_MS_DEPTH_FAI)->setTexture(r.getMs().getDepthFai(), textureUnit++);
	}

	if(mtl.getStdUniVar(Material::SUV_IS_FAI))
	{
		mtl.getStdUniVar(Material::SUV_IS_FAI)->setTexture(r.getIs().getFai(), textureUnit++);
	}

	if(mtl.getStdUniVar(Material::SUV_PPS_PRE_PASS_FAI))
	{
		mtl.getStdUniVar(Material::SUV_PPS_PRE_PASS_FAI)->setTexture(r.getPps().getPrePassFai(), textureUnit++);
	}

	if(mtl.getStdUniVar(Material::SUV_PPS_POST_PASS_FAI))
	{
		mtl.getStdUniVar(Material::SUV_PPS_POST_PASS_FAI)->setTexture(r.getPps().getPostPassFai(), textureUnit++);
	}


	//
	// Other
	//
	if(mtl.getStdUniVar(Material::SUV_RENDERER_SIZE))
	{
		Vec2 v(r.getWidth(), r.getHeight());
		mtl.getStdUniVar(Material::SUV_RENDERER_SIZE)->setVec2(&v);
	}

	if(mtl.getStdUniVar(Material::SUV_SCENE_AMBIENT_COLOR))
	{
		Vec3 col(SceneSingleton::getInstance().getAmbientCol());
		mtl.getStdUniVar(Material::SUV_SCENE_AMBIENT_COLOR)->setVec3(&col);
	}


	//
	// set user defined vars
	//
	BOOST_FOREACH(const MtlUserDefinedVar& udv, mtl.getUserDefinedVars())
	{
		boost::apply_visitor(UsrDefVarVisitor(udv, r, textureUnit), udv.getDataVariant());
	}

	/*boost::ptr_vector<MtlUserDefinedVar>::const_iterator it = mtl.getUserDefinedVars().begin();
	for(; it !=  mtl.getUserDefinedVars().end(); it++)
	{
		const MtlUserDefinedVar& udv = *it;

		switch(udv.getUniVar().getGlDataType())
		{
			// texture or FAI
			case GL_SAMPLER_2D:
				RsrcPtr<Texture>* tex;
				MtlUserDefinedVar::Fai* fai;

				if(fai = udv.get<RsrcPtr<Texture> >())
				{
					udv.getTexture()->setRepeat(true);
					udv.getUniVar().setTexture(*udv.getTexture(), textureUnit);
				}
				else if(fai = udv.get< >())
				{
					switch(udv.getFai())
					{
						case MtlUserDefinedVar::MS_DEPTH_FAI:
							udv.getUniVar().setTexture(r.getMs().getDepthFai(), textureUnit);
							break;
						case MtlUserDefinedVar::IS_FAI:
							udv.getUniVar().setTexture(r.getIs().getFai(), textureUnit);
							break;
						case MtlUserDefinedVar::PPS_PRE_PASS_FAI:
							udv.getUniVar().setTexture(r.getPps().getPrePassFai(), textureUnit);
							break;
						case MtlUserDefinedVar::PPS_POST_PASS_FAI:
							udv.getUniVar().setTexture(r.getPps().getPostPassFai(), textureUnit);
							break;
						default:
							ASSERT(0);
					}
				}
				++textureUnit;
				break;
			// float
			case GL_FLOAT:
				udv.getUniVar().setFloat(udv.get<float>());
				break;
			// vec2
			case GL_FLOAT_VEC2:
				udv.getUniVar().setVec2(&udv.get<Vec2>());
				break;
			// vec3
			case GL_FLOAT_VEC3:
				udv.getUniVar().setVec3(&udv.get<Vec3>());
				break;
			// vec4
			case GL_FLOAT_VEC4:
				udv.getUniVar().setVec4(&udv.get<Vec4>());
				break;
		}
	}*/

	ON_GL_FAIL_THROW_EXCEPTION();
}


//======================================================================================================================
// renderRenderableNode                                                                                                =
//======================================================================================================================
void SceneDrawer::renderRenderableNode(const RenderableNode& renderable, const Camera& cam,
                                       RenderingPassType rtype) const
{
	const Material* mtl;
	const Vao* vao;

	switch(rtype)
	{
		case RPT_COLOR:
			mtl = &renderable.getCpMtl();
			vao = &renderable.getCpVao();
			break;

		case RPT_DEPTH:
			mtl = &renderable.getDpMtl();
			vao = &renderable.getDpVao();
			break;
	}

	setupShaderProg(*mtl, renderable.getWorldTransform(), cam, r);

	vao->bind();
	glDrawElements(GL_TRIANGLES, renderable.getVertIdsNum(), GL_UNSIGNED_SHORT, 0);
	vao->unbind();
}
