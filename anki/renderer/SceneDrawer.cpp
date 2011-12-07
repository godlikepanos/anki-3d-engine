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
SceneDrawer::UsrDefVarVisitor::UsrDefVarVisitor(
	const MaterialRuntimeVariable& udvr_,
	const Renderer& r_, const PassLevelKey& pt_, uint& texUnit_)
	: udvr(udvr_), r(r_), key(pt_), texUnit(texUnit_)
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

	// normalMat?
	if(mtl.variableExistsAndInKey("normalMat", pt))
	{
		normalMat = modelViewMat.getRotationPart();
	}

	// modelViewProjectionMat?
	if(mtl.variableExistsAndInKey("modelViewProjectionMat", pt))
	{
		modelViewProjectionMat = projectionMat * modelViewMat;
	}

	//
	// For all the vars
	//
	BOOST_FOREACH(const MaterialRuntimeVariable& mrvar, mtlr.getVariables())
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
				uni.set(modelViewMat);
				break;
			case B_VIEW_PROJECTION_MAT:
				uni.set(viewProjectionMat);
				break;
			case B_NORMAL_MAT:
				uni.set(normalMat);
				break;
			case B_MODEL_VIEW_PROJECTION_MAT:
				uni.set(modelViewProjectionMat);
				break;
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
				uni.set(r.getPps().getPreFai(), textureUnit++);
				break;
			case B_PPS_POST_PASS_FAI:
				uni.set(r.getPps().getPostFai(), textureUnit++);
				break;
			case B_RENDERER_SIZE:
			{
				Vec2 v(r.getWidth(), r.getHeight());
				uni.set(v);
				break;
			}
			case B_SCENE_AMBIENT_COLOR:
				uni.set(SceneSingleton::get().getAmbientColor());
				break;
			case B_BLURRING:
				uni.set(blurring);
				break;
			case B_NUM:
				/// XXX
				break;
		}
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
