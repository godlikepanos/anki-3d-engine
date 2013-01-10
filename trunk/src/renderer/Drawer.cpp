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
	PassLevelKey key;
	const Frustumable* fr = nullptr;
	Renderer* r = nullptr;
	Renderable* renderable = nullptr;
	Array<U8, RenderableDrawer::UNIFORM_BLOCK_MAX_SIZE> clientBlock;
	RenderableVariable* rvar = nullptr;

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
		const ShaderProgramUniformVariable* uni =
			x.tryFindShaderProgramUniformVariable(key);
		if(!uni)
		{
			return;
		}

		const U32 instancesCount = renderable->getRenderableInstancesCount();
		const U32 uniArrSize = uni->getSize();
		const U32 size = std::min(instancesCount, uniArrSize);

		// Set uniform
		//
		const Transform* trfs = renderable->getRenderableWorldTransforms();
		const Mat4& vp = fr->getViewProjectionMatrix();
		const Mat4& v = fr->getViewMatrix();

		const U maxInstances = 32; // XXX Use a proper vector with allocator

		switch(x.getBuildinId())
		{
		case BMV_NO_BUILDIN:
			uniSet<typename TRenderableVariableTemplate::Type>(
				*uni, x.get(), x.getArraySize());
			break;
		case BMV_MODEL_VIEW_PROJECTION_MATRIX:
			{
				ANKI_ASSERT(trfs != nullptr);
				Array<Mat4, maxInstances> mvp;

				for(U i = 0; i < size; i++)
				{
					mvp[i] = vp * Mat4(trfs[i]);
				}

				uniSet(*uni, &mvp[0], size);
			}
			break;
		case BMV_BILLBOARD_MVP_MATRIX:
			{
				// Calc the billboard rotation matrix
				Mat3 rot =
					fr->getViewMatrix().getRotationPart().getTransposed();

				/*Vec3 up(0.0, 1.0, 0.0);
				Vec3 front = rot.getColumn(2);
				Vec3 right = up.cross(front);
				up = front.cross(right);
				rot.setColumns(right, up, front);*/

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
		case BMV_MODEL_VIEW_MATRIX:
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
		case BMV_NORMAL_MATRIX:
			{
				Array<Mat3, maxInstances> normm;

				for(U i = 0; i < size; i++)
				{
					Mat4 mv = v * Mat4(trfs[i]);
					normm[i] = mv.getRotationPart();
				}

				uniSet(*uni, &normm[0], size);
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
void RenderableDrawer::setupShaderProg(
	const PassLevelKey& key,
	const Frustumable& fr,
	Renderable& renderable)
{
	const Material& mtl = renderable.getRenderableMaterial();
	const ShaderProgram& sprog = mtl.findShaderProgram(key);

	sprog.bind();
	
	SetupRenderableVariableVisitor vis;

	vis.fr = &fr;
	vis.key = key;
	vis.renderable = &renderable;
	vis.r = r;

	// Set the uniforms
	for(auto it = renderable.getVariablesBegin();
		it != renderable.getVariablesEnd(); ++it)
	{
		RenderableVariable* rvar = *it;
		vis.rvar = rvar;
		rvar->acceptVisitor(vis);
	}

	// Write the block
	const ShaderProgramUniformBlock* block = mtl.getCommonUniformBlock();
	if(block)
	{
		ANKI_ASSERT(block->getSize() <= UNIFORM_BLOCK_MAX_SIZE);
		ANKI_ASSERT(block->getBinding() == 0);
		renderable.getUbo().write(&vis.clientBlock[0]);
		renderable.getUbo().setBinding(0);
	}
}

//==============================================================================
void RenderableDrawer::render(const Frustumable& fr, RenderingStage stage,
	U32 pass, Renderable& renderable)
{
	const Material& mtl = renderable.getRenderableMaterial();

	Bool blending = mtl.isBlendingEnabled();

	if(blending)
	{
		if(stage != RS_BLEND)
		{
			return;
		}

		GlStateSingleton::get().setBlendFunctions(
			mtl.getBlendingSfactor(), mtl.getBlendingDfactor());
	}
	else
	{
		if(stage == RS_BLEND)
		{
			return;
		}
	}

	GlStateSingleton::get().enable(GL_BLEND, blending);

	/*float dist = (node.getWorldTransform().getOrigin() -
		cam.getWorldTransform().getOrigin()).getLength();
	uint lod = std::min(r.calculateLod(dist), mtl.getLevelsOfDetail() - 1);*/

	U32 instancesCount = renderable.getRenderableInstancesCount();

	if(instancesCount < 1)
	{
		return;
	}

	PassLevelKey key(pass, 0);

	// Setup shader
	setupShaderProg(key, fr, renderable);

	// Render
	U32 indicesCount = 
		renderable.getRenderableModelPatchBase().getIndecesCount();

	const Vao& vao = renderable.getRenderableModelPatchBase().getVao(key);
	ANKI_ASSERT(vao.getAttachmentsCount() > 1);
	vao.bind();

	if(instancesCount == 1)
	{
		glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_SHORT, 0);
	}
	else
	{
		glDrawElementsInstanced(
			GL_TRIANGLES, indicesCount, GL_UNSIGNED_SHORT, 0, instancesCount);
	}
}

}  // end namespace anki
