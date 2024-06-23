// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DGraphicsState.h>
#include <AnKi/Gr/D3D/D3DShaderProgram.h>
#include <AnKi/Gr/BackendCommon/Functions.h>

namespace anki {

static void getVertexAttributeSemanticInfo(VertexAttributeSemantic x, const Char*& str, U32& idx)
{
	switch(x)
	{
	case VertexAttributeSemantic::kPosition:
		str = "POSITION";
		idx = 0;
		break;
	case VertexAttributeSemantic::kNormal:
		str = "NORMAL";
		idx = 0;
		break;
	case VertexAttributeSemantic::kTexCoord:
		str = "TEX_COORD";
		idx = 0;
		break;
	case VertexAttributeSemantic::kColor:
		str = "COLOR";
		idx = 0;
		break;
	case VertexAttributeSemantic::kMisc0:
		str = "MISC";
		idx = 0;
		break;
	case VertexAttributeSemantic::kMisc1:
		str = "MISC";
		idx = 1;
		break;
	case VertexAttributeSemantic::kMisc2:
		str = "MISC";
		idx = 2;
		break;
	case VertexAttributeSemantic::kMisc3:
		str = "MISC";
		idx = 3;
		break;
	default:
		ANKI_ASSERT(0);
	};
}

GraphicsPipelineFactory::~GraphicsPipelineFactory()
{
	for(auto pso : m_map)
	{
		safeRelease(pso);
	}
}

void GraphicsPipelineFactory::flushState(GraphicsStateTracker& state, D3D12GraphicsCommandListX& cmdList)
{
	const GraphicsStateTracker::StaticState& staticState = state.m_staticState;
	GraphicsStateTracker::DynamicState& dynState = state.m_dynState;

	// Set dynamic state
	const auto& ss = staticState.m_stencil;
	const Bool stencilTestEnabled = anki::stencilTestEnabled(ss.m_face[0].m_fail, ss.m_face[0].m_stencilPassDepthFail,
															 ss.m_face[0].m_stencilPassDepthPass, ss.m_face[0].m_compare)
									|| anki::stencilTestEnabled(ss.m_face[1].m_fail, ss.m_face[1].m_stencilPassDepthFail,
																ss.m_face[1].m_stencilPassDepthPass, ss.m_face[1].m_compare);

	const Bool hasStencilRt =
		staticState.m_misc.m_depthStencilFormat != Format::kNone && getFormatInfo(staticState.m_misc.m_depthStencilFormat).isStencil();

	if(stencilTestEnabled && hasStencilRt && dynState.m_stencilRefDirty)
	{
		dynState.m_stencilRefDirty = false;
		cmdList.OMSetStencilRef(dynState.m_stencilFaces[0].m_ref);
	}

	if(dynState.m_topologyDirty)
	{
		dynState.m_topologyDirty = false;
		cmdList.IASetPrimitiveTopology(convertPrimitiveTopology2(staticState.m_ia.m_topology));
	}

	if(dynState.m_viewportDirty)
	{
		dynState.m_viewportDirty = false;
		const D3D12_VIEWPORT vp = {.TopLeftX = F32(dynState.m_viewport[0]),
								   .TopLeftY = F32(dynState.m_viewport[1]),
								   .Width = F32(dynState.m_viewport[2]),
								   .Height = F32(dynState.m_viewport[3]),
								   .MinDepth = 0.0f,
								   .MaxDepth = 1.0f};
		cmdList.RSSetViewports(1, &vp);
	}

	if(dynState.m_scissorDirty)
	{
		dynState.m_scissorDirty = false;

		const U32 minx = max(dynState.m_scissor[0], dynState.m_viewport[0]);
		const U32 miny = max(dynState.m_scissor[1], dynState.m_viewport[1]);
		const U32 right = min(dynState.m_scissor[0] + dynState.m_scissor[2], dynState.m_viewport[0] + dynState.m_viewport[2]);
		const U32 bottom = min(dynState.m_scissor[1] + dynState.m_scissor[3], dynState.m_viewport[1] + dynState.m_viewport[3]);

		const D3D12_RECT rect = {.left = I32(minx), .top = I32(miny), .right = I32(right), .bottom = I32(bottom)};
		cmdList.RSSetScissorRects(1, &rect);
	}

	const Bool rebindPso = state.updateHashes();

	// Find the PSO
	ID3D12PipelineState* pso = nullptr;
	{
		RLockGuard lock(m_mtx);

		auto it = m_map.find(state.m_globalHash);
		if(it != m_map.getEnd())
		{
			pso = *it;
		}
	}

	if(pso) [[likely]]
	{
		if(rebindPso)
		{
			cmdList.SetPipelineState(pso);
		}

		return;
	}

	// PSO not found, proactively create it WITHOUT a lock (we dont't want to serialize pipeline creation)

	const ShaderProgramImpl& prog = static_cast<const ShaderProgramImpl&>(*staticState.m_shaderProg);

	// Vertex input
	Array<D3D12_INPUT_ELEMENT_DESC, U32(VertexAttributeSemantic::kCount)> inputElementDescs;
	U32 inputElementDescCount = 0;
	for(VertexAttributeSemantic i : EnumIterable<VertexAttributeSemantic>())
	{
		if(staticState.m_vert.m_activeAttribs.get(i))
		{
			D3D12_INPUT_ELEMENT_DESC& elem = inputElementDescs[inputElementDescCount++];

			getVertexAttributeSemanticInfo(i, elem.SemanticName, elem.SemanticIndex);
			elem.Format = convertFormat(staticState.m_vert.m_attribs[i].m_fmt);
			elem.InputSlot = staticState.m_vert.m_attribs[i].m_binding;
			elem.AlignedByteOffset = staticState.m_vert.m_attribs[i].m_relativeOffset;
			elem.InputSlotClass = (staticState.m_vert.m_bindings[staticState.m_vert.m_attribs[i].m_binding].m_stepRate == VertexStepRate::kVertex)
									  ? D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA
									  : D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
			elem.InstanceDataStepRate = (elem.InputSlotClass == D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA) ? 0 : 1;
		}
	}

	// Blending
	D3D12_BLEND_DESC blendDesc = {.AlphaToCoverageEnable = staticState.m_blend.m_alphaToCoverage, .IndependentBlendEnable = true};
	for(U32 i = 0; i < kMaxColorRenderTargets; ++i)
	{
		if(staticState.m_misc.m_colorRtMask.get(i))
		{
			const auto& in = staticState.m_blend.m_colorRts[i];
			D3D12_RENDER_TARGET_BLEND_DESC& out = blendDesc.RenderTarget[i];
			out.BlendEnable = blendingEnabled(in.m_srcRgb, in.m_dstRgb, in.m_srcA, in.m_dstA, in.m_funcRgb, in.m_funcA);
			out.SrcBlend = convertBlendFactor(in.m_srcRgb);
			out.DestBlend = convertBlendFactor(in.m_dstRgb);
			out.BlendOp = convertBlendOperation(in.m_funcRgb);
			out.SrcBlendAlpha = convertBlendFactor(in.m_srcA);
			out.DestBlendAlpha = convertBlendFactor(in.m_dstA);
			out.BlendOpAlpha = convertBlendOperation(in.m_funcA);
			out.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		}
	}

	// DS
	D3D12_DEPTH_STENCIL_DESC2 dsDesc = {};
	if(staticState.m_misc.m_depthStencilFormat != Format::kNone)
	{
		Array<D3D12_DEPTH_STENCILOP_DESC1, 2> stencilDescs;
		if(hasStencilRt)
		{
			for(U32 w = 0; w < 2; ++w)
			{
				stencilDescs[w].StencilFailOp = convertStencilOperation(staticState.m_stencil.m_face[w].m_fail);
				stencilDescs[w].StencilDepthFailOp = convertStencilOperation(staticState.m_stencil.m_face[w].m_stencilPassDepthFail);
				stencilDescs[w].StencilPassOp = convertStencilOperation(staticState.m_stencil.m_face[w].m_stencilPassDepthPass);
				stencilDescs[w].StencilFunc = convertComparisonFunc(staticState.m_stencil.m_face[w].m_compare);

				ANKI_ASSERT(
					!stencilTestEnabled
					|| (staticState.m_stencil.m_face[w].m_compareMask != 0x5A5A5A5A && staticState.m_stencil.m_face[w].m_writeMask != 0x5A5A5A5A));
				stencilDescs[w].StencilReadMask = U8(staticState.m_stencil.m_face[w].m_compareMask);
				stencilDescs[w].StencilWriteMask = U8(staticState.m_stencil.m_face[w].m_writeMask);
			}
		}
		else
		{
			for(U32 w = 0; w < 2; ++w)
			{
				stencilDescs[w].StencilFailOp = D3D12_STENCIL_OP_KEEP;
				stencilDescs[w].StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
				stencilDescs[w].StencilPassOp = D3D12_STENCIL_OP_KEEP;
				stencilDescs[w].StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

				stencilDescs[w].StencilReadMask = 0;
				stencilDescs[w].StencilWriteMask = 0;
			}
		}

		dsDesc = {.DepthEnable = depthTestEnabled(staticState.m_depth.m_compare, staticState.m_depth.m_writeEnabled),
				  .DepthWriteMask = staticState.m_depth.m_writeEnabled ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO,
				  .DepthFunc = convertCompareOperation(staticState.m_depth.m_compare),
				  .StencilEnable = stencilTestEnabled,
				  .FrontFace = stencilDescs[0],
				  .BackFace = stencilDescs[1],
				  .DepthBoundsTestEnable = false};
	}

	// Rast state
	const D3D12_RASTERIZER_DESC2 rastDesc = {.FillMode = convertFillMode(staticState.m_rast.m_fillMode),
											 .CullMode = convertCullMode(staticState.m_rast.m_cullMode),
											 .FrontCounterClockwise = true,
											 .DepthBias = staticState.m_rast.m_depthBias,
											 .DepthBiasClamp = staticState.m_rast.m_depthBiasClamp,
											 .SlopeScaledDepthBias = staticState.m_rast.m_slopeScaledDepthBias};

	// Misc
	D3D12_RT_FORMAT_ARRAY rtFormats = {};
	for(U32 i = 0; i < kMaxColorRenderTargets; ++i)
	{
		if(staticState.m_misc.m_colorRtMask.get(i))
		{
			rtFormats.RTFormats[i] = convertFormat(staticState.m_misc.m_colorRtFormats[i]);
			rtFormats.NumRenderTargets = i + 1;
		}
	}

	const DXGI_SAMPLE_DESC sampleDesc = {.Count = 1, .Quality = 0};

	// Main descriptor
#define ANKI_SET_IR(member, stage) \
	if(!!(prog.getShaderTypes() & ShaderTypeBit::stage)) \
	{ \
		desc.member = prog.m_graphics.m_shaderCreateInfos[ShaderType::stage]; \
	}

	CD3DX12_PIPELINE_STATE_STREAM5 desc = {};
	desc.Flags = D3D12_PIPELINE_STATE_FLAG_DYNAMIC_DEPTH_BIAS;
	desc.pRootSignature = &prog.m_rootSignature->getD3DRootSignature();
	desc.InputLayout = D3D12_INPUT_LAYOUT_DESC{.pInputElementDescs = inputElementDescs.getBegin(), .NumElements = inputElementDescCount};
	desc.PrimitiveTopologyType = convertPrimitiveTopology(staticState.m_ia.m_topology);
	ANKI_SET_IR(VS, kVertex)
	ANKI_SET_IR(GS, kGeometry)
	ANKI_SET_IR(HS, kTessellationControl)
	ANKI_SET_IR(DS, kTessellationEvaluation)
	ANKI_SET_IR(PS, kFragment)
	ANKI_SET_IR(AS, kTask)
	ANKI_SET_IR(MS, kMesh)
	desc.BlendState = CD3DX12_BLEND_DESC(blendDesc);
	desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC2(dsDesc);
	desc.DSVFormat = convertFormat(staticState.m_misc.m_depthStencilFormat);
	desc.RasterizerState = CD3DX12_RASTERIZER_DESC2(rastDesc);
	desc.RTVFormats = rtFormats;
	desc.SampleDesc = sampleDesc;
	desc.SampleMask = kMaxU32;

#undef ANKI_SET_IR

	// Create the PSO
	const D3D12_PIPELINE_STATE_STREAM_DESC streamDec = {.SizeInBytes = sizeof(desc), .pPipelineStateSubobjectStream = &desc};
	ANKI_D3D_CHECKF(getDevice().CreatePipelineState(&streamDec, IID_PPV_ARGS(&pso)));

	// Now try to add the PSO to the hashmap
	{
		WLockGuard lock(m_mtx);

		auto it = m_map.find(state.m_globalHash);
		if(it == m_map.getEnd())
		{
			// Not found, add it
			m_map.emplace(state.m_globalHash, pso);
		}
		else
		{
			// Found, remove the PSO that was proactively created and use the old one
			safeRelease(pso);
			pso = *it;
		}
	}

	// Final thing, bind the PSO
	cmdList.SetPipelineState(pso);
}

} // end namespace anki
