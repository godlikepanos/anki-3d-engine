// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/gr/Common.h"
#include "anki/gr/ShaderPtr.h"
#include "anki/gr/PipelinePtr.h"

namespace anki {

/// @addtogroup graphics
/// @{

class VertexBinding
{
public:
	PtrSize m_stride; ///< Vertex stride.
	VertexStepRate m_stepRate = VertexStepRate::VERTEX;
};

class VertexAttributeBinding
{
public:
	PixelFormat m_format;
	PtrSize m_offset = 0;
	U8 m_binding = 0;
};

class VertexStateInfo
{
public:
	U8 m_bindingCount = 0;
	Array<VertexBinding, MAX_VERTEX_ATTRIBUTES> m_bindings;
	U8 m_attributeCount = 0;
	Array<VertexAttributeBinding, MAX_VERTEX_ATTRIBUTES> m_attributes;
};

class InputAssemblerStateInfo
{
public:
	PrimitiveTopology m_topology = PrimitiveTopology::TRIANGLES;
	Bool8 m_primitiveRestartEnabled = false;
};

class TessellationStateInfo
{
public:
	U32 m_patchControlPointsCount = 3;
};

class ViewportStateInfo
{
public:
	Bool8 m_scissorEnabled = false;
};

class RasterizerStateInfo
{
public:
	FillMode m_fillMode = FillMode::SOLID;
	CullMode m_cullMode = CullMode::BACK;
};

class DepthStencilStateInfo
{
public:
	Bool8 m_depthWriteEnabled = true;
	CompareOperation m_depthCompareFunction = CompareOperation::LESS;
	PixelFormat m_format;
	F32 m_polygonOffsetFactor = 0.0;
	F32 m_polygonOffsetUnits = 0.0;
};

class ColorAttachmentStateInfo
{
public:
	PixelFormat m_format;
	BlendMethod m_srcBlendMethod = BlendMethod::ONE;
	BlendMethod m_dstBlendMethod = BlendMethod::ZERO;
	BlendFunction m_blendFunction = BlendFunction::ADD;
	ColorBit m_channelWriteMask = ColorBit::ALL;
};

class ColorStateInfo
{
public:
	Bool8 m_alphaToCoverageEnabled = false;
	U8 m_attachmentCount = 0;
	Array<ColorAttachmentStateInfo, MAX_COLOR_ATTACHMENTS> m_attachments;
};

enum class PipelineSubStateBit: U16
{
	NONE = 0,
	VERTEX = 1 << 0,
	INPUT_ASSEMBLER = 1 << 1,
	TESSELLATION = 1 << 2,
	VIEWPORT = 1 << 3,
	RASTERIZER = 1 << 4,
	DEPTH_STENCIL = 1 << 5,
	COLOR = 1 << 6,
	ALL = VERTEX | INPUT_ASSEMBLER | TESSELLATION | VIEWPORT | RASTERIZER
		| DEPTH_STENCIL | COLOR
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(PipelineSubStateBit, inline)

/// Pipeline initializer.
class PipelineInitializer
{
public:
	VertexStateInfo m_vertex;
	InputAssemblerStateInfo m_inputAssembler;
	TessellationStateInfo m_tessellation;
	ViewportStateInfo m_viewport;
	RasterizerStateInfo m_rasterizer;
	DepthStencilStateInfo m_depthStencil;
	ColorStateInfo m_color;

	Array<ShaderPtr, 6> m_shaders;
};
/// @}

} // end namespace anki

