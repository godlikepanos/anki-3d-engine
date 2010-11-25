#include "Renderer.h"
#include "Camera.h"
#include "RendererInitializer.h"
#include "Material.h"
#include "App.h"
#include "Scene.h"
#include "Exception.h"
#include "ModelNode.h"
#include "Model.h"
#include "Mesh.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
Renderer::Renderer(Object* parent):
	Object(parent),
	width(640),
	height(480),
	ms(new Ms(*this, this)),
	is(new Is(*this, this)),
	pps(new Pps(*this, this)),
	bs(new Bs(*this, this))
{}


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Renderer::init(const RendererInitializer& initializer)
{
	// set from the initializer
	width = initializer.width;
	height = initializer.height;

	aspectRatio = float(width)/height;
	framesNum = 0;

	// a few sanity checks
	if(width < 10 || height < 10)
	{
		throw EXCEPTION("Incorrect sizes");
	}

	// init the stages. Careful with the order!!!!!!!!!!
	ms->init(initializer);
	is->init(initializer);
	/*pps.init(initializer);
	bs.init(initializer);*/

	// quad VBOs and VAO
	float quadVertCoords[][2] = {{1.0, 1.0}, {0.0, 1.0}, {0.0, 0.0}, {1.0, 0.0}};
	quadPositionsVbo = new Vbo(GL_ARRAY_BUFFER, sizeof(quadVertCoords), quadVertCoords, GL_STATIC_DRAW, this);

	ushort quadVertIndeces[2][3] = {{0, 1, 3}, {1, 2, 3}}; // 2 triangles
	quadVertIndecesVbo = new Vbo(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadVertIndeces), quadVertIndeces, GL_STATIC_DRAW, this);

	globalVao = new Vao(this);
	globalVao->attachArrayBufferVbo(*quadPositionsVbo, 0, 2, GL_FLOAT, false, 0, NULL);
	globalVao->attachElementArrayBufferVbo(*quadVertIndecesVbo);
}


//======================================================================================================================
// render                                                                                                              =
//======================================================================================================================
void Renderer::render(Camera& cam_)
{
	cam = &cam_;

	viewProjectionMat = cam->getProjectionMatrix() * cam->getViewMatrix();

	ms->run();
	is->run();
	/*pps.runPrePass();
	bs.run();
	pps.runPostPass();*/

	++framesNum;
}


//======================================================================================================================
// drawQuad                                                                                                            =
//======================================================================================================================
void Renderer::drawQuad()
{
	globalVao->bind();
	glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_SHORT, 0);
	globalVao->unbind();
	ON_GL_FAIL_THROW_EXCEPTION();
}


//======================================================================================================================
// setupMaterial                                                                                                       =
//======================================================================================================================
void Renderer::setupMaterial(const Material& mtl, const SceneNode& sceneNode, const Camera& cam) const
{
	mtl.getShaderProg().bind();
	uint textureUnit = 0;

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
	Mat4 modelMat(sceneNode.getWorldTransform());
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
		mtl.getStdUniVar(Material::SUV_VIEWPROJECTION_MAT)->setMat4(&viewProjectionMat);
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
		mtl.getStdUniVar(Material::SUV_MS_NORMAL_FAI)->setTexture(ms->getNormalFai(), textureUnit++);
	}

	if(mtl.getStdUniVar(Material::SUV_MS_DIFFUSE_FAI))
	{
		mtl.getStdUniVar(Material::SUV_MS_DIFFUSE_FAI)->setTexture(ms->getDiffuseFai(), textureUnit++);
	}

	if(mtl.getStdUniVar(Material::SUV_MS_SPECULAR_FAI))
	{
		mtl.getStdUniVar(Material::SUV_MS_SPECULAR_FAI)->setTexture(ms->getSpecularFai(), textureUnit++);
	}

	if(mtl.getStdUniVar(Material::SUV_MS_DEPTH_FAI))
	{
		mtl.getStdUniVar(Material::SUV_MS_DEPTH_FAI)->setTexture(ms->getDepthFai(), textureUnit++);
	}

	if(mtl.getStdUniVar(Material::SUV_IS_FAI))
	{
		mtl.getStdUniVar(Material::SUV_IS_FAI)->setTexture(is->getFai(), textureUnit++);
	}

	if(mtl.getStdUniVar(Material::SUV_PPS_PRE_PASS_FAI))
	{
		mtl.getStdUniVar(Material::SUV_PPS_PRE_PASS_FAI)->setTexture(pps->getPrePassFai(), textureUnit++);
	}

	if(mtl.getStdUniVar(Material::SUV_PPS_POST_PASS_FAI))
	{
		mtl.getStdUniVar(Material::SUV_PPS_POST_PASS_FAI)->setTexture(pps->getPostPassFai(), textureUnit++);
	}


	//
	// Other
	//
	if(mtl.getStdUniVar(Material::SUV_RENDERER_SIZE))
	{
		Vec2 v(width, height);
		mtl.getStdUniVar(Material::SUV_RENDERER_SIZE)->setVec2(&v);
	}

	if(mtl.getStdUniVar(Material::SUV_SCENE_AMBIENT_COLOR))
	{
		Vec3 col(app->getScene().getAmbientCol());
		mtl.getStdUniVar(Material::SUV_SCENE_AMBIENT_COLOR)->setVec3(&col);
	}


	//
	// set user defined vars
	//
	for(uint i=0; i<mtl.getUserDefinedVars().size(); i++)
	{
		const Material::UserDefinedUniVar* udv = &mtl.getUserDefinedVars()[i];
		switch(udv->sProgVar->getGlDataType())
		{
			// texture
			case GL_SAMPLER_2D:
				udv->sProgVar->setTexture(*udv->value.texture, textureUnit++);
				break;
			// float
			case GL_FLOAT:
				udv->sProgVar->setFloat(udv->value.float_);
				break;
			// vec2
			case GL_FLOAT_VEC2:
				udv->sProgVar->setVec2(&udv->value.vec2);
				break;
			// vec3
			case GL_FLOAT_VEC3:
				udv->sProgVar->setVec3(&udv->value.vec3);
				break;
			// vec4
			case GL_FLOAT_VEC4:
				udv->sProgVar->setVec4(&udv->value.vec4);
				break;
		}
	}

	ON_GL_FAIL_THROW_EXCEPTION();
}


//======================================================================================================================
// renderModelNode                                                                                                     =
//======================================================================================================================
void Renderer::renderModelNode(const ModelNode& modelNode, const Camera& cam, ModelNodeRenderType type) const
{
	for(uint i = 0; i < modelNode.getModel().getSubModels().size(); i++)
	{
		const Model::SubModel& subModel = modelNode.getModel().getSubModels()[i];

		const Material* mtl;
		const Vao* vao;
		if(type == MNRT_NORMAL)
		{
			mtl = &subModel.getMaterial();
			vao = &subModel.getVao();
		}
		else
		{
			mtl = &subModel.getDpMaterial();
			vao = &subModel.getDpVao();
		}

		// Material
		const SceneNode* sceneNode = &modelNode;
		setupMaterial(*mtl, *sceneNode, cam);

		// Render
		if(modelNode.hasSkeleton())
		{
			RASSERT_THROW_EXCEPTION(!mtl->hasHWSkinning()); // it has skel controller but no skinning

			// first the uniforms
			mtl->getStdUniVar(Material::SUV_SKINNING_ROTATIONS)->setMat3(&modelNode.getBoneRotations()[0],
			                                                             modelNode.getBoneRotations().size());

			mtl->getStdUniVar(Material::SUV_SKINNING_TRANSLATIONS)->setVec3(&modelNode.getBoneTranslations()[0],
			                                                                modelNode.getBoneTranslations().size());
		}

		vao->bind();
		glDrawElements(GL_TRIANGLES, subModel.getMesh().getVertIdsNum(), GL_UNSIGNED_SHORT, 0);
		vao->unbind();
	}
}


//======================================================================================================================
// unproject                                                                                                           =
//======================================================================================================================
Vec3 Renderer::unproject(const Vec3& windowCoords, const Mat4& modelViewMat, const Mat4& projectionMat,
                         const int view[4])
{
	Mat4 invPm = projectionMat * modelViewMat;
	invPm.invert();

	// the vec is in NDC space meaning: -1<=vec.x<=1 -1<=vec.y<=1 -1<=vec.z<=1
	Vec4 vec;
	vec.x = (2.0 * (windowCoords.x - view[0])) / view[2] - 1.0;
	vec.y = (2.0 * (windowCoords.y - view[1])) / view[3] - 1.0;
	vec.z = 2.0 * windowCoords.z - 1.0;
	vec.w = 1.0;

	Vec4 final = invPm * vec;
	final /= final.w;
	return Vec3(final);
}


//======================================================================================================================
// ortho                                                                                                               =
//======================================================================================================================
Mat4 Renderer::ortho(float left, float right, float bottom, float top, float near, float far)
{
	float difx = right-left;
	float dify = top-bottom;
	float difz = far-near;
	float tx = -(right+left) / difx;
	float ty = -(top+bottom) / dify;
	float tz = -(far+near) / difz;
	Mat4 m;

	m(0, 0) = 2.0 / difx;
	m(0, 1) = 0.0;
	m(0, 2) = 0.0;
	m(0, 3) = tx;
	m(1, 0) = 0.0;
	m(1, 1) = 2.0 / dify;
	m(1, 2) = 0.0;
	m(1, 3) = ty;
	m(2, 0) = 0.0;
	m(2, 1) = 0.0;
	m(2, 2) = -2.0 / difz;
	m(2, 3) = tz;
	m(3, 0) = 0.0;
	m(3, 1) = 0.0;
	m(3, 2) = 0.0;
	m(3, 3) = 1.0;

	return m;
}

