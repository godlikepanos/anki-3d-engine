// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/gl/GlObject.h>
#include <anki/gr/Pipeline.h>

namespace anki {

/// @addtogroup opengl
/// @{

/// Program pipeline
class PipelineImpl: public GlObject
{
public:
	PipelineImpl(GrManager* manager)
		: GlObject(manager)
	{}

	~PipelineImpl()
	{
		destroyDeferred(glDeleteProgramPipelines);
	}

	ANKI_USE_RESULT Error create(const PipelineInitializer& init);

	/// Bind the pipeline to the state
	void bind(GlState& state);

private:
	class Attribute
	{
	public:
		GLenum m_type = 0;
		U8 m_compCount = 0;
		Bool8 m_normalized = false;
	};

	class Attachment
	{
	public:
		GLenum m_srcBlendMethod = GL_ONE;
		GLenum m_dstBlendMethod = GL_ZERO;
		GLenum m_blendFunction = GL_ADD;
		Array<Bool8, 4> m_channelWriteMask;
	};

	Bool8 m_compute = false; ///< Is compute
	Bool8 m_tessellation = false;
	Bool8 m_blendEnabled = false;

	/// Input values.
	PipelineInitializer m_in;

	/// Cached values.
	class
	{
	public:
		Array<Attribute, MAX_VERTEX_ATTRIBUTES> m_attribs;
		GLenum m_topology = 0;
		GLenum m_fillMode = 0;
		GLenum m_cullMode = 0;
		Bool8 m_depthWrite = false;
		GLenum m_depthCompareFunction = 0;
		Array<Attachment, MAX_COLOR_ATTACHMENTS> m_attachments;
	} m_cache;

	/// State hashes.
	class
	{
	public:
		U64 m_vertex = 0;
		U64 m_inputAssembler = 0;
		U64 m_tessellation = 0;
		U64 m_viewport = 0;
		U64 m_rasterizer = 0;
		U64 m_depthStencil = 0;
		U64 m_color = 0;
	} m_hashes;

	/// Attach all the programs
	ANKI_USE_RESULT Error createGlPipeline();

	void initVertexState();
	void initInputAssemblerState();
	void initTessellationState();
	void initRasterizerState();
	void initDepthStencilState();
	void initColorState();

	void setVertexState(GlState& state) const;
	void setInputAssemblerState(GlState& state) const;
	void setTessellationState(GlState& state) const;
	void setViewportState(GlState& state) const
	{}
	void setRasterizerState(GlState& state) const;
	void setDepthStencilState(GlState& state) const;
	void setColorState(GlState& state) const;
};
/// @}

} // end namespace anki

