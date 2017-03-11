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

/// Visitor that sets a uniform
class SetupRenderableVariableVisitor
{
public:
	DrawContext* m_ctx ANKI_DBG_NULLIFY;
	const RenderableDrawer* m_drawer ANKI_DBG_NULLIFY;
	WeakArray<U8> m_uniformBuffer;
	const MaterialVariant* m_variant ANKI_DBG_NULLIFY;
	F32 m_flod;

	/// Set a uniform in a client block
	template<typename T>
	void uniSet(const MaterialVariable& mtlVar, const T* value, U32 size)
	{
		mtlVar.writeShaderBlockMemory<T>(
			*m_variant, value, size, &m_uniformBuffer[0], &m_uniformBuffer[0] + m_uniformBuffer.getSize());
	}

	template<typename TRenderableVariableTemplate>
	Error visit(const TRenderableVariableTemplate& rvar);
};

template<typename TRenderableVariableTemplate>
Error SetupRenderableVariableVisitor::visit(const TRenderableVariableTemplate& rvar)
{
	using DataType = typename TRenderableVariableTemplate::Type;
	const MaterialVariable& mvar = rvar.getMaterialVariable();

	// Array size
	U cachedTrfs = m_ctx->m_cachedTrfCount;
	ANKI_ASSERT(cachedTrfs <= MAX_INSTANCES);

	// Set uniform
	//
	const Mat4& vp = m_ctx->m_viewProjMat;
	const Mat4& v = m_ctx->m_viewMat;

	switch(mvar.getBuiltin())
	{
	case BuiltinMaterialVariableId::NONE:
		uniSet<DataType>(mvar, &rvar.getValue(), 1);
		break;
	case BuiltinMaterialVariableId::MVP_MATRIX:
	{
		ANKI_ASSERT(cachedTrfs > 0);

		DynamicArrayAuto<Mat4> mvp(m_drawer->m_r->getFrameAllocator());
		mvp.create(cachedTrfs);

		for(U i = 0; i < cachedTrfs; i++)
		{
			mvp[i] = vp * m_ctx->m_cachedTrfs[i];
		}

		uniSet(mvar, &mvp[0], cachedTrfs);
		break;
	}
	case BuiltinMaterialVariableId::MV_MATRIX:
	{
		ANKI_ASSERT(cachedTrfs > 0);

		DynamicArrayAuto<Mat4> mv(m_drawer->m_r->getFrameAllocator());
		mv.create(cachedTrfs);

		for(U i = 0; i < cachedTrfs; i++)
		{
			mv[i] = v * m_ctx->m_cachedTrfs[i];
		}

		uniSet(mvar, &mv[0], cachedTrfs);
		break;
	}
	case BuiltinMaterialVariableId::VP_MATRIX:
		ANKI_ASSERT(cachedTrfs == 0 && "Cannot have transform");
		uniSet(mvar, &vp, 1);
		break;
	case BuiltinMaterialVariableId::NORMAL_MATRIX:
	{
		ANKI_ASSERT(cachedTrfs > 0);

		DynamicArrayAuto<Mat3> normMats(m_drawer->m_r->getFrameAllocator());
		normMats.create(cachedTrfs);

		for(U i = 0; i < cachedTrfs; i++)
		{
			Mat4 mv = v * m_ctx->m_cachedTrfs[i];
			normMats[i] = mv.getRotationPart();
			normMats[i].reorthogonalize();
		}

		uniSet(mvar, &normMats[0], cachedTrfs);
		break;
	}
	case BuiltinMaterialVariableId::BILLBOARD_MVP_MATRIX:
	{
		// Calc the billboard rotation matrix
		Mat3 rot = v.getRotationPart().getTransposed();

		DynamicArrayAuto<Mat4> bmvp(m_drawer->m_r->getFrameAllocator());
		bmvp.create(cachedTrfs);

		for(U i = 0; i < cachedTrfs; i++)
		{
			Mat4 trf = m_ctx->m_cachedTrfs[i];
			trf.setRotationPart(rot);
			bmvp[i] = vp * trf;
		}

		uniSet(mvar, &bmvp[0], cachedTrfs);
		break;
	}
	case BuiltinMaterialVariableId::MAX_TESS_LEVEL:
	{
		const RenderComponentVariable& base = rvar;
		F32 maxtess = base.getValue<F32>();
		F32 tess = 0.0;

		if(m_flod >= 1.0)
		{
			tess = 1.0;
		}
		else
		{
			tess = maxtess - m_flod * maxtess;
			tess = std::max(tess, 1.0f);
		}

		uniSet(mvar, &tess, 1);
		break;
	}
	case BuiltinMaterialVariableId::MS_DEPTH_MAP:
		// Do nothing
		break;
	default:
		ANKI_ASSERT(0);
		break;
	}

	return ErrorCode::NONE;
}

// Texture specialization
template<>
void SetupRenderableVariableVisitor::uniSet<TextureResourcePtr>(
	const MaterialVariable& mtlvar, const TextureResourcePtr* values, U32 size)
{
	ANKI_ASSERT(size == 1);
	ANKI_ASSERT(values);
	m_ctx->m_cmdb->bindTexture(0, mtlvar.getTextureUnit(), (*values)->getGrTexture());
}

RenderableDrawer::~RenderableDrawer()
{
}

void RenderableDrawer::setupUniforms(DrawContext& ctx, CompleteRenderingBuildInfo& build)
{
	const Material& mtl = build.m_rc->getMaterial();
	const MaterialVariant& variant = mtl.getVariant(build.m_in.m_key);

	// Get some memory for uniforms
	U8* uniforms = static_cast<U8*>(m_r->getStagingGpuMemoryManager().allocateFrame(
		variant.getDefaultBlockSize(), StagingGpuMemoryType::UNIFORM, ctx.m_uboToken));

	// Call the visitor
	SetupRenderableVariableVisitor visitor;
	visitor.m_ctx = &ctx;
	visitor.m_drawer = this;
	visitor.m_uniformBuffer = WeakArray<U8>(uniforms, variant.getDefaultBlockSize());
	visitor.m_variant = &variant;
	visitor.m_flod = build.m_flod;

	for(auto it = build.m_rc->getVariablesBegin(); it != build.m_rc->getVariablesEnd(); ++it)
	{
		RenderComponentVariable* rvar = *it;

		if(variant.variableActive(rvar->getMaterialVariable()))
		{
			Error err = rvar->acceptVisitor(visitor);
			(void)err;
		}
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
			cmdb->bindVertexBuffer(i, binding.m_buffer, binding.m_offset, binding.m_stride);
		}
		else
		{
			ANKI_ASSERT(!!(binding.m_token));
			cmdb->bindVertexBuffer(i, binding.m_token.m_buffer, binding.m_token.m_offset, binding.m_stride);
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
	const Material& mtl = renderable.getMaterial();

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
	crntBuild.m_in.m_key.m_tessellation = m_r->getTessellationEnabled() && mtl.getTessellationEnabled()
		&& crntBuild.m_in.m_key.m_lod == 0 && ctx.m_pass != Pass::SM;
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
