#include "anki/renderer/SceneDrawer.h"
#include "anki/math/Math.h"
#include "anki/resource/Material.h"
#include "anki/scene/RenderableNode.h"
#include "anki/scene/Camera.h"
#include "anki/renderer/Renderer.h"
#include "anki/core/App.h"
#include "anki/scene/Scene.h"
#include "anki/scene/MaterialRuntime.h"
#include "anki/gl/GlStateMachine.h"
#include <boost/foreach.hpp>
#include <algorithm>


namespace anki {


//==============================================================================
boost::array<const char*, SceneDrawer::B_NUM> SceneDrawer::buildinsTxt = {{
	// Matrices
	"modelMat",
	"viewMat",
	"projectionMat",
	"modelViewMat",
	"viewProjectionMat",
	"normalMat",
	"modelViewProjectionMat",
	// FAIs (for materials in blending stage)
	"msNormalFai",
	"msDiffuseFai",
	"msSpecularFai",
	"msDepthFai",
	"isFai",
	"ppsPrePassFai",
	"ppsPostPassFai",
	// Other
	"rendererSize",
	"sceneAmbientColor",
	"blurring"
}};


//==============================================================================
SceneDrawer::SetUniformVisitor::SetUniformVisitor(
	const ShaderProgramUniformVariable& uni_, uint& texUnit_)
	: uni(uni_), texUnit(texUnit_)
{}


//==============================================================================
template<typename Type>
void SceneDrawer::SetUniformVisitor::operator()(const Type& x) const
{
	uni.set(x);
}


//==============================================================================
template<>
void SceneDrawer::SetUniformVisitor::operator()<TextureResourcePointer>(
	const TextureResourcePointer& x) const
{
	uni.set(*x, texUnit++);
}


//==============================================================================
void SceneDrawer::tryCalcModelViewMat(const Mat4& modelMat,
	const Mat4& viewMat, bool& calculated, Mat4& modelViewMat)
{
	if(!calculated)
	{
		modelViewMat = (modelMat == Mat4::getIdentity()) ? viewMat :
			Mat4::combineTransformations(viewMat, modelMat);

		calculated = true;
	}
}


//==============================================================================
void SceneDrawer::setupShaderProg(
	const PassLevelKey& key,
	const Transform& nodeWorldTransform,
	const Transform& prevNodeWorldTransform,
	const Camera& cam,
	const Renderer& r,
	MaterialRuntime& mtlr)
{
	uint textureUnit = 0;
	GlStateMachine& gl = GlStateMachineSingleton::get();
	const Material& mtl = mtlr.getMaterial();
	const ShaderProgram& sprog = mtl.getShaderProgram(key);

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
	// Get needed matrices
	//
	Mat4 modelMat(nodeWorldTransform);
	const Mat4& projectionMat = cam.getProjectionMatrix();
	const Mat4& viewMat = cam.getViewMatrix();
	Mat4 modelViewMat;
	bool modelViewMatCalculated = false;

	//
	// For all the vars
	//
	BOOST_FOREACH(MaterialRuntimeVariable& mrvar, mtlr.getVariables())
	{
		const MaterialVariable& mvar = mrvar.getMaterialVariable();

		// Skip some
		if(!mvar.inPass(key))
		{
			continue;
		}

		// Set the buildinId for all
		if(mrvar.getBuildinId() == -1)
		{
			for(uint i = 0; i < B_NUM; i++)
			{
				if(mvar.getName() == buildinsTxt[i])
				{
					mrvar.setBuildinId(i);
					break;
				}
			}

			// If still -1 thn set it as B_NUM
			if(mrvar.getBuildinId() == -1)
			{
				mrvar.setBuildinId(B_NUM);
			}
		}

		// Sanity checks
		if(mrvar.getBuildinId() == B_NUM && !mvar.getInitialized())
		{
			ANKI_WARNING("Uninitialized variable: " << mvar.getName());
		}

		if(mrvar.getBuildinId() != B_NUM && mvar.getInitialized())
		{
			ANKI_WARNING("The renderer will override the given value for: " <<
				mvar.getName());
		}

		// Big switch
		const ShaderProgramUniformVariable& uni =
			mvar.getShaderProgramUniformVariable(key);

		switch(mrvar.getBuildinId())
		{
			case B_MODEL_MAT:
				uni.set(modelMat);
				break;
			case B_VIEW_MAT:
				uni.set(viewMat);
				break;
			case B_PROJECTION_MAT:
				uni.set(projectionMat);
				break;
			case B_MODEL_VIEW_MAT:
			{
				tryCalcModelViewMat(modelMat, viewMat, modelViewMatCalculated,
					modelViewMat);

				uni.set(modelViewMat);
				break;
			}
			case B_VIEW_PROJECTION_MAT:
				uni.set(r.getViewProjectionMat());
				break;
			case B_NORMAL_MAT:
			{
				tryCalcModelViewMat(modelMat, viewMat, modelViewMatCalculated,
					modelViewMat);

				Mat3 normalMat = modelViewMat.getRotationPart();
				uni.set(normalMat);
				break;
			}
			case B_MODEL_VIEW_PROJECTION_MAT:
			{
				tryCalcModelViewMat(modelMat, viewMat, modelViewMatCalculated,
					modelViewMat);

				Mat4 modelViewProjectionMat = projectionMat * modelViewMat;
				uni.set(modelViewProjectionMat);
				break;
			}
			case B_MS_NORMAL_FAI:
				uni.set(r.getMs().getNormalFai(), textureUnit++);
				break;
			case B_MS_DIFFUSE_FAI:
				uni.set(r.getMs().getDiffuseFai(), textureUnit++);
				break;
			case B_MS_SPECULAR_FAI:
				uni.set(r.getMs().getSpecularFai(), textureUnit++);
				break;
			case B_MS_DEPTH_FAI:
				uni.set(r.getMs().getDepthFai(), textureUnit++);
				break;
			case B_IS_FAI:
				uni.set(r.getIs().getFai(), textureUnit++);
				break;
			case B_PPS_PRE_PASS_FAI:
				uni.set(r.getPps().getPrePassFai(), textureUnit++);
				break;
			case B_PPS_POST_PASS_FAI:
				uni.set(r.getPps().getPostPassFai(), textureUnit++);
				break;
			case B_RENDERER_SIZE:
			{
				uni.set(Vec2(r.getWidth(), r.getHeight()));
				break;
			}
			case B_SCENE_AMBIENT_COLOR:
				uni.set(SceneSingleton::get().getAmbientColor());
				break;
			case B_BLURRING:
			{
				float prev = (prevNodeWorldTransform.getOrigin() -
					cam.getPrevWorldTransform().getOrigin()).getLength();

				float crnt = (nodeWorldTransform.getOrigin() -
					cam.getWorldTransform().getOrigin()).getLength();

				float blurring = abs(crnt - prev);
				uni.set(blurring);
				break;
			}
			case B_NUM:
			{
				// ATTENTION: Don't EVER use the mutable version of
				// MaterialRuntimeVariable::getVariant() here as it copies data
				const MaterialRuntimeVariable::Variant& v = mrvar.getVariant();
				boost::apply_visitor(SetUniformVisitor(uni, textureUnit), v);
				break;
			}
		}
	}

	ANKI_CHECK_GL_ERROR();
}


//==============================================================================
void SceneDrawer::renderRenderableNode(const Camera& cam,
	uint pass, RenderableNode& node) const
{
	MaterialRuntime& mtlr = node.getMaterialRuntime();
	const Material& mtl = mtlr.getMaterial();

	// Calc the LOD and the key
	float dist = (node.getWorldTransform().getOrigin() -
		cam.getWorldTransform().getOrigin()).getLength();
	uint lod = std::min(r.calculateLod(dist), mtl.getLevelsOfDetail() - 1);
	PassLevelKey key(pass, lod);

	// Setup shader
	setupShaderProg(key, node.getWorldTransform(),
		node.getPrevWorldTransform(), cam, r, mtlr);

	node.getVao(key).bind();
	glDrawElements(GL_TRIANGLES, node.getVertIdsNum(key), GL_UNSIGNED_SHORT, 0);
	node.getVao(key).unbind();
}


} // end namespace
