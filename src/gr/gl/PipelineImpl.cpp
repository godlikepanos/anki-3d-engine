// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/gl/PipelineImpl.h"
#include "anki/gr/gl/ShaderImpl.h"
#include "anki/gr/gl/GrManagerImpl.h"
#include "anki/gr/gl/RenderingThread.h"
#include "anki/util/Logger.h"

namespace anki {

//==============================================================================
static GLenum computeGlShaderType(const ShaderType idx, GLbitfield* bit)
{
	static const Array<GLenum, 6> gltype = {{GL_VERTEX_SHADER, 
		GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER,
		GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER}};

	static const Array<GLuint, 6> glbit = {{
		GL_VERTEX_SHADER_BIT, GL_TESS_CONTROL_SHADER_BIT, 
		GL_TESS_EVALUATION_SHADER_BIT, GL_GEOMETRY_SHADER_BIT,
		GL_FRAGMENT_SHADER_BIT, GL_COMPUTE_SHADER_BIT}};

	if(bit)
	{
		*bit = glbit[enumToType(idx)];
	}

	return gltype[enumToType(idx)];
}

//==============================================================================
static GLenum convertBlendMethod(BlendMethod in)
{
	GLenum out = 0;

	switch(in)
	{
	case BlendMethod::ZERO:
		out = GL_ZERO;
		break;
	case BlendMethod::ONE:
		out = GL_ONE;
		break;
	case BlendMethod::SRC_COLOR:
		out = GL_SRC_COLOR;
		break;
	case BlendMethod::ONE_MINUS_SRC_COLOR:
		out = GL_ONE_MINUS_SRC_COLOR;
		break;
	case BlendMethod::DST_COLOR:
		out = GL_DST_COLOR;
		break;
	case BlendMethod::ONE_MINUS_DST_COLOR:
		out = GL_ONE_MINUS_DST_COLOR;
		break;
	case BlendMethod::SRC_ALPHA:
		out = GL_SRC_ALPHA;
		break;
	case BlendMethod::ONE_MINUS_SRC_ALPHA:
		out = GL_ONE_MINUS_SRC_ALPHA;
		break;
	case BlendMethod::DST_ALPHA:
		out = GL_DST_ALPHA;
		break;
	case BlendMethod::ONE_MINUS_DST_ALPHA:
		out = GL_ONE_MINUS_DST_ALPHA;
		break;
	case BlendMethod::CONSTANT_COLOR:
		out = GL_CONSTANT_COLOR;
		break;
	case BlendMethod::ONE_MINUS_CONSTANT_COLOR:
		out = GL_ONE_MINUS_CONSTANT_COLOR;
		break;
	case BlendMethod::CONSTANT_ALPHA:
		out = GL_CONSTANT_ALPHA;
		break;
	case BlendMethod::ONE_MINUS_CONSTANT_ALPHA:
		out = GL_ONE_MINUS_CONSTANT_ALPHA;
		break;
	case BlendMethod::SRC_ALPHA_SATURATE:
		out = GL_SRC_ALPHA_SATURATE;
		break;
	case BlendMethod::SRC1_COLOR:
		out = GL_SRC1_COLOR;
		break;
	case BlendMethod::ONE_MINUS_SRC1_COLOR:
		out = GL_ONE_MINUS_SRC1_COLOR;
		break;
	case BlendMethod::SRC1_ALPHA:
		out = GL_SRC1_ALPHA;
		break;
	case BlendMethod::ONE_MINUS_SRC1_ALPHA:
		out = GL_ONE_MINUS_SRC1_ALPHA;
		break;
	default:
		ANKI_ASSERT(0);	
	}

	return out;
}

//==============================================================================
Error PipelineImpl::create(const Initializer& init)
{
	static_cast<Initializer&>(*this) = init;

	// Check if complete
	m_complete = true;

	if(m_templatePipeline.isCreated())
	{
		// If there is a template it should always be complete
		SubStateBit mask = 
			m_definedState | m_templatePipeline.get().m_definedState;
		ANKI_ASSERT(mask == SubStateBit::ALL);
	}
	else if(m_definedState != SubStateBit::ALL)
	{
		m_complete = false;
	}

	ANKI_CHECK(createGlPipeline());
	initVertexState();
	initInputAssemblerState();
	initRasterizerState();
	initDepthStencilState();
	initColorState();

	return ErrorCode::NONE;
}

//==============================================================================
void PipelineImpl::destroy()
{
	if(m_glName)
	{
		glDeleteProgramPipelines(1, &m_glName);
		m_glName = 0;
	}
}

//==============================================================================
Error PipelineImpl::createGlPipeline()
{
	Error err = ErrorCode::NONE;

	// Do checks
	U mask = 0;
	U count = 6;
	while(count-- != 0)
	{
		const ShaderHandle& shader = m_shaders[count];
		if(shader.isCreated())
		{
			ANKI_ASSERT(count == enumToType(shader.get().getType()));
			mask |= 1 << count;
		}
	}

	if(mask & (1 << 5))
	{
		// Is compute

		ANKI_ASSERT((mask & (1 << 5)) == (1 << 5) 
			&& "Compute should be alone in the pipeline");
	}
	else
	{
		const U fragVert = (1 << 0) | (1 << 4);
		ANKI_ASSERT((mask & fragVert) && "Should contain vert and frag");
		(void)fragVert;

		const U tess = (1 << 1) | (1 << 2);
		if((mask & tess) != 0)
		{
			ANKI_ASSERT(((mask & tess) == 0x6)
				&& "Should set both the tessellation shaders");
		}
	}

	// Create and attach programs
	glGenProgramPipelines(1, &m_glName);
	ANKI_ASSERT(m_glName != 0);

	glBindProgramPipeline(m_glName);

	for(U i = 0; i < m_shaders.getSize(); i++)
	{
		ShaderHandle& shader = m_shaders[i];

		if(shader.isCreated())
		{
			GLbitfield bit;
			computeGlShaderType(static_cast<ShaderType>(i), &bit);
			glUseProgramStages(m_glName, bit, shader.get().getGlName());
		}
	}

	// Validate and check error
	glValidateProgramPipeline(m_glName);
	GLint status = 0;
	glGetProgramPipelineiv(m_glName, GL_VALIDATE_STATUS, &status);

	if(!status)
	{
		GLint infoLen = 0;
		GLint charsWritten = 0;
		DArray<char> infoLogTxt;

		glGetProgramPipelineiv(m_glName, GL_INFO_LOG_LENGTH, &infoLen);

		auto alloc = getAllocator();
		infoLogTxt.create(alloc, infoLen + 1);

		glGetProgramPipelineInfoLog(
			m_glName, infoLen, &charsWritten, &infoLogTxt[0]);

		ANKI_LOGE("Ppline error log follows:\n%s", &infoLogTxt[0]);
		err = ErrorCode::USER_DATA;
	}

	glBindProgramPipeline(0);

	return err;
}

//==============================================================================
void PipelineImpl::bind()
{
#if 1
	ANKI_ASSERT(isCreated());
	glBindProgramPipeline(m_glName);
#else
	ANKI_ASSERT(m_complete && "Should be complete");

	// Get last pipeline
	auto& state = 
		getManager().getImplementation().getRenderingThread().getState();

	const PipelineImpl* lastPpline = nullptr;
	const PipelineImpl* lastPplineTempl = nullptr;
	if(state.m_lastPipeline.isCreated())
	{
		lastPpline = &state.m_lastPipeline.get();

		if(ANKI_UNLIKELY(lastPpline == this))
		{
			// Binding the same pipeline, early out
			return;
		}

		if(lastPpline->m_templatePipeline.isCreated())
		{
			lastPplineTempl = &lastPpline->m_templatePipeline.get();
		}
	}

	// Get crnt pipeline template
	const PipelineImpl* pplineTempl = nullptr;
	if(m_templatePipeline.isCreated())
	{
		pplineTempl = &m_templatePipeline.get();
	}
	
#define ANKI_PPLINE_BIND(enum_, method_) \
	do { \
		const PipelineImpl* ppline = getPipelineForState(SubStateBit::enum_, \
			lastPpline, lastPplineTempl, pplineTempl); \
		if(ppline) { \
			ppline->method_(state); \
		} \
	} while(0) \

	ANKI_PPLINE_BIND(VERTEX, setVertexState);
	ANKI_PPLINE_BIND(INPUT_ASSEMBLER, setInputAssemblerState);
	ANKI_PPLINE_BIND(TESSELLATION, setTessellationState);
	ANKI_PPLINE_BIND(VIEWPORT, setViewportState);
	ANKI_PPLINE_BIND(RASTERIZER, setRasterizerState);
	ANKI_PPLINE_BIND(DEPTH_STENCIL, setDepthStencilState);
	ANKI_PPLINE_BIND(COLOR, setColorState);

#undef ANKI_PPLINE_BIND

#endif
}

//==============================================================================
void PipelineImpl::initVertexState()
{
	for(U i = 0; i < m_vertex.m_attributeCount; ++i)
	{
		const VertexAttributeBinding& binding = m_vertex.m_attributes[i];
		ANKI_ASSERT(binding.m_format.m_srgb == false);
		Attribute& cache = m_attribs[i];

		// Component count
		if(binding.m_format == PixelFormat(
			ComponentFormat::R32G32, TransformFormat::FLOAT))
		{
			cache.m_compCount = 2;
			cache.m_type = GL_FLOAT;
			cache.m_normalized = false;
		}
		else if(binding.m_format == PixelFormat(
			ComponentFormat::R32G32B32, TransformFormat::FLOAT))
		{
			cache.m_compCount = 3;
			cache.m_type = GL_FLOAT;
			cache.m_normalized = false;
		}
		else if(binding.m_format == PixelFormat(
			ComponentFormat::R10G10B10A2, TransformFormat::SNORM))
		{
			cache.m_compCount = 4;
			cache.m_type = GL_INT_2_10_10_10_REV;
			cache.m_normalized = true;
		}
		else
		{
			ANKI_ASSERT(0 && "TODO");
		}
	}
}

//==============================================================================
void PipelineImpl::initInputAssemblerState()
{
	switch(m_inputAssembler.m_topology)
	{
	case POINTS:
		m_topology = GL_POINTS;
		break;
	case LINES:
		m_topology = GL_LINES;
		break;
	case LINE_STIP:
		m_topology = GL_LINE_STRIP;
		break;
	case TRIANGLES:
		m_topology = GL_TRIANGLES;
		break;
	case TRIANGLE_STRIP:
		m_topology = GL_TRIANGLE_STRIP;
		break;
	case PATCHES:
		m_topology = GL_PATCHES;
		break;
	default:
		ANKI_ASSERT(0);
	}
}

//==============================================================================
void PipelineImpl::initRasterizerState()
{
	switch(m_rasterizer.m_fillMode)
	{
	case FillMode::POINTS:
		m_fillMode = GL_POINT;
		break;
	case FillMode::WIREFRAME:
		m_fillMode = GL_LINE;
		break;
	case FillMode::SOLID:
		m_fillMode = GL_FILL;
		break;
	default:
		ANKI_ASSERT(0);
	}

	switch(m_rasterizer.m_cullMode)
	{
	case CullMode::FRONT:
		m_cullMode = GL_FRONT;
		break;
	case CullMode::BACK:
		m_cullMode = GL_BACK;
		break;
	case CullMode::FRONT_AND_BACK:
		m_cullMode = GL_FRONT_AND_BACK;
		break;
	default:
		ANKI_ASSERT(0);
	}
}

//==============================================================================
void PipelineImpl::initDepthStencilState()
{
	m_depthCompareFunction = 
		convertCompareOperation(m_depthStencil.m_depthCompareFunction);
}

//==============================================================================
void PipelineImpl::initColorState()
{
	for(U i = 0; i < m_color.m_colorAttachmentsCount; ++i)
	{
		Attachment& out = m_attachments[i];
		const ColorAttachmentStateInfo& in = m_color.m_attachments[i];

		out.m_srcBlendMethod = convertBlendMethod(in.m_srcBlendMethod);
		out.m_dstBlendMethod = convertBlendMethod(in.m_dstBlendMethod);

		switch(in.m_blendFunction)
		{
		case BlendFunction::ADD:
			out.m_blendFunction = GL_FUNC_ADD;
			break;
		case BlendFunction::SUBTRACT:
			out.m_blendFunction = GL_FUNC_SUBTRACT;
			break;
		case BlendFunction::REVERSE_SUBTRACT:
			out.m_blendFunction = GL_FUNC_REVERSE_SUBTRACT;
			break;
		case BlendFunction::MIN:
			out.m_blendFunction = GL_MIN;
			break;
		case BlendFunction::MAX:
			out.m_blendFunction = GL_MAX;
			break;
		default:
			ANKI_ASSERT(0);
		}

		out.m_channelWriteMask[0] = in.m_channelWriteMask | ColorBit::RED;
		out.m_channelWriteMask[1] = in.m_channelWriteMask | ColorBit::GREEN;
		out.m_channelWriteMask[2] = in.m_channelWriteMask | ColorBit::BLUE;
		out.m_channelWriteMask[3] = in.m_channelWriteMask | ColorBit::ALPHA;
	}
}

//==============================================================================
void PipelineImpl::setVertexState(GlState&) const
{
	for(U i = 0; i < m_vertex.m_attributeCount; ++i)
	{
		const Attribute& attrib = m_attribs[i];
		ANKI_ASSERT(attrib.m_type);
		glVertexAttribFormat(i, attrib.m_compCount, attrib.m_type, 
			attrib.m_normalized, m_vertex.m_attributes[i].m_offset);

		glVertexAttribBinding(i, m_vertex.m_attributes[i].m_binding);
	}
}

//==============================================================================
void PipelineImpl::setInputAssemblerState(GlState& state) const
{
	if(m_inputAssembler.m_primitiveRestartEnabled 
		!= state.m_primitiveRestartEnabled)
	{
		if(m_inputAssembler.m_primitiveRestartEnabled)
		{
			glEnable(GL_PRIMITIVE_RESTART);
		}
		else
		{
			glDisable(GL_PRIMITIVE_RESTART);
		}

		state.m_primitiveRestartEnabled = 
			m_inputAssembler.m_primitiveRestartEnabled;
	}

	state.m_topology = m_topology;
}

//==============================================================================
void PipelineImpl::setTessellationState(GlState& state) const
{
	if(m_tessellation.m_patchControlPointsCount 
		!= state.m_patchControlPointsCount)
	{
		glPatchParameteri(GL_PATCH_VERTICES, 
			m_tessellation.m_patchControlPointsCount);

		state.m_patchControlPointsCount = 
			m_tessellation.m_patchControlPointsCount;
	}
}

//==============================================================================
void PipelineImpl::setRasterizerState(GlState& state) const
{
	if(m_fillMode != state.m_fillMode)
	{
		glPolygonMode(GL_FRONT_AND_BACK, m_fillMode);
		state.m_fillMode = m_fillMode;
	}

	if(m_cullMode != state.m_cullMode)
	{
		glCullFace(m_cullMode);
		state.m_cullMode = m_cullMode;
	}
}

//==============================================================================
void PipelineImpl::setDepthStencilState(GlState& state) const
{
	if(m_depthCompareFunction != state.m_depthCompareFunction)
	{
		if(m_depthCompareFunction == GL_ALWAYS)
		{
			glDisable(GL_DEPTH_TEST);
		}
		else
		{
			glEnable(GL_DEPTH_TEST);
		}

		glDepthFunc(m_depthCompareFunction);

		state.m_depthCompareFunction = m_depthCompareFunction;
	}
}

//==============================================================================
void PipelineImpl::setColorState(GlState&) const
{
	for(U i = 0; i < m_color.m_colorAttachmentsCount; ++i)
	{
		const Attachment& att = m_attachments[i];

		glBlendFunci(i, att.m_srcBlendMethod, att.m_dstBlendMethod);
		glBlendEquationi(i, att.m_blendFunction);
		glColorMaski(i, att.m_channelWriteMask[0], att.m_channelWriteMask[1], 
			att.m_channelWriteMask[2], att.m_channelWriteMask[3]);
	}
}

//==============================================================================
const PipelineImpl* PipelineImpl::getPipelineForState(
	const SubStateBit bit, 
	const PipelineImpl* lastPpline,
	const PipelineImpl* lastPplineTempl,
	const PipelineImpl* pplineTempl) const 
{
	const PipelineImpl* out = nullptr;

	if(pplineTempl == nullptr || (m_definedState | bit) != SubStateBit::NONE)
	{
		// Current pipeline overrides the state
		out = this;
	}
	else
	{
		// Need to get the state from the templates

		if(lastPplineTempl == nullptr 
			|| lastPplineTempl != pplineTempl
			|| (lastPpline->m_definedState | bit) != SubStateBit::NONE)
		{
			// Last template cannot be used

			out = pplineTempl;
		}
		else
		{
			// Last template can be used but since it's already bound skipp
			out = nullptr;
		}
	}

	return out;
}

} // end namespace anki

