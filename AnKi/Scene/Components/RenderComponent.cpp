// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/RenderComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Shaders/Include/GpuSceneTypes.h>

namespace anki {

ANKI_SCENE_COMPONENT_STATICS(RenderComponent)

void RenderComponent::allocateAndSetupUniforms(const MaterialResourcePtr& mtl, const RenderQueueDrawContext& ctx,
											   ConstWeakArray<Mat3x4> transforms, ConstWeakArray<Mat3x4> prevTransforms,
											   StagingGpuMemoryPool& alloc)
{
	ANKI_ASSERT(transforms.getSize() <= MAX_INSTANCE_COUNT);
	ANKI_ASSERT(prevTransforms.getSize() == transforms.getSize());

	CommandBufferPtr cmdb = ctx.m_commandBuffer;

	const U32 set = MATERIAL_SET_EXTERNAL;

	cmdb->bindSampler(set, MATERIAL_BINDING_GLOBAL_SAMPLER, ctx.m_sampler);

	cmdb->bindUniformBuffer(set, MATERIAL_BINDING_GLOBAL_UNIFORMS, ctx.m_globalUniforms.m_buffer,
							ctx.m_globalUniforms.m_offset, ctx.m_globalUniforms.m_range);

	// Fill the RenderableGpuView
	const U32 renderableGpuViewsUboSize = sizeof(RenderableGpuView) * transforms.getSize();
	if(renderableGpuViewsUboSize)
	{
		StagingGpuMemoryToken token;
		RenderableGpuView* renderableGpuViews = static_cast<RenderableGpuView*>(
			alloc.allocateFrame(renderableGpuViewsUboSize, StagingGpuMemoryType::UNIFORM, token));
		ANKI_ASSERT(isAligned(alignof(RenderableGpuView), renderableGpuViews));

		cmdb->bindUniformBuffer(set, MATERIAL_BINDING_RENDERABLE_GPU_VIEW, token.m_buffer, token.m_offset,
								token.m_range);

		for(U32 i = 0; i < transforms.getSize(); ++i)
		{
			memcpy(&renderableGpuViews->m_worldTransform, &transforms[i], sizeof(renderableGpuViews->m_worldTransform));
			memcpy(&renderableGpuViews->m_previousWorldTransform, &prevTransforms[i],
				   sizeof(renderableGpuViews->m_previousWorldTransform));
			renderableGpuViews->m_worldRotation = transforms[i].getRotationPart();

			++renderableGpuViews;
		}
	}

	// Local uniforms
	const U32 localUniformsUboSize = U32(mtl->getPrefilledLocalUniforms().getSizeInBytes());

	StagingGpuMemoryToken token;
	U8* const localUniformsBegin =
		(localUniformsUboSize != 0)
			? static_cast<U8*>(alloc.allocateFrame(localUniformsUboSize, StagingGpuMemoryType::UNIFORM, token))
			: nullptr;

	if(localUniformsUboSize)
	{
		cmdb->bindUniformBuffer(set, MATERIAL_BINDING_LOCAL_UNIFORMS, token.m_buffer, token.m_offset, token.m_range);
	}

	// Iterate variables
	for(const MaterialVariable& mvar : mtl->getVariables())
	{
		switch(mvar.getDataType())
		{
		case ShaderVariableDataType::F32:
		{
			const F32 val = mvar.getValue<F32>();
			memcpy(localUniformsBegin + mvar.getOffsetInLocalUniforms(), &val, sizeof(val));
			break;
		}
		case ShaderVariableDataType::VEC2:
		{
			const Vec2 val = mvar.getValue<Vec2>();
			memcpy(localUniformsBegin + mvar.getOffsetInLocalUniforms(), &val, sizeof(val));
			break;
		}
		case ShaderVariableDataType::VEC3:
		{
			const Vec3 val = mvar.getValue<Vec3>();
			memcpy(localUniformsBegin + mvar.getOffsetInLocalUniforms(), &val, sizeof(val));
			break;
		}
		case ShaderVariableDataType::VEC4:
		{
			const Vec4 val = mvar.getValue<Vec4>();
			memcpy(localUniformsBegin + mvar.getOffsetInLocalUniforms(), &val, sizeof(val));
			break;
		}
		case ShaderVariableDataType::TEXTURE_2D:
		case ShaderVariableDataType::TEXTURE_2D_ARRAY:
		case ShaderVariableDataType::TEXTURE_3D:
		case ShaderVariableDataType::TEXTURE_CUBE:
		{
			cmdb->bindTexture(set, mvar.getTextureBinding(), mvar.getValue<ImageResourcePtr>()->getTextureView());
			break;
		}
		default:
			ANKI_ASSERT(0);
		} // end switch
	}
}

} // end namespace anki
