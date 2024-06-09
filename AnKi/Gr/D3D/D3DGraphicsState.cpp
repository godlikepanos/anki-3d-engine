// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DGraphicsState.h>
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

Bool GraphicsStateTracker::updateHashes()
{
	if(m_hashes.m_vert == 0)
	{
		if(m_state.m_vert.m_activeAttribs.getAnySet())
		{
			m_hashes.m_vert = 0xC0FEE;

			for(VertexAttributeSemantic i : EnumIterable<VertexAttributeSemantic>())
			{
				if(!m_state.m_vert.m_activeAttribs.get(i))
				{
					continue;
				}

				ANKI_ASSERT(m_state.m_vert.m_attribsSetMask.get(i) && "Forgot to set the vert attribute");
				m_hashes.m_vert = appendObjectHash(m_state.m_vert.m_attribs[i], m_hashes.m_vert);

				ANKI_ASSERT(m_state.m_vert.m_bindingsSetMask.get(m_state.m_vert.m_attribs[i].m_binding) && "Forgot to inform about the vert binding");
				m_hashes.m_vert = appendObjectHash(m_state.m_vert.m_bindings[m_state.m_vert.m_attribs[i].m_binding], m_hashes.m_vert);
			}
		}
		else
		{
			m_hashes.m_vert = 0xC0FEE;
		}
	}

	if(m_hashes.m_rast == 0)
	{
		m_hashes.m_rast = computeObjectHash(&m_state.m_rast);
	}

	if(m_hashes.m_depthStencil == 0)
	{
		const Bool hasStencil =
			m_state.m_misc.m_depthStencilFormat != Format::kNone && getFormatInfo(m_state.m_misc.m_depthStencilFormat).isStencil();

		const Bool hasDepth = m_state.m_misc.m_depthStencilFormat != Format::kNone && getFormatInfo(m_state.m_misc.m_depthStencilFormat).isDepth();

		m_hashes.m_depthStencil = 0xC0FEE;

		if(hasStencil)
		{
			m_hashes.m_depthStencil = appendObjectHash(m_state.m_stencil, m_hashes.m_depthStencil);
		}

		if(hasDepth)
		{
			m_hashes.m_depthStencil = appendObjectHash(m_state.m_depth, m_hashes.m_depthStencil);
		}
	}

	if(m_hashes.m_blend == 0)
	{
		if(m_state.m_misc.m_colorRtMask.getAnySet())
		{
			m_hashes.m_blend = m_state.m_blend.m_alphaToCoverage;

			for(U32 i = 0; i < kMaxColorRenderTargets; ++i)
			{
				if(m_state.m_misc.m_colorRtMask.get(i))
				{
					m_hashes.m_blend = appendObjectHash(m_state.m_blend.m_colorRts[i], m_hashes.m_blend);
				}
			}
		}
		else
		{
			m_hashes.m_blend = 0xC0FFE;
		}
	}

	if(m_hashes.m_misc == 0)
	{
		Array<U32, kMaxColorRenderTargets + 3> toHash;
		U32 toHashCount = 0;

		toHash[toHashCount++] = m_state.m_misc.m_colorRtMask.getData()[0];
		for(U32 i = 0; i < kMaxColorRenderTargets; ++i)
		{
			if(m_state.m_misc.m_colorRtMask.get(i))
			{
				toHash[toHashCount++] = U32(m_state.m_misc.m_colorRtFormats[i]);
			}
		}

		if(m_state.m_misc.m_depthStencilFormat != Format::kNone)
		{
			toHash[toHashCount++] = U32(m_state.m_misc.m_depthStencilFormat);
		}

		toHash[toHashCount++] = U32(m_state.m_misc.m_topology);

		m_hashes.m_misc = computeHash(toHash.getBegin(), sizeof(toHash[0]) * toHashCount);
	}

	if(m_hashes.m_shaderProg == 0)
	{
		m_hashes.m_shaderProg = m_state.m_shaderProg->getUuid();
	}

	// Compute complete hash
	const U64 globalHash = computeObjectHash(m_hashes);

	if(globalHash != m_globalHash)
	{
		m_globalHash = globalHash;
		return true;
	}
	else
	{
		return false;
	}
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
	// Set dynamic state
	if(state.m_dynState.m_stencilRefMaskDirty && state.m_state.m_misc.m_depthStencilFormat != Format::kNone
	   && getFormatInfo(state.m_state.m_misc.m_depthStencilFormat).isStencil())
	{
		state.m_dynState.m_stencilRefMaskDirty = false;
		cmdList.OMSetStencilRef(state.m_dynState.m_stencilRefMask);
	}

	if(state.m_dynState.m_topologyDirty)
	{
		state.m_dynState.m_topologyDirty = false;
		cmdList.IASetPrimitiveTopology(convertPrimitiveTopology2(state.m_dynState.m_topology));
	}

	if(state.m_dynState.m_viewportDirty)
	{
		state.m_dynState.m_viewportDirty = false;
		const D3D12_VIEWPORT vp = {.TopLeftX = F32(state.m_dynState.m_viewport[0]),
								   .TopLeftY = F32(state.m_dynState.m_viewport[1]),
								   .Width = F32(state.m_dynState.m_viewport[2]),
								   .Height = F32(state.m_dynState.m_viewport[3]),
								   .MinDepth = 0.0f,
								   .MaxDepth = 1.0f};
		cmdList.RSSetViewports(1, &vp);
	}

	if(state.m_dynState.m_scissorDirty)
	{
		state.m_dynState.m_scissorDirty = false;

		const U32 minx = max(state.m_dynState.m_scissor[0], state.m_dynState.m_viewport[0]);
		const U32 miny = max(state.m_dynState.m_scissor[1], state.m_dynState.m_viewport[1]);
		const U32 right =
			min(state.m_dynState.m_scissor[0] + state.m_dynState.m_scissor[2], state.m_dynState.m_viewport[0] + state.m_dynState.m_viewport[2]);
		const U32 bottom =
			min(state.m_dynState.m_scissor[1] + state.m_dynState.m_scissor[3], state.m_dynState.m_viewport[1] + state.m_dynState.m_viewport[3]);

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

	const ShaderProgramImpl& prog = *state.m_state.m_shaderProg;

	// Vertex input
	Array<D3D12_INPUT_ELEMENT_DESC, U32(VertexAttributeSemantic::kCount)> inputElementDescs;
	U32 inputElementDescCount = 0;
	for(VertexAttributeSemantic i : EnumIterable<VertexAttributeSemantic>())
	{
		if(state.m_state.m_vert.m_activeAttribs.get(i))
		{
			D3D12_INPUT_ELEMENT_DESC& elem = inputElementDescs[inputElementDescCount++];

			getVertexAttributeSemanticInfo(i, elem.SemanticName, elem.SemanticIndex);
			elem.Format = DXGI_FORMAT(state.m_state.m_vert.m_attribs[i].m_fmt);
			elem.InputSlot = state.m_state.m_vert.m_attribs[i].m_binding;
			elem.AlignedByteOffset = state.m_state.m_vert.m_attribs[i].m_relativeOffset;
			elem.InputSlotClass = (state.m_state.m_vert.m_bindings[state.m_state.m_vert.m_attribs[i].m_binding].m_stepRate == VertexStepRate::kVertex)
									  ? D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA
									  : D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
			elem.InstanceDataStepRate = (elem.InputSlotClass == D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA) ? 0 : 1;
		}
	}

	// Blending
	D3D12_BLEND_DESC blendDesc = {.AlphaToCoverageEnable = state.m_state.m_blend.m_alphaToCoverage, .IndependentBlendEnable = true};
	for(U32 i = 0; i < kMaxColorRenderTargets; ++i)
	{
		if(state.m_state.m_misc.m_colorRtMask.get(i))
		{
			const auto& in = state.m_state.m_blend.m_colorRts[i];
			D3D12_RENDER_TARGET_BLEND_DESC& out = blendDesc.RenderTarget[i];
			out.BlendEnable = blendingDisabled(in.m_srcRgb, in.m_dstRgb, in.m_srcA, in.m_dstA, in.m_funcRgb, in.m_funcA);
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
	if(state.m_state.m_misc.m_depthStencilFormat != Format::kNone)
	{
		const Bool stencilEnabled = !stencilTestDisabled(state.m_state.m_stencil.m_fail[0], state.m_state.m_stencil.m_stencilPassDepthFail[0],
														 state.m_state.m_stencil.m_stencilPassDepthPass[0], state.m_state.m_stencil.m_compare[0])
									|| !stencilTestDisabled(state.m_state.m_stencil.m_fail[1], state.m_state.m_stencil.m_stencilPassDepthFail[1],
															state.m_state.m_stencil.m_stencilPassDepthPass[1], state.m_state.m_stencil.m_compare[1]);

		Array<D3D12_DEPTH_STENCILOP_DESC1, 2> stencilDescs;
		for(U32 w = 0; w < 2; ++w)
		{
			stencilDescs[w].StencilFailOp = convertStencilOperation(state.m_state.m_stencil.m_fail[w]);
			stencilDescs[w].StencilDepthFailOp = convertStencilOperation(state.m_state.m_stencil.m_stencilPassDepthFail[w]);
			stencilDescs[w].StencilPassOp = convertStencilOperation(state.m_state.m_stencil.m_stencilPassDepthPass[w]);
			stencilDescs[w].StencilFunc = convertComparisonFunc(state.m_state.m_stencil.m_compare[w]);
			stencilDescs[w].StencilReadMask = U8(state.m_state.m_stencil.m_compareMask[w]);
			stencilDescs[w].StencilWriteMask = U8(state.m_state.m_stencil.m_writeMask[w]);
		}

		dsDesc = {.DepthEnable = state.m_state.m_depth.m_compare != CompareOperation::kAlways || state.m_state.m_depth.m_writeEnabled,
				  .DepthWriteMask = state.m_state.m_depth.m_writeEnabled ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO,
				  .DepthFunc = convertCompareOperation(state.m_state.m_depth.m_compare),
				  .StencilEnable = stencilEnabled,
				  .FrontFace = stencilDescs[0],
				  .BackFace = stencilDescs[1],
				  .DepthBoundsTestEnable = false};
	}

	// Rast state
	const D3D12_RASTERIZER_DESC2 rastDesc = {.FillMode = convertFillMode(state.m_state.m_rast.m_fillMode),
											 .CullMode = convertCullMode(state.m_state.m_rast.m_cullMode),
											 .FrontCounterClockwise = true,
											 .DepthBias = state.m_state.m_rast.m_depthBias,
											 .DepthBiasClamp = state.m_state.m_rast.m_depthBiasClamp,
											 .SlopeScaledDepthBias = state.m_state.m_rast.m_slopeScaledDepthBias};

	// Misc
	D3D12_RT_FORMAT_ARRAY rtFormats = {};
	for(U32 i = 0; i < kMaxColorRenderTargets; ++i)
	{
		if(state.m_state.m_misc.m_colorRtMask.get(i))
		{
			rtFormats.RTFormats[i] = DXGI_FORMAT(state.m_state.m_misc.m_colorRtFormats[i]);
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
	desc.pRootSignature = &state.m_state.m_shaderProg->m_rootSignature->getD3DRootSignature();
	desc.InputLayout = D3D12_INPUT_LAYOUT_DESC{.pInputElementDescs = inputElementDescs.getBegin(), .NumElements = inputElementDescCount};
	desc.PrimitiveTopologyType = convertPrimitiveTopology(state.m_state.m_misc.m_topology);
	ANKI_SET_IR(VS, kVertex)
	ANKI_SET_IR(GS, kGeometry)
	ANKI_SET_IR(HS, kTessellationControl)
	ANKI_SET_IR(DS, kTessellationEvaluation)
	ANKI_SET_IR(PS, kFragment)
	ANKI_SET_IR(AS, kTask)
	ANKI_SET_IR(MS, kMesh)
	desc.BlendState = CD3DX12_BLEND_DESC(blendDesc);
	desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC2(dsDesc);
	desc.DSVFormat = DXGI_FORMAT(state.m_state.m_misc.m_depthStencilFormat);
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
