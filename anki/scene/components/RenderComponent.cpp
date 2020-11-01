// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/RenderComponent.h>
#include <anki/scene/SceneNode.h>
#include <anki/resource/TextureResource.h>
#include <anki/resource/ResourceManager.h>
#include <anki/util/Logger.h>

namespace anki
{

void RenderComponent::allocateAndSetupUniforms(const MaterialResourcePtr& mtl, const RenderQueueDrawContext& ctx,
											   ConstWeakArray<Mat4> transforms, ConstWeakArray<Mat4> prevTransforms,
											   StagingGpuMemoryManager& alloc)
{
	ANKI_ASSERT(transforms.getSize() <= MAX_INSTANCES);
	ANKI_ASSERT(prevTransforms.getSize() == transforms.getSize());

	const MaterialVariant& variant = mtl->getOrCreateVariant(ctx.m_key);
	const U32 set = mtl->getDescriptorSetIndex();

	// Allocate uniform memory
	StagingGpuMemoryToken token;
	void* const uniformsBegin =
		alloc.allocateFrame(variant.getUniformBlockSize(), StagingGpuMemoryType::UNIFORM, token);
	const void* const uniformsEnd = static_cast<U8*>(uniformsBegin) + variant.getUniformBlockSize();
	ctx.m_commandBuffer->bindUniformBuffer(set, mtl->getUniformsBinding(), token.m_buffer, token.m_offset,
										   token.m_range);

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
			variant.writeShaderBlockMemory(mvar, &val, 1, uniformsBegin, uniformsEnd);
			break;
		}
		case ShaderVariableDataType::VEC2:
		{
			const Vec2 val = mvar.getValue<Vec2>();
			variant.writeShaderBlockMemory(mvar, &val, 1, uniformsBegin, uniformsEnd);
			break;
		}
		case ShaderVariableDataType::VEC3:
		{
			switch(mvar.getBuiltin())
			{
			case BuiltinMaterialVariableId::NONE:
			{
				const Vec3 val = mvar.getValue<Vec3>();
				variant.writeShaderBlockMemory(mvar, &val, 1, uniformsBegin, uniformsEnd);
				break;
			}
			case BuiltinMaterialVariableId::CAMERA_POSITION:
			{
				const Vec3 val = ctx.m_cameraTransform.getTranslationPart().xyz();
				variant.writeShaderBlockMemory(mvar, &val, 1, uniformsBegin, uniformsEnd);
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
			variant.writeShaderBlockMemory(mvar, &val, 1, uniformsBegin, uniformsEnd);
			break;
		}
		case ShaderVariableDataType::MAT3:
		{
			switch(mvar.getBuiltin())
			{
			case BuiltinMaterialVariableId::NONE:
			{
				const Mat3 val = mvar.getValue<Mat3>();
				variant.writeShaderBlockMemory(mvar, &val, 1, uniformsBegin, uniformsEnd);
				break;
			}
			case BuiltinMaterialVariableId::NORMAL_MATRIX:
			{
				ANKI_ASSERT(transforms.getSize() > 0);

				Array<Mat3, MAX_INSTANCES> normMats;
				for(U32 i = 0; i < transforms.getSize(); i++)
				{
					const Mat4 mv = ctx.m_viewMatrix * transforms[i];
					normMats[i] = mv.getRotationPart();
					normMats[i].reorthogonalize();
				}

				variant.writeShaderBlockMemory(mvar, &normMats[0], transforms.getSize(), uniformsBegin, uniformsEnd);
				break;
			}
			case BuiltinMaterialVariableId::ROTATION_MATRIX:
			{
				ANKI_ASSERT(transforms.getSize() > 0);

				Array<Mat3, MAX_INSTANCES> rots;
				for(U32 i = 0; i < transforms.getSize(); i++)
				{
					rots[i] = transforms[i].getRotationPart();
				}

				variant.writeShaderBlockMemory(mvar, &rots[0], transforms.getSize(), uniformsBegin, uniformsEnd);
				break;
			}
			case BuiltinMaterialVariableId::CAMERA_ROTATION_MATRIX:
			{
				const Mat3 rot = ctx.m_cameraTransform.getRotationPart();
				variant.writeShaderBlockMemory(mvar, &rot, 1, uniformsBegin, uniformsEnd);
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
				variant.writeShaderBlockMemory(mvar, &val, 1, uniformsBegin, uniformsEnd);
				break;
			}
			case BuiltinMaterialVariableId::MODEL_VIEW_PROJECTION_MATRIX:
			{
				ANKI_ASSERT(transforms.getSize() > 0);

				Array<Mat4, MAX_INSTANCES> mvp;
				for(U32 i = 0; i < transforms.getSize(); i++)
				{
					mvp[i] = ctx.m_viewProjectionMatrix * transforms[i];
				}

				variant.writeShaderBlockMemory(mvar, &mvp[0], transforms.getSize(), uniformsBegin, uniformsEnd);
				break;
			}
			case BuiltinMaterialVariableId::PREVIOUS_MODEL_VIEW_PROJECTION_MATRIX:
			{
				ANKI_ASSERT(prevTransforms.getSize() > 0);

				Array<Mat4, MAX_INSTANCES> mvp;
				for(U32 i = 0; i < prevTransforms.getSize(); i++)
				{
					mvp[i] = ctx.m_previousViewProjectionMatrix * prevTransforms[i];
				}

				variant.writeShaderBlockMemory(mvar, &mvp[0], prevTransforms.getSize(), uniformsBegin, uniformsEnd);
				break;
			}
			case BuiltinMaterialVariableId::MODEL_VIEW_MATRIX:
			{
				ANKI_ASSERT(transforms.getSize() > 0);

				Array<Mat4, MAX_INSTANCES> mv;
				for(U32 i = 0; i < transforms.getSize(); i++)
				{
					mv[i] = ctx.m_viewMatrix * transforms[i];
				}

				variant.writeShaderBlockMemory(mvar, &mv[0], transforms.getSize(), uniformsBegin, uniformsEnd);
				break;
			}
			case BuiltinMaterialVariableId::MODEL_MATRIX:
			{
				ANKI_ASSERT(transforms.getSize() > 0);

				variant.writeShaderBlockMemory(mvar, &transforms[0], transforms.getSize(), uniformsBegin, uniformsEnd);
				break;
			}
			case BuiltinMaterialVariableId::VIEW_PROJECTION_MATRIX:
			{
				ANKI_ASSERT(transforms.getSize() == 0 && "Cannot have transform");
				variant.writeShaderBlockMemory(mvar, &ctx.m_viewProjectionMatrix, 1, uniformsBegin, uniformsEnd);
				break;
			}
			case BuiltinMaterialVariableId::VIEW_MATRIX:
			{
				variant.writeShaderBlockMemory(mvar, &ctx.m_viewMatrix, 1, uniformsBegin, uniformsEnd);
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
			ctx.m_commandBuffer->bindTexture(set, variant.getOpaqueBinding(mvar),
											 mvar.getValue<TextureResourcePtr>()->getGrTextureView(),
											 TextureUsageBit::SAMPLED_FRAGMENT);
			break;
		}
		case ShaderVariableDataType::SAMPLER:
		{
			switch(mvar.getBuiltin())
			{
			case BuiltinMaterialVariableId::GLOBAL_SAMPLER:
				ctx.m_commandBuffer->bindSampler(set, variant.getOpaqueBinding(mvar), ctx.m_sampler);
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
