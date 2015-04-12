// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_GL_PIPELINE_IMPL_H
#define ANKI_GR_GL_PIPELINE_IMPL_H

#include "anki/gr/gl/GlObject.h"
#include "anki/gr/PipelineCommon.h"

namespace anki {

// Forward
class GlState;

/// @addtogroup opengl
/// @{

/// Program pipeline
class PipelineImpl: public GlObject, private PipelineInitializer
{
public:
	using Base = GlObject;
	using Initializer = PipelineInitializer;

	PipelineImpl(GrManager* manager)
	:	Base(manager)
	{}

	~PipelineImpl()
	{
		destroy();
	}

	ANKI_USE_RESULT Error create(const Initializer& init);

	/// Bind the pipeline to the state
	void bind();

private:
	struct Attribute
	{
		U8 m_compCount = 0;
		Bool8 m_normalized = false;
		GLenum m_type = 0;
	};

	struct Attachment
	{
		GLenum m_srcBlendMethod = GL_ONE;
		GLenum m_dstBlendMethod = GL_ZERO;
		GLenum m_blendFunction = GL_ADD;
		Array<Bool8, 4> m_channelWriteMask;
	};

	Bool m_complete;

	// Cached values
	Array<Attribute, MAX_VERTEX_ATTRIBUTES> m_attribs;
	GLenum m_topology = 0;
	GLenum m_fillMode = 0;
	GLenum m_cullMode = 0;
	Bool8 m_depthWrite = false;
	GLenum m_depthCompareFunction = 0;
	Array<Attachment, MAX_COLOR_ATTACHMENTS> m_attachments;

	/// Create pipeline object
	void createPpline();

	/// Attach all the programs
	ANKI_USE_RESULT Error createGlPipeline();

	void destroy();

	void initVertexState();
	void initInputAssemblerState();
	void initRasterizerState();
	void initDepthStencilState();
	void initColorState();

	void setVertexState(GlState& state);
	void setInputAssemblerState(GlState& state);
	void setTessellationState(GlState& state);
	void setRasterizerState(GlState& state);
	void setDepthStencilState(GlState& state);
	void setColorState(GlState& state);

	const PipelineImpl* getPipelineForState(const SubStateBit bit) const;
	const PipelineImpl* chosePipelineForState(const SubStateBit bit) const;
};
/// @}

} // end namespace anki

#endif

