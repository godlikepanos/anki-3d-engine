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

void RenderComponent::allocateAndSetupUniforms(const MaterialResourcePtr& mtl, const RenderQueueDrawContext& ctx,
											   ConstWeakArray<Mat3x4> transforms, ConstWeakArray<Mat3x4> prevTransforms,
											   StagingGpuMemoryPool& alloc)
{
	ANKI_ASSERT(transforms.getSize() <= MAX_INSTANCE_COUNT);
	ANKI_ASSERT(prevTransforms.getSize() == transforms.getSize());

	const MaterialVariant& variant = mtl->getOrCreateVariant(ctx.m_key);
	const U32 set = mtl->getDescriptorSetIndex();

	// Allocate and bind uniform memory
	const U32 perDrawUboSize = variant.getPerDrawUniformBlockSize();
	const U32 perInstanceUboSize = variant.getPerInstanceUniformBlockSize(transforms.getSize());

	StagingGpuMemoryToken token;
	void* const perDrawUniformsBegin =
		(perDrawUboSize != 0) ? alloc.allocateFrame(perDrawUboSize, StagingGpuMemoryType::UNIFORM, token) : nullptr;
	const void* const perDrawUniformsEnd = static_cast<U8*>(perDrawUniformsBegin) + perDrawUboSize;

	StagingGpuMemoryToken token1;
	void* const perInstanceUniformsBegin =
		(perInstanceUboSize != 0) ? alloc.allocateFrame(perInstanceUboSize, StagingGpuMemoryType::UNIFORM, token1)
								  : nullptr;
	const void* const perInstanceUniformsEnd = static_cast<U8*>(perInstanceUniformsBegin) + perInstanceUboSize;

	if(perDrawUboSize)
	{
		ctx.m_commandBuffer->bindUniformBuffer(set, mtl->getPerDrawUniformBlockBinding(), token.m_buffer,
											   token.m_offset, token.m_range);
	}

	if(perInstanceUboSize)
	{
		ctx.m_commandBuffer->bindUniformBuffer(set, mtl->getPerInstanceUniformBlockBinding(), token1.m_buffer,
											   token1.m_offset, token1.m_range);
	}

	ctx.m_commandBuffer->bindUniformBuffer(set, mtl->getGlobalUniformsUniformBlockBinding(),
										   ctx.m_globalUniforms.m_buffer, ctx.m_globalUniforms.m_offset,
										   ctx.m_globalUniforms.m_range);

	// Iterate variables
	for(const MaterialVariable& mvar : mtl->getVariables())
	{
		if(!variant.isVariableActive(mvar))
		{
			continue;
		}

		switch(mvar.getDataType())
		{
		case ShaderVariableDataType::F32:
		{
			const F32 val = mvar.getValue<F32>();
			variant.writeShaderBlockMemory(mvar, &val, 1, perDrawUniformsBegin, perDrawUniformsEnd);
			break;
		}
		case ShaderVariableDataType::VEC2:
		{
			const Vec2 val = mvar.getValue<Vec2>();
			variant.writeShaderBlockMemory(mvar, &val, 1, perDrawUniformsBegin, perDrawUniformsEnd);
			break;
		}
		case ShaderVariableDataType::VEC3:
		{
			switch(mvar.getBuiltin())
			{
			case BuiltinMaterialVariableId::NONE:
			{
				const Vec3 val = mvar.getValue<Vec3>();
				variant.writeShaderBlockMemory(mvar, &val, 1, perDrawUniformsBegin, perDrawUniformsEnd);
				break;
			}
			default:
				ANKI_ASSERT(0);
			}

			break;
		}
		case ShaderVariableDataType::VEC4:
		{
			const Vec4 val = mvar.getValue<Vec4>();
			variant.writeShaderBlockMemory(mvar, &val, 1, perDrawUniformsBegin, perDrawUniformsEnd);
			break;
		}
		case ShaderVariableDataType::MAT3:
		{
			switch(mvar.getBuiltin())
			{
			case BuiltinMaterialVariableId::NONE:
			{
				const Mat3 val = mvar.getValue<Mat3>();
				variant.writeShaderBlockMemory(mvar, &val, 1, perDrawUniformsBegin, perDrawUniformsEnd);
				break;
			}
			case BuiltinMaterialVariableId::ROTATION:
			{
				ANKI_ASSERT(transforms.getSize() > 0);

				Array<Mat3, MAX_INSTANCE_COUNT> rots;
				for(U32 i = 0; i < transforms.getSize(); i++)
				{
					rots[i] = transforms[i].getRotationPart();
				}

				variant.writeShaderBlockMemory(mvar, &rots[0], transforms.getSize(),
											   (mvar.isInstanced()) ? perInstanceUniformsBegin : perDrawUniformsBegin,
											   (mvar.isInstanced()) ? perInstanceUniformsEnd : perDrawUniformsEnd);
				break;
			}
			default:
				ANKI_ASSERT(0);
			}

			break;
		}
		case ShaderVariableDataType::MAT4:
		{
			switch(mvar.getBuiltin())
			{
			case BuiltinMaterialVariableId::NONE:
			{
				const Mat4 val = mvar.getValue<Mat4>();
				variant.writeShaderBlockMemory(mvar, &val, 1, perDrawUniformsBegin, perDrawUniformsEnd);
				break;
			}
			default:
				ANKI_ASSERT(0);
			}

			break;
		}
		case ShaderVariableDataType::MAT3X4:
		{
			switch(mvar.getBuiltin())
			{
			case BuiltinMaterialVariableId::NONE:
			{
				const Mat3x4 val = mvar.getValue<Mat3x4>();
				variant.writeShaderBlockMemory(mvar, &val, 1, perDrawUniformsBegin, perDrawUniformsEnd);
				break;
			}
			case BuiltinMaterialVariableId::TRANSFORM:
			{
				ANKI_ASSERT(transforms.getSize() > 0);
				variant.writeShaderBlockMemory(mvar, &transforms[0], transforms.getSize(),
											   (mvar.isInstanced()) ? perInstanceUniformsBegin : perDrawUniformsBegin,
											   (mvar.isInstanced()) ? perInstanceUniformsEnd : perDrawUniformsEnd);
				break;
			}
			case BuiltinMaterialVariableId::PREVIOUS_TRANSFORM:
			{
				ANKI_ASSERT(prevTransforms.getSize() > 0);
				variant.writeShaderBlockMemory(mvar, &prevTransforms[0], prevTransforms.getSize(),
											   (mvar.isInstanced()) ? perInstanceUniformsBegin : perDrawUniformsBegin,
											   (mvar.isInstanced()) ? perInstanceUniformsEnd : perDrawUniformsEnd);
				break;
			}
			default:
				ANKI_ASSERT(0);
			}

			break;
		}
		case ShaderVariableDataType::TEXTURE_2D:
		case ShaderVariableDataType::TEXTURE_2D_ARRAY:
		case ShaderVariableDataType::TEXTURE_3D:
		case ShaderVariableDataType::TEXTURE_CUBE:
		{
			ctx.m_commandBuffer->bindTexture(set, mvar.getOpaqueBinding(),
											 mvar.getValue<ImageResourcePtr>()->getTextureView());
			break;
		}
		case ShaderVariableDataType::SAMPLER:
		{
			switch(mvar.getBuiltin())
			{
			case BuiltinMaterialVariableId::GLOBAL_SAMPLER:
				ctx.m_commandBuffer->bindSampler(set, mvar.getOpaqueBinding(), ctx.m_sampler);
				break;
			default:
				ANKI_ASSERT(0);
			}

			break;
		}
		default:
			ANKI_ASSERT(0);
		} // end switch
	}
}

} // end namespace anki
