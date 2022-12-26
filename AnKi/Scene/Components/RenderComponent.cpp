// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/RenderComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Util/Logger.h>

namespace anki {

ANKI_SCENE_COMPONENT_STATICS(RenderComponent)

RenderComponent::RenderComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
{
	getExternalSubsystems(*node).m_gpuSceneMemoryPool->allocate(sizeof(GpuSceneRenderable), sizeof(U32),
																m_gpuSceneRenderable);
}

void RenderComponent::allocateAndSetupUniforms(const MaterialResourcePtr& mtl, const RenderQueueDrawContext& ctx,
											   ConstWeakArray<Mat3x4> transforms, ConstWeakArray<Mat3x4> prevTransforms,
											   RebarStagingGpuMemoryPool& alloc,
											   const Vec4& positionScaleAndTranslation)
{
	ANKI_ASSERT(transforms.getSize() <= kMaxInstanceCount);
	ANKI_ASSERT(prevTransforms.getSize() == transforms.getSize());

	CommandBufferPtr cmdb = ctx.m_commandBuffer;

	const U32 set = kMaterialSetLocal;

	// Fill the RenderableGpuView
	const U32 renderableGpuViewsUboSize = sizeof(RenderableGpuView) * transforms.getSize();
	if(renderableGpuViewsUboSize)
	{
		RebarGpuMemoryToken token;
		RenderableGpuView* renderableGpuViews =
			static_cast<RenderableGpuView*>(alloc.allocateFrame(renderableGpuViewsUboSize, token));
		ANKI_ASSERT(isAligned(alignof(RenderableGpuView), renderableGpuViews));

		cmdb->bindStorageBuffer(set, kMaterialBindingRenderableGpuView, alloc.getBuffer(), token.m_offset,
								token.m_range);

		for(U32 i = 0; i < transforms.getSize(); ++i)
		{
			renderableGpuViews->m_worldTransform = transforms[i];
			renderableGpuViews->m_previousWorldTransform = prevTransforms[i];
			renderableGpuViews->m_positionScaleF32AndTranslationVec3 = positionScaleAndTranslation;

			++renderableGpuViews;
		}
	}

	// Local uniforms
	const U32 localUniformsUboSize = U32(mtl->getPrefilledLocalUniforms().getSizeInBytes());

	RebarGpuMemoryToken token;
	U8* const localUniformsBegin =
		(localUniformsUboSize != 0) ? static_cast<U8*>(alloc.allocateFrame(localUniformsUboSize, token)) : nullptr;

	if(localUniformsUboSize)
	{
		cmdb->bindStorageBuffer(set, kMaterialBindingLocalUniforms, alloc.getBuffer(), token.m_offset, token.m_range);
	}

	// Iterate variables
	for(const MaterialVariable& mvar : mtl->getVariables())
	{
		switch(mvar.getDataType())
		{
		case ShaderVariableDataType::kU32:
		{
			const U32 val = mvar.getValue<U32>();
			memcpy(localUniformsBegin + mvar.getOffsetInLocalUniforms(), &val, sizeof(val));
			break;
		}
		case ShaderVariableDataType::kF32:
		{
			const F32 val = mvar.getValue<F32>();
			memcpy(localUniformsBegin + mvar.getOffsetInLocalUniforms(), &val, sizeof(val));
			break;
		}
		case ShaderVariableDataType::kVec2:
		{
			const Vec2 val = mvar.getValue<Vec2>();
			memcpy(localUniformsBegin + mvar.getOffsetInLocalUniforms(), &val, sizeof(val));
			break;
		}
		case ShaderVariableDataType::kVec3:
		{
			const Vec3 val = mvar.getValue<Vec3>();
			memcpy(localUniformsBegin + mvar.getOffsetInLocalUniforms(), &val, sizeof(val));
			break;
		}
		case ShaderVariableDataType::kVec4:
		{
			const Vec4 val = mvar.getValue<Vec4>();
			memcpy(localUniformsBegin + mvar.getOffsetInLocalUniforms(), &val, sizeof(val));
			break;
		}
		case ShaderVariableDataType::kTexture2D:
		case ShaderVariableDataType::kTexture2DArray:
		case ShaderVariableDataType::kTexture3D:
		case ShaderVariableDataType::kTextureCube:
		{
			cmdb->bindTexture(set, mvar.getTextureBinding(), mvar.getValue<ImageResourcePtr>()->getTextureView());
			break;
		}
		default:
			ANKI_ASSERT(0);
		} // end switch
	}
}

void RenderComponent::onDestroy(SceneNode& node)
{
	getExternalSubsystems(node).m_gpuSceneMemoryPool->free(m_gpuSceneRenderable);
}

} // end namespace anki
