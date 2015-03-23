// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_PIPELINE_COMMON_H
#define ANKI_GR_PIPELINE_COMMON_H

#include "anki/gr/Common.h"
#include "anki/gr/ShaderHandle.h"

namespace anki {

/// @addtogroup graphics
/// @{

struct VertexBinding
{
	PtrSize m_stride; ///< Vertex stride.
	VertexStepRate m_stepRate = VertexStepRate::VERTEX;
};

struct VertexAttributeBinding
{
	PixelFormat m_format;
	PtrSize m_offset = 0;
	U8 m_binding = 0;
};

struct VertexStateInfo
{
	U8 m_bindingCount = 0;
	Array<VertexBinding, MAX_VERTEX_ATTRIBUTES> m_bindings;
	U8 m_attributeCount = 0;
	Array<VertexAttributeBinding, MAX_VERTEX_ATTRIBUTES> m_attributes;
};

struct InputAssemblerStateInfo
{
	PrimitiveTopology m_topology = PrimitiveTopology::TRIANGLES;
	Bool8 m_primitiveRestartEnabled = false;
};

struct TessellationStateInfo
{
	U32 m_patchControlPointsCount = 3;
};

struct ViewportStateInfo
{
	Bool8 m_scissorEnabled = false;
	Array<U32, 4> m_scissorRect = {{0, 0, 0, 0}};
	Array<U32, 4> m_viewport = {{0, 0, 0, 0}};
};

struct RasterizerStateInfo
{
	FillMode m_fillCode = FillMode::SOLID;
	CullMode m_cullMode = CullMode::BACK;
};

struct DepthStateInfo
{
	Bool8 m_writeEnabled = false;
	DepthCompareFunction m_compareFunction;
};

struct ColorStateInfo
{
	Bool8 m_alphaToCoverageEnabled = false;
	Bool8 m_blendEnabled = false;
	BlendMethod m_srcBlendMethod = BlendMethod::ONE;
	BlendMethod m_dstBlendMethod = BlendMethod::ONE;
	BlendFunction m_blendFunction = BlendFunction::ADD;
	U8 m_channelWriteMask = 15;
};

enum class SubStateBit: U16
{
	NONE = 0,
	VERTEX = 1 << 0,
	INPUT_ASSEMBLER = 1 << 1,
	TESSELLATION = 1 << 2,
	VIEWPORT = 1 << 3,
	RASTERIZER = 1 << 4,
	DEPTH = 1 << 5,
	COLOR = 1 << 6
};

/// Pipeline initializer.
struct PipelineInitializer
{
	VertexStateInfo m_vertex;
	InputAssemblerStateInfo m_inputAssembler;
	TessellationStateInfo m_tessellation;
	ViewportStateInfo m_viewport;
	RasterizerStateInfo m_rasterizer;
	DepthStateInfo m_depthStencil;
	ColorStateInfo m_color;

	Array<ShaderHandle, 6> m_shaders;
	SubStateBit m_definedState = SubStateBit::NONE;
};
/// @}

} // end namespace anki

#endif
