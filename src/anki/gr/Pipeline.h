// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>
#include <anki/gr/Shader.h>
#include <anki/util/Hash.h>

namespace anki
{

/// @addtogroup graphics
/// @{

class VertexBinding
{
public:
	PtrSize m_stride; ///< Vertex stride.
	VertexStepRate m_stepRate = VertexStepRate::VERTEX;

	Bool operator==(const VertexBinding& b) const
	{
		return m_stride == b.m_stride && m_stepRate == b.m_stepRate;
	}

	Bool operator!=(const VertexBinding& b) const
	{
		return !(*this == b);
	}
};

class VertexAttributeBinding
{
public:
	PixelFormat m_format;
	PtrSize m_offset = 0;
	U8 m_binding = 0;

	Bool operator==(const VertexAttributeBinding& b) const
	{
		return m_format == b.m_format && m_offset == b.m_offset && m_binding == b.m_binding;
	}

	Bool operator!=(const VertexAttributeBinding& b) const
	{
		return !(*this == b);
	}
};

class VertexStateInfo
{
public:
	U8 m_bindingCount = 0;
	Array<VertexBinding, MAX_VERTEX_ATTRIBUTES> m_bindings;
	U8 m_attributeCount = 0;
	Array<VertexAttributeBinding, MAX_VERTEX_ATTRIBUTES> m_attributes;

	Bool operator==(const VertexStateInfo& b) const
	{
		if(m_bindingCount != b.m_bindingCount || m_attributeCount != b.m_attributeCount)
		{
			return false;
		}

		for(U i = 0; i < m_bindingCount; ++i)
		{
			if(m_bindings[i] != b.m_bindings[i])
			{
				return false;
			}
		}

		for(U i = 0; i < m_attributeCount; ++i)
		{
			if(m_attributes[i] != b.m_attributes[i])
			{
				return false;
			}
		}

		return true;
	}

	Bool operator!=(const VertexStateInfo& b) const
	{
		return !(*this == b);
	}
};

class InputAssemblerStateInfo
{
public:
	PrimitiveTopology m_topology = PrimitiveTopology::TRIANGLES;
	Bool8 m_primitiveRestartEnabled = false;

	Bool operator==(const InputAssemblerStateInfo& b) const
	{
		return m_topology == b.m_topology && m_primitiveRestartEnabled == b.m_primitiveRestartEnabled;
	}

	Bool operator!=(const InputAssemblerStateInfo& b) const
	{
		return !(*this == b);
	}
};

class TessellationStateInfo
{
public:
	U32 m_patchControlPointCount = 3;
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
	FaceSelectionMask m_cullMode = FaceSelectionMask::BACK;
};

class StencilStateInfo
{
public:
	StencilOperation m_stencilFailOperation = StencilOperation::KEEP;
	StencilOperation m_stencilPassDepthFailOperation = StencilOperation::KEEP;
	StencilOperation m_stencilPassDepthPassOperation = StencilOperation::KEEP;
	CompareOperation m_compareFunction = CompareOperation::ALWAYS;
};

/// Depth stencil state.
/// To disable depth test set m_depthWriteEnabled to false and m_depthCompareFunction to always.
/// To disable stencil test leave m_stencilFront and m_stencilBack as is.
class DepthStencilStateInfo
{
public:
	// Depth
	Bool8 m_depthWriteEnabled = true;
	CompareOperation m_depthCompareFunction = CompareOperation::LESS;

	// Stencil
	StencilStateInfo m_stencilFront;
	StencilStateInfo m_stencilBack;

	// Common
	PixelFormat m_format;

	Bool isInUse() const
	{
		return m_format.m_components != ComponentFormat::NONE;
	}
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
	SHADERS = 1 << 7,
	ALL = VERTEX | INPUT_ASSEMBLER | TESSELLATION | VIEWPORT | RASTERIZER | DEPTH_STENCIL | COLOR | SHADERS
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(PipelineSubStateBit, inline)

/// Only the state part of PipelineInitInfo. It's separate for easy hashing.
class PipelineInitInfoState
{
public:
	PipelineInitInfoState()
	{
		// Do a special construction. The state will be hashed and the padding may contain garbage. With this trick
		// zero the padding
		memset(this, 0, sizeof(*this));

#define ANKI_CONSTRUCT_AND_ZERO_PADDING(memb_) new(&memb_) decltype(memb_)()

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
};

/// Pipeline initializer.
class PipelineInitInfo : public PipelineInitInfoState
{
public:
	Array<ShaderPtr, U(ShaderType::COUNT)> m_shaders;

	U64 computeHash() const
	{
		U64 h = anki::computeHash(static_cast<const PipelineInitInfoState*>(this), sizeof(PipelineInitInfoState));

		Array<U64, U(ShaderType::COUNT)> uuids;
		for(U i = 0; i < m_shaders.getSize(); ++i)
		{
			uuids[i] = (m_shaders[i].isCreated()) ? m_shaders[i]->getUuid() : 0;
		}

		return appendHash(&uuids[0], sizeof(uuids), h);
	}
};

/// Graphics and compute pipeline. Contains the static state.
class Pipeline : public GrObject
{
	ANKI_GR_OBJECT

anki_internal:
	UniquePtr<PipelineImpl> m_impl;

	static const GrObjectType CLASS_TYPE = GrObjectType::PIPELINE;

	/// Construct.
	Pipeline(GrManager* manager, U64 hash, GrObjectCache* cache);

	/// Destroy.
	~Pipeline();

	/// Create.
	void init(const PipelineInitInfo& init);
};
/// @}

} // end namespace anki
