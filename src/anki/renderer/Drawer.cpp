// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Drawer.h>
#include <anki/resource/ShaderResource.h>
#include <anki/scene/FrustumComponent.h>
#include <anki/resource/Material.h>
#include <anki/scene/RenderComponent.h>
#include <anki/scene/Visibility.h>
#include <anki/scene/SceneGraph.h>
#include <anki/resource/TextureResource.h>
#include <anki/renderer/Renderer.h>
#include <anki/core/Trace.h>
#include <anki/util/Logger.h>

namespace anki
{

/// Check if the drawcalls can be merged.
static Bool canMergeBuildInfo(const RenderingBuildInfoOut& a, const RenderingBuildInfoOut& b)
{
	if(!a.m_hasTransform || !b.m_hasTransform)
	{
		// Cannot merge if there is no transform
		return false;
	}

	ANKI_ASSERT(a.m_hasTransform == b.m_hasTransform);

	if(a.m_program != b.m_program)
	{
		return false;
	}

	if(a.m_vertexBufferBindingCount != b.m_vertexBufferBindingCount
		|| a.m_vertexAttributeCount != b.m_vertexAttributeCount)
	{
		return false;
	}

	for(U i = 0; i < a.m_vertexBufferBindingCount; ++i)
	{
		if(a.m_vertexBufferBindings[i] != b.m_vertexBufferBindings[i])
		{
			return false;
		}
	}

	for(U i = 0; i < a.m_vertexAttributeCount; ++i)
	{
		if(a.m_vertexAttributes[i] != b.m_vertexAttributes[i])
		{
			return false;
		}
	}

	if(a.m_indexBuffer != b.m_indexBuffer)
	{
		return false;
	}

	if(a.m_indexBufferToken != b.m_indexBufferToken)
	{
		return false;
	}

	// Drawcall
	if(a.m_drawArrays != b.m_drawArrays)
	{
		return false;
	}

	if(a.m_drawArrays && a.m_drawcall.m_arrays != b.m_drawcall.m_arrays)
	{
		return false;
	}

	if(!a.m_drawArrays && a.m_drawcall.m_elements != b.m_drawcall.m_elements)
	{
		return false;
	}

	if(a.m_topology != b.m_topology)
	{
		return false;
	}

	return true;
}

static void resetRenderingBuildInfoOut(RenderingBuildInfoOut& b)
{
	b.m_hasTransform = false;
	b.m_program = {};

	b.m_vertexBufferBindingCount = 0;
	b.m_vertexAttributeCount = 0;

	b.m_indexBuffer = {};
	b.m_indexBufferToken = {};

	b.m_drawcall.m_elements = DrawElementsIndirectInfo();
	b.m_drawArrays = false;
	b.m_topology = PrimitiveTopology::TRIANGLES;
}

class CompleteRenderingBuildInfo
{
public:
	F32 m_flod = 0.0;
	RenderComponent* m_rc = nullptr;
	RenderingBuildInfoIn m_in;
	RenderingBuildInfoOut m_out;
};

/// Drawer's context
class DrawContext
{
public:
	Pass m_pass;
	Mat4 m_viewMat;
	Mat4 m_viewProjMat;
	CommandBufferPtr m_cmdb;

	const VisibleNode* m_visibleNode = nullptr;

	Array<Mat4, MAX_INSTANCES> m_cachedTrfs;
	U m_cachedTrfCount = 0;

	StagingGpuMemoryToken m_uboToken;

	U m_nodeProcessedCount = 0;

	Array<CompleteRenderingBuildInfo, 2> m_buildInfo;
	U m_crntBuildInfo = 0;
};

RenderableDrawer::~RenderableDrawer()
{
}

void RenderableDrawer::setupUniforms(DrawContext& ctx, CompleteRenderingBuildInfo& build)
{
	const Material& mtl = build.m_rc->getMaterial();
	const MaterialVariant& variant = mtl.getOrCreateVariant(build.m_in.m_key);
	const ShaderProgramResourceVariant& progVariant = variant.getShaderProgramResourceVariant();

	const U cachedTrfs = ctx.m_cachedTrfCount;
	ANKI_ASSERT(cachedTrfs <= MAX_INSTANCES);

	// Get some memory for uniforms
	U8* uniforms = static_cast<U8*>(m_r->getStagingGpuMemoryManager().allocateFrame(
		variant.getUniformBlockSize(), StagingGpuMemoryType::UNIFORM, ctx.m_uboToken));
	void* const uniformsBegin = uniforms;
	const void* const uniformsEnd = uniforms + variant.getUniformBlockSize();

	// Iterate variables
	for(auto it = build.m_rc->getVariablesBegin(); it != build.m_rc->getVariablesEnd(); ++it)
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
				ANKI_ASSERT(cachedTrfs > 0);

				DynamicArrayAuto<Mat3> normMats(m_r->getFrameAllocator());
				normMats.create(cachedTrfs);

				for(U i = 0; i < cachedTrfs; i++)
				{
					Mat4 mv = ctx.m_viewMat * ctx.m_cachedTrfs[i];
					normMats[i] = mv.getRotationPart();
					normMats[i].reorthogonalize();
				}

				progVariant.writeShaderBlockMemory(progvar, &normMats[0], cachedTrfs, uniformsBegin, uniformsEnd);
				break;
			}
			case BuiltinMaterialVariableId::CAMERA_ROTATION_MATRIX:
			{
				Mat3 rot = ctx.m_viewMat.getRotationPart().getTransposed();
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
				ANKI_ASSERT(cachedTrfs > 0);

				DynamicArrayAuto<Mat4> mvp(m_r->getFrameAllocator());
				mvp.create(cachedTrfs);

				for(U i = 0; i < cachedTrfs; i++)
				{
					mvp[i] = ctx.m_viewProjMat * ctx.m_cachedTrfs[i];
				}

				progVariant.writeShaderBlockMemory(progvar, &mvp[0], cachedTrfs, uniformsBegin, uniformsEnd);
				break;
			}
			case BuiltinMaterialVariableId::MODEL_VIEW_MATRIX:
			{
				ANKI_ASSERT(cachedTrfs > 0);

				DynamicArrayAuto<Mat4> mv(m_r->getFrameAllocator());
				mv.create(cachedTrfs);

				for(U i = 0; i < cachedTrfs; i++)
				{
					mv[i] = ctx.m_viewMat * ctx.m_cachedTrfs[i];
				}

				progVariant.writeShaderBlockMemory(progvar, &mv[0], cachedTrfs, uniformsBegin, uniformsEnd);
				break;
			}
			case BuiltinMaterialVariableId::VIEW_PROJECTION_MATRIX:
			{
				ANKI_ASSERT(cachedTrfs == 0 && "Cannot have transform");
				progVariant.writeShaderBlockMemory(progvar, &ctx.m_viewProjMat, 1, uniformsBegin, uniformsEnd);
				break;
			}
			case BuiltinMaterialVariableId::VIEW_MATRIX:
			{
				progVariant.writeShaderBlockMemory(progvar, &ctx.m_viewMat, 1, uniformsBegin, uniformsEnd);
				break;
			}
			default:
				ANKI_ASSERT(0);
			}
		}
		case ShaderVariableDataType::SAMPLER_2D:
		case ShaderVariableDataType::SAMPLER_2D_ARRAY:
		case ShaderVariableDataType::SAMPLER_3D:
		case ShaderVariableDataType::SAMPLER_CUBE:
		{
			ctx.m_cmdb->bindTexture(
				0, progVariant.getTextureUnit(progvar), mvar.getValue<TextureResourcePtr>()->getGrTexture());
			break;
		}
		default:
			ANKI_ASSERT(0);
		} // end switch
	}
}

Error RenderableDrawer::drawRange(Pass pass,
	const Mat4& viewMat,
	const Mat4& viewProjMat,
	CommandBufferPtr cmdb,
	const VisibleNode* begin,
	const VisibleNode* end)
{
	ANKI_ASSERT(begin && end && begin < end);

	DrawContext ctx;
	ctx.m_viewMat = viewMat;
	ctx.m_viewProjMat = viewProjMat;
	ctx.m_pass = pass;
	ctx.m_cmdb = cmdb;

	for(; begin != end; ++begin)
	{
		ctx.m_visibleNode = begin;

		ANKI_CHECK(drawSingle(ctx));

		++ctx.m_nodeProcessedCount;
	}

	// Flush the last drawcall
	CompleteRenderingBuildInfo& build = ctx.m_buildInfo[!ctx.m_crntBuildInfo];
	ANKI_CHECK(flushDrawcall(ctx, build));

	return ErrorCode::NONE;
}

Error RenderableDrawer::flushDrawcall(DrawContext& ctx, CompleteRenderingBuildInfo& build)
{
	RenderComponent& rc = *build.m_rc;

	// Get the new info with the correct instance count
	if(ctx.m_cachedTrfCount > 1)
	{
		build.m_in.m_key.m_instanceCount = ctx.m_cachedTrfCount;
		ANKI_CHECK(rc.buildRendering(build.m_in, build.m_out));
	}

	// Enqueue uniform state updates
	setupUniforms(ctx, build);

	// Finaly, touch the command buffer
	CommandBufferPtr& cmdb = ctx.m_cmdb;

	cmdb->bindUniformBuffer(0, 0, ctx.m_uboToken.m_buffer, ctx.m_uboToken.m_offset, ctx.m_uboToken.m_range);
	cmdb->bindShaderProgram(build.m_out.m_program);

	for(U i = 0; i < build.m_out.m_vertexBufferBindingCount; ++i)
	{
		const RenderingVertexBufferBinding& binding = build.m_out.m_vertexBufferBindings[i];
		if(binding.m_buffer)
		{
			cmdb->bindVertexBuffer(i, binding.m_buffer, binding.m_offset, binding.m_stride, binding.m_stepRate);
		}
		else
		{
			ANKI_ASSERT(!!(binding.m_token));
			cmdb->bindVertexBuffer(
				i, binding.m_token.m_buffer, binding.m_token.m_offset, binding.m_stride, binding.m_stepRate);
		}
	}

	for(U i = 0; i < build.m_out.m_vertexAttributeCount; ++i)
	{
		const RenderingVertexAttributeInfo& attrib = build.m_out.m_vertexAttributes[i];

		cmdb->setVertexAttribute(i, attrib.m_bufferBinding, attrib.m_format, attrib.m_relativeOffset);
	}

	if(!build.m_out.m_drawArrays)
	{
		const DrawElementsIndirectInfo& drawc = build.m_out.m_drawcall.m_elements;

		if(build.m_out.m_indexBuffer)
		{
			cmdb->bindIndexBuffer(build.m_out.m_indexBuffer, 0, IndexType::U16);
		}
		else
		{
			ANKI_ASSERT(!!(build.m_out.m_indexBufferToken));
			cmdb->bindIndexBuffer(
				build.m_out.m_indexBufferToken.m_buffer, build.m_out.m_indexBufferToken.m_offset, IndexType::U16);
		}

		cmdb->drawElements(build.m_out.m_topology,
			drawc.m_count,
			drawc.m_instanceCount,
			drawc.m_firstIndex,
			drawc.m_baseVertex,
			drawc.m_baseInstance);
	}
	else
	{
		const DrawArraysIndirectInfo& drawc = build.m_out.m_drawcall.m_arrays;

		cmdb->drawArrays(
			build.m_out.m_topology, drawc.m_count, drawc.m_instanceCount, drawc.m_first, drawc.m_baseInstance);
	}

	// Rendered something, reset the cached transforms
	if(ctx.m_cachedTrfCount > 1)
	{
		ANKI_TRACE_INC_COUNTER(RENDERER_MERGED_DRAWCALLS, ctx.m_cachedTrfCount - 1);
	}
	ctx.m_cachedTrfCount = 0;

	return ErrorCode::NONE;
}

Error RenderableDrawer::drawSingle(DrawContext& ctx)
{
	// Get components
	RenderComponent& renderable = ctx.m_visibleNode->m_node->getComponent<RenderComponent>();

	// Get info of current
	CompleteRenderingBuildInfo& crntBuild = ctx.m_buildInfo[ctx.m_crntBuildInfo];
	CompleteRenderingBuildInfo& prevBuild = ctx.m_buildInfo[!ctx.m_crntBuildInfo];
	ctx.m_crntBuildInfo = !ctx.m_crntBuildInfo;

	// Fill the crntBuild
	F32 flod = m_r->calculateLod(sqrt(ctx.m_visibleNode->m_frustumDistanceSquared));
	flod = min<F32>(flod, MAX_LODS - 1);

	crntBuild.m_rc = &renderable;
	crntBuild.m_flod = flod;

	crntBuild.m_in.m_key.m_lod = flod;
	crntBuild.m_in.m_key.m_pass = ctx.m_pass;
	crntBuild.m_in.m_key.m_instanceCount = 1;
	crntBuild.m_in.m_subMeshIndicesArray = &ctx.m_visibleNode->m_spatialIndices[0];
	crntBuild.m_in.m_subMeshIndicesCount = ctx.m_visibleNode->m_spatialsCount;

	resetRenderingBuildInfoOut(crntBuild.m_out);
	ANKI_CHECK(renderable.buildRendering(crntBuild.m_in, crntBuild.m_out));

	if(ANKI_UNLIKELY(ctx.m_nodeProcessedCount == 0))
	{
		// First drawcall, cache it

		if(crntBuild.m_out.m_hasTransform)
		{
			ctx.m_cachedTrfs[ctx.m_cachedTrfCount++] = crntBuild.m_out.m_transform;
		}
	}
	else if(crntBuild.m_out.m_hasTransform && canMergeBuildInfo(crntBuild.m_out, prevBuild.m_out)
		&& ctx.m_cachedTrfCount < MAX_INSTANCES - 1)
	{
		// Can merge, will cache the drawcall and skip the drawcall

		ctx.m_cachedTrfs[ctx.m_cachedTrfCount++] = crntBuild.m_out.m_transform;
	}
	else
	{
		// Cannot merge, flush the previous build info

		ANKI_CHECK(flushDrawcall(ctx, prevBuild));

		// Cache the current build
		if(crntBuild.m_out.m_hasTransform)
		{
			ctx.m_cachedTrfs[ctx.m_cachedTrfCount++] = crntBuild.m_out.m_transform;
		}
	}

	return ErrorCode::NONE;
}

} // end namespace anki
