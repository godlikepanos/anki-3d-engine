#include "anki/renderer/Drawer.h"
#include "anki/resource/ShaderProgramResource.h"
#include "anki/scene/Frustumable.h"
#include "anki/resource/Material.h"
#include "anki/scene/Renderable.h"
#include "anki/scene/Camera.h"
#include "anki/scene/ModelNode.h"
#include "anki/resource/TextureResource.h"
#include "anki/renderer/Renderer.h"

namespace anki {

//==============================================================================
/// Visitor that sets a uniform
struct SetupRenderableVariableVisitor
{
	const Frustumable* fr = nullptr;
	Renderer* r = nullptr;
	Renderable* renderable = nullptr;
	Array<U8, RenderableDrawer::UNIFORM_BLOCK_MAX_SIZE> clientBlock;
	RenderableVariable* rvar = nullptr;
	const ShaderProgramUniformVariable* uni;

	/// Set a uniform in a client block
	template<typename T>
	void uniSet(const ShaderProgramUniformVariable& uni,
		const T* value, U32 size)
	{
		ANKI_ASSERT(0);
	}

	template<typename TRenderableVariableTemplate>
	void visit(TRenderableVariableTemplate& x)
	{
		const U32 instancesCount = renderable->getRenderableInstancesCount();
		const U32 uniArrSize = uni->getSize();
		const U32 size = std::min(instancesCount, uniArrSize);

		// Set uniform
		//
		const Transform* trfs = renderable->getRenderableWorldTransforms();
		const Mat4& vp = fr->getViewProjectionMatrix();
		const Mat4& v = fr->getViewMatrix();

		const U32 maxInstances = 32;

		switch(x.getBuildinId())
		{
		case BMV_NO_BUILDIN:
			uniSet<typename TRenderableVariableTemplate::Type>(
				*uni, x.get(), x.getArraySize());
			break;
		case BMV_MVP_MATRIX:
			if(trfs)
			{
				Array<Mat4, maxInstances> mvp;

				for(U i = 0; i < size; i++)
				{
					mvp[i] = vp * Mat4(trfs[i]);
				}

				uniSet(*uni, &mvp[0], size);
			}
			else
			{
				ANKI_ASSERT(size == 1 && "Shouldn't instance that one");
				uniSet(*uni, &vp, 1);
			}
			break;
		case BMV_MV_MATRIX:
			{
				ANKI_ASSERT(trfs != nullptr);
				Array<Mat4, maxInstances> mv;

				for(U i = 0; i < size; i++)
				{
					mv[i] = v * Mat4(trfs[i]);
				}

				uniSet(*uni, &mv[0], size);
			}
			break;
		case BMV_VP_MATRIX:
			uniSet(*uni, &vp, 1);
			break;
		case BMV_NORMAL_MATRIX:
			if(trfs)
			{
				Array<Mat3, maxInstances> normm;

				for(U i = 0; i < size; i++)
				{
					Mat4 mv = v * Mat4(trfs[i]);
					normm[i] = mv.getRotationPart();
				}

				uniSet(*uni, &normm[0], size);
			}
			else
			{
				ANKI_ASSERT(uniArrSize == 1
					&& "Having that instanced doesn't make sense");

				Mat3 norm = v.getRotationPart();

				uniSet(*uni, &norm, 1);
			}
			break;
		case BMV_BILLBOARD_MVP_MATRIX:
			{
				// Calc the billboard rotation matrix
				Mat3 rot =
					fr->getViewMatrix().getRotationPart().getTransposed();

				Array<Mat4, maxInstances> bmvp;

				for(U i = 0; i < size; i++)
				{
					Transform trf = trfs[i];
					trf.setRotation(rot);
					bmvp[i] = vp * Mat4(trf);
				}

				uniSet(*uni, &bmvp[0], size);
			}
			break;
		case BMV_BLURRING:
			{
				F32 blurring = 0.0;
				uniSet(*uni, &blurring, 1);
			}
			break;
		default:
			ANKI_ASSERT(0);
			break;
		}
	}
};

/// Specialize the material accepted types. The un-specialized will be used for
/// all Property types like strings, we don't need strings in our case
#define TEMPLATE_SPECIALIZATION(type) \
	template<> \
	void SetupRenderableVariableVisitor::uniSet<type>( \
		const ShaderProgramUniformVariable& uni, const type* values, \
		U32 size) \
	{ \
		if(uni.getUniformBlock()) \
		{ \
			uni.setClientMemory(&clientBlock[0], \
				RenderableDrawer::UNIFORM_BLOCK_MAX_SIZE, \
				values, size); \
		} \
		else \
		{ \
			uni.set(values, size); \
		} \
	}

TEMPLATE_SPECIALIZATION(F32)
TEMPLATE_SPECIALIZATION(Vec2)
TEMPLATE_SPECIALIZATION(Vec3)
TEMPLATE_SPECIALIZATION(Vec4)
TEMPLATE_SPECIALIZATION(Mat3)
TEMPLATE_SPECIALIZATION(Mat4)

// Texture specialization
template<>
void SetupRenderableVariableVisitor::uniSet<TextureResourcePointer>(
	const ShaderProgramUniformVariable& uni, 
	const TextureResourcePointer* values, U32 size)
{
	ANKI_ASSERT(size == 1);
	const Texture* tex = values->get();
	uni.set(*tex);
}

//==============================================================================
void RenderableDrawer::setupShaderProg(const PassLevelKey& key_,
	const Frustumable& fr, const ShaderProgram &prog,
	Renderable& renderable)
{
	prog.bind();
	
	SetupRenderableVariableVisitor vis;

	vis.fr = &fr;
	vis.renderable = &renderable;
	vis.r = r;

	PassLevelKey key(key_.pass,
		std::min(key_.level,
		U8(renderable.getRenderableMaterial().getLevelsOfDetail() - 1)));

	// Set the uniforms
	for(auto it = renderable.getVariablesBegin();
		it != renderable.getVariablesEnd(); ++it)
	{
		RenderableVariable* rvar = *it;

		const ShaderProgramUniformVariable* uni =
			rvar->tryFindShaderProgramUniformVariable(key);

		if(uni)
		{
			vis.rvar = rvar;
			vis.uni = uni;
			rvar->acceptVisitor(vis);
		}
	}

	// Write the block
	const ShaderProgramUniformBlock* block =
		renderable.getRenderableMaterial().getCommonUniformBlock();
	if(block)
	{
		ANKI_ASSERT(block->getSize() <= UNIFORM_BLOCK_MAX_SIZE);
		ANKI_ASSERT(block->getBinding() == 0);
		renderable.getUbo().write(&vis.clientBlock[0]);
		renderable.getUbo().setBinding(0);
	}
}

//==============================================================================
void RenderableDrawer::render(SceneNode& frsn, RenderingStage stage,
	U32 pass, SceneNode& rsn)
{
	ANKI_ASSERT(frsn.getFrustumable());
	Frustumable& fr = *frsn.getFrustumable();
	Renderable* renderable = rsn.getRenderable();
	ANKI_ASSERT(renderable);

	/* Instancing */
	U32 instancesCount = renderable->getRenderableInstancesCount();

	if(ANKI_UNLIKELY(instancesCount < 1))
	{
		return;
	}

	/* Blending */
	const Material& mtl = renderable->getRenderableMaterial();

	Bool blending = mtl.isBlendingEnabled();

	if(!blending)
	{
		if(stage == RS_BLEND)
		{
			return;
		}
	}
	else
	{
		if(stage != RS_BLEND)
		{
			return;
		}

		GlStateSingleton::get().setBlendFunctions(
			mtl.getBlendingSfactor(), mtl.getBlendingDfactor());
	}

	GlStateSingleton::get().enable(GL_BLEND, blending);

	// Calculate the LOD
	Vec3 camPos = fr.getFrustumableOrigin();

	F32 dist = (rsn.getSpatial()->getSpatialOrigin() - camPos).getLength();
	U8 lod = r->calculateLod(dist);

	PassLevelKey key(pass, lod);

	// Get rendering useful stuff
	const ShaderProgram* prog;
	const Vao* vao;
	U32 indicesCountArray[64];
	void* indicesOffsetArray[64] = {nullptr};
#if ANKI_DEBUG
	memset(indicesCountArray, 0, sizeof(indicesCountArray));
	memset(indicesOffsetArray, 0, sizeof(indicesOffsetArray));
#endif

	U32 primCount = 1;

	if(renderable->getSubMeshesCount() == 0)
	{
		renderable->getRenderableModelPatchBase().getRenderingData(
			key, vao, prog, indicesCountArray[0]);
	}
	else
	{
		U64 mask = renderable->getVisibleSubMeshesMask(frsn);

		renderable->getRenderableModelPatchBase().getRenderingDataSub(
			key, mask, vao, prog, indicesCountArray, indicesOffsetArray,
			primCount);
	}

	// Setup shader
	setupShaderProg(key, fr, *prog, *renderable);

	// Render
	ANKI_ASSERT(vao->getAttachmentsCount() > 1);
	vao->bind();

	if(instancesCount == 1)
	{
		if(primCount == 1)
		{
			glDrawElements(
				GL_TRIANGLES, 
				indicesCountArray[0], 
				GL_UNSIGNED_SHORT, 
				indicesOffsetArray[0]);
		}
		else
		{
#if ANKI_GL == ANKI_GL_DESKTOP
			glMultiDrawElements(GL_TRIANGLES, 
				(GLsizei*)indicesCountArray, 
				GL_UNSIGNED_SHORT, 
				(const void**)indicesOffsetArray, 
				primCount);
#else
			for(U i = 0; i < primCount; i++)
			{
				glDrawElements(
					GL_TRIANGLES, 
					indicesCountArray[i],
					GL_UNSIGNED_SHORT,
					indicesOffsetArray[i]);
			}
#endif
		}
	}
	else
	{
		glDrawElementsInstanced(
			GL_TRIANGLES, 
			indicesCountArray[0], 
			GL_UNSIGNED_SHORT, 
			0, 
			instancesCount);
	}
}

}  // end namespace anki
