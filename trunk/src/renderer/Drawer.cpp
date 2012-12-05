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
struct SetupMaterialVariableVisitor
{
	PassLevelKey key;
	const Frustumable* fr = nullptr;
	Renderer* r = nullptr;
	Renderable* renderable = nullptr;
	Array<U8, RenderableDrawer::UNIFORM_BLOCK_MAX_SIZE> clientBlock;
	RenderableMaterialVariable* rvar = nullptr;

	/// Set a uniform in a client block
	template<typename T>
	void uniSet(const ShaderProgramUniformVariable& uni, const T& value)
	{
		ANKI_ASSERT(0);
	}

	template<typename TProp>
	void visit(TProp& x)
	{
		const MaterialVariable& mv = rvar->getMaterialVariable();

		const ShaderProgramUniformVariable* uni =
			mv.findShaderProgramUniformVariable(key);
		if(!uni)
		{
			return;
		}

		// Set uniform
		//
		const Transform* rwtrf = renderable->getRenderableWorldTransform();

		Mat4 mMat = (rwtrf) ? Mat4(*rwtrf) : Mat4::getIdentity();
		const Mat4& vpMat = fr->getViewProjectionMatrix();

		Mat4 mvMat;
		Bool mvMatCalculated = false; // Opt

		switch(rvar->getBuildinId())
		{
		case BMV_NO_BUILDIN:
			uniSet(*uni, x);
			break;
		case BMV_MODEL_VIEW_PROJECTION_MATRIX:
			{
				Mat4 mvpMat = vpMat * mMat;
				uniSet(*uni, mvpMat);
			}
			break;
		case BMV_MODEL_VIEW_MATRIX:
			if(!mvMatCalculated)
			{
				mvMat = fr->getViewMatrix() * mMat;
				mvMatCalculated = true;
			}
			uniSet(*uni, mvMat);
			break;
		case BMV_NORMAL_MATRIX:
			if(!mvMatCalculated)
			{
				mvMat = fr->getViewMatrix() * mMat;
				mvMatCalculated = true;
			}
			uniSet(*uni, mvMat.getRotationPart());
			break;
		case BMV_INSTANCING_MODEL_VIEW_PROJECTION_MATRICES:
			{
				U32 instancesCount = renderable->getRenderableInstancesCount();

				Array<Mat4, 64> mvps;
				ANKI_ASSERT(mvps.getSize() >= instancesCount);
				const Transform* trfs =
					renderable->getRenderableInstancingWorldTransforms();
				ANKI_ASSERT(trfs != nullptr);

				for(U i = 0; i < instancesCount; i++)
				{
					mvps[i] = vpMat * Mat4(trfs[0]);
				}

				uni->set(&mvps[0], instancesCount);
			}
			break;
		case BMV_BLURRING:
			uniSet(*uni, 0.0);
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
	void SetupMaterialVariableVisitor::uniSet<type>( \
		const ShaderProgramUniformVariable& uni, const type& value) \
	{ \
		if(uni.getUniformBlock()) \
		{ \
			uni.setClientMemory(&clientBlock[0], \
				RenderableDrawer::UNIFORM_BLOCK_MAX_SIZE, \
				&value, 1); \
		} \
		else \
		{ \
			uni.set(value); \
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
void SetupMaterialVariableVisitor::uniSet<TextureResourcePointer>(
	const ShaderProgramUniformVariable& uni, 
	const TextureResourcePointer& value)
{
	const Texture* tex = value.get();
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
	
	SetupMaterialVariableVisitor vis;

	vis.fr = &fr;
	vis.key = key;
	vis.renderable = &renderable;
	vis.r = r;

	// Set the uniforms
	for(auto it = renderable.getVariablesBegin();
		it != renderable.getVariablesEnd(); ++it)
	{
		RenderableMaterialVariable* rvar = *it;
		vis.rvar = rvar;
		rvar->getMaterialVariable().acceptVisitor(vis);
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
void RenderableDrawer::render(const Frustumable& fr, U32 pass,
	Renderable& renderable)
{
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
		renderable.getRenderableModelPatchBase().getIndecesCount(0);

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
