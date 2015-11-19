// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/PipelineImpl.h>
#include <anki/gr/gl/ShaderImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/gl/GrManagerImpl.h>
#include <anki/gr/gl/RenderingThread.h>
#include <anki/gr/gl/CommandBufferImpl.h>
#include <anki/util/Logger.h>
#include <anki/util/Hash.h>

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
Error PipelineImpl::create(const PipelineInitializer& init)
{
	m_in = init;

	ANKI_CHECK(createGlPipeline());
	if(!m_compute)
	{
		initVertexState();
		initInputAssemblerState();
		initTessellationState();
		initRasterizerState();
		initDepthStencilState();
		initColorState();
	}

	return ErrorCode::NONE;
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
		const ShaderPtr& shader = m_in.m_shaders[count];
		if(shader.isCreated())
		{
			ANKI_ASSERT(
				count == enumToType(shader->getImplementation().m_type));
			mask |= 1 << count;
		}
	}

	if(mask & (1 << 5))
	{
		// Is compute

		m_compute = true;

		ANKI_ASSERT((mask & (1 << 5)) == (1 << 5)
			&& "Compute should be alone in the pipeline");
	}
	else
	{
		m_compute = false;

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

	for(U i = 0; i < m_in.m_shaders.getSize(); i++)
	{
		ShaderPtr& shader = m_in.m_shaders[i];

		if(shader.isCreated())
		{
			GLbitfield bit;
			computeGlShaderType(static_cast<ShaderType>(i), &bit);
			glUseProgramStages(
				m_glName, bit, shader->getImplementation().getGlName());

			if(i == U(ShaderType::TESSELLATION_CONTROL)
				|| i == U(ShaderType::TESSELLATION_EVALUATION))
			{
				m_tessellation = true;
			}
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
		DArrayAuto<char> infoLogTxt(getAllocator());

		glGetProgramPipelineiv(m_glName, GL_INFO_LOG_LENGTH, &infoLen);

		infoLogTxt.create(infoLen + 1);

		glGetProgramPipelineInfoLog(
			m_glName, infoLen, &charsWritten, &infoLogTxt[0]);

		ANKI_LOGE("Ppline error log follows:\n%s", &infoLogTxt[0]);
		err = ErrorCode::USER_DATA;
	}

	glBindProgramPipeline(0);

	return err;
}

//==============================================================================
void PipelineImpl::bind(GlState& state)
{
	glBindProgramPipeline(m_glName);

	if(m_compute)
	{
		return;
	}

	// Set state
	setVertexState(state);
	setInputAssemblerState(state);
	setTessellationState(state);
	setViewportState(state);
	setRasterizerState(state);
	setDepthStencilState(state);
	setColorState(state);
}

//==============================================================================
void PipelineImpl::initVertexState()
{
	for(U i = 0; i < m_in.m_vertex.m_attributeCount; ++i)
	{
		const VertexAttributeBinding& binding = m_in.m_vertex.m_attributes[i];
		ANKI_ASSERT(binding.m_format.m_srgb == false);
		Attribute& cache = m_cache.m_attribs[i];

		// Component count
		if(binding.m_format == PixelFormat(
			ComponentFormat::R32, TransformFormat::FLOAT))
		{
			cache.m_compCount = 1;
			cache.m_type = GL_FLOAT;
			cache.m_normalized = false;
		}
		else if(binding.m_format == PixelFormat(
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
			ComponentFormat::R32G32B32A32, TransformFormat::FLOAT))
		{
			cache.m_compCount = 4;
			cache.m_type = GL_FLOAT;
			cache.m_normalized = false;
		}
		else if(binding.m_format == PixelFormat(
			ComponentFormat::R16G16, TransformFormat::FLOAT))
		{
			cache.m_compCount = 2;
			cache.m_type = GL_HALF_FLOAT;
			cache.m_normalized = false;
		}
		else if(binding.m_format == PixelFormat(
			ComponentFormat::R16G16, TransformFormat::UNORM))
		{
			cache.m_compCount = 2;
			cache.m_type = GL_UNSIGNED_SHORT;
			cache.m_normalized = true;
		}
		else if(binding.m_format == PixelFormat(
			ComponentFormat::R10G10B10A2, TransformFormat::SNORM))
		{
			cache.m_compCount = 4;
			cache.m_type = GL_INT_2_10_10_10_REV;
			cache.m_normalized = true;
		}
		else if(binding.m_format == PixelFormat(
			ComponentFormat::R8G8B8A8, TransformFormat::UNORM))
		{
			cache.m_compCount = 4;
			cache.m_type = GL_UNSIGNED_BYTE;
			cache.m_normalized = true;
		}
		else
		{
			ANKI_ASSERT(0 && "TODO");
		}
	}

	m_hashes.m_vertex = computeHash(&m_in.m_vertex, sizeof(m_in.m_vertex));
}

//==============================================================================
void PipelineImpl::initInputAssemblerState()
{
	switch(m_in.m_inputAssembler.m_topology)
	{
	case PrimitiveTopology::POINTS:
		m_cache.m_topology = GL_POINTS;
		break;
	case PrimitiveTopology::LINES:
		m_cache.m_topology = GL_LINES;
		break;
	case PrimitiveTopology::LINE_STIP:
		m_cache.m_topology = GL_LINE_STRIP;
		break;
	case PrimitiveTopology::TRIANGLES:
		m_cache.m_topology = GL_TRIANGLES;
		break;
	case PrimitiveTopology::TRIANGLE_STRIP:
		m_cache.m_topology = GL_TRIANGLE_STRIP;
		break;
	case PrimitiveTopology::PATCHES:
		m_cache.m_topology = GL_PATCHES;
		break;
	default:
		ANKI_ASSERT(0);
	}

	m_hashes.m_inputAssembler =
		computeHash(&m_in.m_inputAssembler, sizeof(m_in.m_inputAssembler));
}

//==============================================================================
void PipelineImpl::initTessellationState()
{
	m_hashes.m_tessellation =
		computeHash(&m_in.m_tessellation, sizeof(m_in.m_tessellation));
}

//==============================================================================
void PipelineImpl::initRasterizerState()
{
	switch(m_in.m_rasterizer.m_fillMode)
	{
	case FillMode::POINTS:
		m_cache.m_fillMode = GL_POINT;
		break;
	case FillMode::WIREFRAME:
		m_cache.m_fillMode = GL_LINE;
		break;
	case FillMode::SOLID:
		m_cache.m_fillMode = GL_FILL;
		break;
	default:
		ANKI_ASSERT(0);
	}

	switch(m_in.m_rasterizer.m_cullMode)
	{
	case CullMode::FRONT:
		m_cache.m_cullMode = GL_FRONT;
		break;
	case CullMode::BACK:
		m_cache.m_cullMode = GL_BACK;
		break;
	case CullMode::FRONT_AND_BACK:
		m_cache.m_cullMode = GL_FRONT_AND_BACK;
		break;
	default:
		ANKI_ASSERT(0);
	}

	m_hashes.m_rasterizer =
		computeHash(&m_in.m_rasterizer, sizeof(m_in.m_rasterizer));
}

//==============================================================================
void PipelineImpl::initDepthStencilState()
{
	m_cache.m_depthCompareFunction =
		convertCompareOperation(m_in.m_depthStencil.m_depthCompareFunction);

	m_hashes.m_depthStencil =
		computeHash(&m_in.m_depthStencil, sizeof(m_in.m_depthStencil));
}

//==============================================================================
void PipelineImpl::initColorState()
{
	for(U i = 0; i < m_in.m_color.m_attachmentCount; ++i)
	{
		Attachment& out = m_cache.m_attachments[i];
		const ColorAttachmentStateInfo& in = m_in.m_color.m_attachments[i];

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

		out.m_channelWriteMask[0] =
			(in.m_channelWriteMask & ColorBit::RED) != 0;
		out.m_channelWriteMask[1] =
			(in.m_channelWriteMask & ColorBit::GREEN) != 0;
		out.m_channelWriteMask[2] =
			(in.m_channelWriteMask & ColorBit::BLUE) != 0;
		out.m_channelWriteMask[3] =
			(in.m_channelWriteMask & ColorBit::ALPHA) != 0;

		if(!(out.m_srcBlendMethod == GL_ONE && out.m_dstBlendMethod == GL_ZERO))
		{
			m_blendEnabled = true;
		}
	}

	m_hashes.m_color = computeHash(&m_in.m_color, sizeof(m_in.m_color));
}

//==============================================================================
void PipelineImpl::setVertexState(GlState& state) const
{
	if(state.m_stateHashes.m_vertex == m_hashes.m_vertex)
	{
		return;
	}

	state.m_stateHashes.m_vertex = m_hashes.m_vertex;

	for(U i = 0; i < m_in.m_vertex.m_attributeCount; ++i)
	{
		const Attribute& attrib = m_cache.m_attribs[i];
		ANKI_ASSERT(attrib.m_type);

		glVertexAttribFormat(i, attrib.m_compCount, attrib.m_type,
			attrib.m_normalized, m_in.m_vertex.m_attributes[i].m_offset);

		glVertexAttribBinding(i, m_in.m_vertex.m_attributes[i].m_binding);
	}

	for(U i = 0; i < m_in.m_vertex.m_bindingCount; ++i)
	{
		ANKI_ASSERT(m_in.m_vertex.m_bindings[i].m_stride > 0);
		state.m_vertexBindingStrides[i] = m_in.m_vertex.m_bindings[i].m_stride;
	}
}

//==============================================================================
void PipelineImpl::setInputAssemblerState(GlState& state) const
{
	if(state.m_stateHashes.m_inputAssembler == m_hashes.m_inputAssembler)
	{
		return;
	}

	state.m_stateHashes.m_inputAssembler = m_hashes.m_inputAssembler;

	state.m_topology = m_cache.m_topology;

	if(m_in.m_inputAssembler.m_primitiveRestartEnabled)
	{
		glEnable(GL_PRIMITIVE_RESTART);
	}
	else
	{
		glDisable(GL_PRIMITIVE_RESTART);
	}
}

//==============================================================================
void PipelineImpl::setTessellationState(GlState& state) const
{
	if(!m_tessellation
		|| state.m_stateHashes.m_tessellation == m_hashes.m_tessellation)
	{
		return;
	}

	state.m_stateHashes.m_tessellation = m_hashes.m_tessellation;

	glPatchParameteri(GL_PATCH_VERTICES,
		m_in.m_tessellation.m_patchControlPointsCount);
}

//==============================================================================
void PipelineImpl::setRasterizerState(GlState& state) const
{
	if(state.m_stateHashes.m_rasterizer == m_hashes.m_rasterizer)
	{
		return;
	}

	state.m_stateHashes.m_rasterizer = m_hashes.m_rasterizer;

	glPolygonMode(GL_FRONT_AND_BACK, m_cache.m_fillMode);
	glCullFace(m_cache.m_cullMode);
	glEnable(GL_CULL_FACE);
}

//==============================================================================
void PipelineImpl::setDepthStencilState(GlState& state) const
{
	if(state.m_stateHashes.m_depthStencil == m_hashes.m_depthStencil)
	{
		return;
	}

	state.m_stateHashes.m_depthStencil = m_hashes.m_depthStencil;

	glDepthMask(m_in.m_depthStencil.m_depthWriteEnabled);
	state.m_depthWriteMask = m_in.m_depthStencil.m_depthWriteEnabled;

	if(m_cache.m_depthCompareFunction == GL_ALWAYS)
	{
		glDisable(GL_DEPTH_TEST);
	}
	else
	{
		glEnable(GL_DEPTH_TEST);
	}

	if(m_in.m_depthStencil.m_polygonOffsetFactor == 0.0
		&& m_in.m_depthStencil.m_polygonOffsetUnits == 0.0)
	{
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
	else
	{
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(m_in.m_depthStencil.m_polygonOffsetFactor,
			m_in.m_depthStencil.m_polygonOffsetUnits);
	}

	glDepthFunc(m_cache.m_depthCompareFunction);
}

//==============================================================================
void PipelineImpl::setColorState(GlState& state) const
{
	if(state.m_stateHashes.m_color == m_hashes.m_color)
	{
		return;
	}

	state.m_stateHashes.m_color = m_hashes.m_color;

	if(m_blendEnabled)
	{
		glEnable(GL_BLEND);

		for(U i = 0; i < m_in.m_color.m_attachmentCount; ++i)
		{
			const Attachment& att = m_cache.m_attachments[i];

			glBlendFunci(i, att.m_srcBlendMethod, att.m_dstBlendMethod);
			glBlendEquationi(i, att.m_blendFunction);
			glColorMaski(i, att.m_channelWriteMask[0],
				att.m_channelWriteMask[1], att.m_channelWriteMask[2],
				att.m_channelWriteMask[3]);

			state.m_colorWriteMasks[i][0] = att.m_channelWriteMask[0];
			state.m_colorWriteMasks[i][1] = att.m_channelWriteMask[1];
			state.m_colorWriteMasks[i][2] = att.m_channelWriteMask[2];
			state.m_colorWriteMasks[i][3] = att.m_channelWriteMask[3];
		}
	}
	else
	{
		glDisable(GL_BLEND);

		for(U i = 0; i < m_in.m_color.m_attachmentCount; ++i)
		{
			const Attachment& att = m_cache.m_attachments[i];

			glColorMaski(i, att.m_channelWriteMask[0],
				att.m_channelWriteMask[1], att.m_channelWriteMask[2],
				att.m_channelWriteMask[3]);

			state.m_colorWriteMasks[i][0] = att.m_channelWriteMask[0];
			state.m_colorWriteMasks[i][1] = att.m_channelWriteMask[1];
			state.m_colorWriteMasks[i][2] = att.m_channelWriteMask[2];
			state.m_colorWriteMasks[i][3] = att.m_channelWriteMask[3];
		}
	}
}

} // end namespace anki

