// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/RenderComponent.h>
#include <anki/scene/SceneNode.h>
#include <anki/resource/TextureResource.h>
#include <anki/resource/ResourceManager.h>
#include <anki/util/Logger.h>

namespace anki
{

RenderComponent::RenderComponent(SceneNode* node, MaterialResourcePtr mtl)
	: SceneComponent(SceneComponentType::RENDER, node)
	, m_mtl(mtl)
{
}

RenderComponent::~RenderComponent()
{
	m_vars.destroy(getAllocator());
}

Error RenderComponent::init()
{
	const Material& mtl = getMaterial();

	// Create the material variables
	m_vars.create(getAllocator(), mtl.getVariables().getSize());
	U count = 0;
	for(const MaterialVariable& mv : mtl.getVariables())
	{
		m_vars[count++].m_mvar = &mv;
	}

	return ErrorCode::NONE;
}

void RenderComponent::allocateAndSetupUniforms(const RenderQueueDrawContext& ctx,
	WeakArray<const Mat4> transforms,
	StagingGpuMemoryManager& alloc,
	StagingGpuMemoryToken& token) const
{
	ANKI_ASSERT(transforms.getSize() <= MAX_INSTANCES);

	const MaterialVariant& variant = m_mtl->getOrCreateVariant(ctx.m_key);
	const ShaderProgramResourceVariant& progVariant = variant.getShaderProgramResourceVariant();

	// Allocate uniform memory
	U8* uniforms =
		static_cast<U8*>(alloc.allocateFrame(variant.getUniformBlockSize(), StagingGpuMemoryType::UNIFORM, token));
	void* const uniformsBegin = uniforms;
	const void* const uniformsEnd = uniforms + variant.getUniformBlockSize();

	// Iterate variables
	for(auto it = m_vars.getBegin(); it != m_vars.getEnd(); ++it)
	{
		const RenderComponentVariable& var = *it;
		const MaterialVariable& mvar = var.getMaterialVariable();
		const ShaderProgramResourceInputVariable& progvar = mvar.getShaderProgramResourceInputVariable();

		if(!variant.variableActive(mvar))
		{
			continue;
		}

		switch(progvar.getShaderVariableDataType())
		{
		case ShaderVariableDataType::FLOAT:
		{
			F32 val = mvar.getValue<F32>();
			progVariant.writeShaderBlockMemory(progvar, &val, 1, uniformsBegin, uniformsEnd);
			break;
		}
		case ShaderVariableDataType::VEC2:
		{
			Vec2 val = mvar.getValue<Vec2>();
			progVariant.writeShaderBlockMemory(progvar, &val, 1, uniformsBegin, uniformsEnd);
			break;
		}
		case ShaderVariableDataType::VEC3:
		{
			Vec3 val = mvar.getValue<Vec3>();
			progVariant.writeShaderBlockMemory(progvar, &val, 1, uniformsBegin, uniformsEnd);
			break;
		}
		case ShaderVariableDataType::VEC4:
		{
			Vec4 val = mvar.getValue<Vec4>();
			progVariant.writeShaderBlockMemory(progvar, &val, 1, uniformsBegin, uniformsEnd);
			break;
		}
		case ShaderVariableDataType::MAT3:
		{
			switch(mvar.getBuiltin())
			{
			case BuiltinMaterialVariableId::NONE:
			{
				Mat3 val = mvar.getValue<Mat3>();
				progVariant.writeShaderBlockMemory(progvar, &val, 1, uniformsBegin, uniformsEnd);
				break;
			}
			case BuiltinMaterialVariableId::NORMAL_MATRIX:
			{
				ANKI_ASSERT(transforms.getSize() > 0);

				DynamicArrayAuto<Mat3> normMats(getFrameAllocator());
				normMats.create(transforms.getSize());

				for(U i = 0; i < transforms.getSize(); i++)
				{
					Mat4 mv = ctx.m_viewMatrix * transforms[i];
					normMats[i] = mv.getRotationPart();
					normMats[i].reorthogonalize();
				}

				progVariant.writeShaderBlockMemory(
					progvar, &normMats[0], transforms.getSize(), uniformsBegin, uniformsEnd);
				break;
			}
			case BuiltinMaterialVariableId::CAMERA_ROTATION_MATRIX:
			{
				Mat3 rot = ctx.m_cameraTransform.getRotationPart();
				progVariant.writeShaderBlockMemory(progvar, &rot, 1, uniformsBegin, uniformsEnd);
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
				Mat4 val = mvar.getValue<Mat4>();
				progVariant.writeShaderBlockMemory(progvar, &val, 1, uniformsBegin, uniformsEnd);
				break;
			}
			case BuiltinMaterialVariableId::MODEL_VIEW_PROJECTION_MATRIX:
			{
				ANKI_ASSERT(transforms.getSize() > 0);

				DynamicArrayAuto<Mat4> mvp(getFrameAllocator());
				mvp.create(transforms.getSize());

				for(U i = 0; i < transforms.getSize(); i++)
				{
					mvp[i] = ctx.m_viewProjectionMatrix * transforms[i];
				}

				progVariant.writeShaderBlockMemory(progvar, &mvp[0], transforms.getSize(), uniformsBegin, uniformsEnd);
				break;
			}
			case BuiltinMaterialVariableId::MODEL_VIEW_MATRIX:
			{
				ANKI_ASSERT(transforms.getSize() > 0);

				DynamicArrayAuto<Mat4> mv(getFrameAllocator());
				mv.create(transforms.getSize());

				for(U i = 0; i < transforms.getSize(); i++)
				{
					mv[i] = ctx.m_viewMatrix * transforms[i];
				}

				progVariant.writeShaderBlockMemory(progvar, &mv[0], transforms.getSize(), uniformsBegin, uniformsEnd);
				break;
			}
			case BuiltinMaterialVariableId::VIEW_PROJECTION_MATRIX:
			{
				ANKI_ASSERT(transforms.getSize() == 0 && "Cannot have transform");
				progVariant.writeShaderBlockMemory(progvar, &ctx.m_viewProjectionMatrix, 1, uniformsBegin, uniformsEnd);
				break;
			}
			case BuiltinMaterialVariableId::VIEW_MATRIX:
			{
				progVariant.writeShaderBlockMemory(progvar, &ctx.m_viewMatrix, 1, uniformsBegin, uniformsEnd);
				break;
			}
			default:
				ANKI_ASSERT(0);
			}

			break;
		}
		case ShaderVariableDataType::SAMPLER_2D:
		case ShaderVariableDataType::SAMPLER_2D_ARRAY:
		case ShaderVariableDataType::SAMPLER_3D:
		case ShaderVariableDataType::SAMPLER_CUBE:
		{
			ctx.m_commandBuffer->bindTexture(
				0, progVariant.getTextureUnit(progvar), mvar.getValue<TextureResourcePtr>()->getGrTexture());
			break;
		}
		default:
			ANKI_ASSERT(0);
		} // end switch
	}
}

} // end namespace anki
