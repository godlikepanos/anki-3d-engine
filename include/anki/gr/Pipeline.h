// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>
#include <anki/gr/Shader.h>

namespace anki
{

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

enum class PipelineSubStateBit : U16
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
		| DEPTH_STENCIL
		| COLOR
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(PipelineSubStateBit, inline)

/// Pipeline initializer.
class PipelineInitInfo
{
public:
	PipelineInitInfo()
	{
// Do a special construction. The state will be hashed and the padding
// may contain garbage. With this trick zero the padding
#define ANKI_CONSTRUCT_AND_ZERO_PADDING(member_)                               \
	memset(&member_, 0, sizeof(member_));                                      \
	new(&member_) decltype(member_)()

		ANKI_CONSTRUCT_AND_ZERO_PADDING(m_vertex);
		ANKI_CONSTRUCT_AND_ZERO_PADDING(m_inputAssembler);
		ANKI_CONSTRUCT_AND_ZERO_PADDING(m_tessellation);
		ANKI_CONSTRUCT_AND_ZERO_PADDING(m_viewport);
		ANKI_CONSTRUCT_AND_ZERO_PADDING(m_rasterizer);
		ANKI_CONSTRUCT_AND_ZERO_PADDING(m_depthStencil);
		ANKI_CONSTRUCT_AND_ZERO_PADDING(m_color);

#undef ANKI_CONSTRUCT_AND_ZERO_PADDING
	}

	VertexStateInfo m_vertex;
	InputAssemblerStateInfo m_inputAssembler;
	TessellationStateInfo m_tessellation;
	ViewportStateInfo m_viewport;
	RasterizerStateInfo m_rasterizer;
	DepthStencilStateInfo m_depthStencil;
	ColorStateInfo m_color;

	Array<ShaderPtr, 6> m_shaders;
};

/// Graphics and compute pipeline. Contains the static state.
class Pipeline : public GrObject
{
public:
	/// Construct.
	Pipeline(GrManager* manager);

	/// Destroy.
	~Pipeline();

	/// Access the implementation.
	PipelineImpl& getImplementation()
	{
		return *m_impl;
	}

	/// Create.
	void init(const PipelineInitInfo& init);

private:
	UniquePtr<PipelineImpl> m_impl;
};
/// @}

} // end namespace anki
