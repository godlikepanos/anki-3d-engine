// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlCommandBufferHandle.h"
#include "anki/gl/GlDevice.h"
#include "anki/gl/GlSyncHandles.h"
#include "anki/gl/GlFramebuffer.h"
#include "anki/gl/GlTextureHandle.h"
#include "anki/gl/GlTexture.h"
#include "anki/gl/GlBufferHandle.h"
#include "anki/gl/GlBuffer.h"
#include "anki/gl/GlOcclusionQueryHandle.h"
#include "anki/gl/GlOcclusionQuery.h"
#include "anki/core/Counters.h"
#include <utility>

namespace anki {

//==============================================================================
// Macros because we are bored to type
#define ANKI_STATE_CMD_0(type_, glfunc_) \
	class Command: public GlCommand \
	{ \
	public: \
		Command() = default \
		Error operator()(GlCommandBuffer*) \
		{ \
			glfunc_(); \
			return ErrorCode::NONE; \
		} \
	}; \
	_pushBackNewCommand<Command>(value_)

#define ANKI_STATE_CMD_1(type_, glfunc_, value_) \
	class Command: public GlCommand \
	{ \
	public: \
		type_ m_value; \
		Command(type_ v) \
		:	m_value(v) \
		{} \
		Error operator()(GlCommandBuffer*) \
		{ \
			glfunc_(m_value); \
			return ErrorCode::NONE; \
		} \
	}; \
	_pushBackNewCommand<Command>(value_)

#define ANKI_STATE_CMD_2(type_, glfunc_, a_, b_) \
	class Command: public GlCommand \
	{ \
	public: \
		Array<type_, 2> m_value; \
		Command(type_ a, type_ b) \
		{ \
			m_value = {{a, b}}; \
		} \
		Error operator()(GlCommandBuffer*) \
		{ \
			glfunc_(m_value[0], m_value[1]); \
			return ErrorCode::NONE; \
		} \
	}; \
	_pushBackNewCommand<Command>(a_, b_)

#define ANKI_STATE_CMD_3(type_, glfunc_, a_, b_, c_) \
	class Command: public GlCommand \
	{ \
	public: \
		Array<type_, 3> m_value; \
		Command(type_ a, type_ b, type_ c) \
		{ \
			m_value = {{a, b, c}}; \
		} \
		Error operator()(GlCommandBuffer*) \
		{ \
			glfunc_(m_value[0], m_value[1], m_value[2]); \
			return ErrorCode::NONE; \
		} \
	}; \
	_pushBackNewCommand<Command>(a_, b_, c_)

#define ANKI_STATE_CMD_4(type_, glfunc_, a_, b_, c_, d_) \
	class Command: public GlCommand \
	{ \
	public: \
		Array<type_, 4> m_value; \
		Command(type_ a, type_ b, type_ c, type_ d) \
		{ \
			m_value = {{a, b, c, d}}; \
		} \
		Error operator()(GlCommandBuffer*) \
		{ \
			glfunc_(m_value[0], m_value[1], m_value[2], m_value[3]); \
			return ErrorCode::NONE; \
		} \
	}; \
	_pushBackNewCommand<Command>(a_, b_, c_, d_)

#define ANKI_STATE_CMD_ENABLE(enum_, enable_) \
	class Command: public GlCommand \
	{ \
	public: \
		Bool8 m_enable; \
		Command(Bool enable) \
		:	m_enable(enable) \
		{} \
		Error operator()(GlCommandBuffer*) \
		{ \
			if(m_enable) \
			{ \
				glEnable(enum_); \
			} \
			else \
			{ \
				glDisable(enum_); \
			} \
			return ErrorCode::NONE; \
		} \
	}; \
	_pushBackNewCommand<Command>(enable_)

//==============================================================================
GlCommandBufferHandle::GlCommandBufferHandle()
{}

//==============================================================================
GlCommandBufferHandle::~GlCommandBufferHandle()
{}

//==============================================================================
Error GlCommandBufferHandle::create(GlDevice* gl, 
	GlCommandBufferInitHints hints)
{
	ANKI_ASSERT(!isCreated());
	ANKI_ASSERT(gl);

	using Alloc = GlAllocator<GlCommandBuffer>;
	Alloc alloc = gl->_getAllocator();

	Error err = _createAdvanced(
		gl,
		alloc, 
		GlHandleDefaultDeleter<GlCommandBuffer, Alloc>());

	if(!err)
	{
		err = _get().create(&gl->_getQueue(), hints);
	}

	return err;
}

//==============================================================================
void GlCommandBufferHandle::pushBackUserCommand(
	UserCallback callback, void* data)
{
	class Command: public GlCommand
	{
	public:
		UserCallback m_callback;
		void* m_userData;

		Command(UserCallback callback, void* userData)
		:	m_callback(callback), 
			m_userData(userData)
		{
			ANKI_ASSERT(m_callback);
		}

		Error operator()(GlCommandBuffer* commands)
		{
			return (*m_callback)(m_userData);
		}
	};

	_pushBackNewCommand<Command>(callback, data);
}

//==============================================================================
void GlCommandBufferHandle::pushBackOtherCommandBuffer(
	GlCommandBufferHandle& commands)
{
	class Command: public GlCommand
	{
	public:
		GlCommandBufferHandle m_commands;

		Command(GlCommandBufferHandle& commands)
		:	m_commands(commands)
		{}

		Error operator()(GlCommandBuffer*)
		{
			return m_commands._executeAllCommands();
		}
	};

	commands._get().makeImmutable();
	_pushBackNewCommand<Command>(commands);
}

//==============================================================================
void GlCommandBufferHandle::flush()
{
	_get().getQueue().flushCommandBuffer(*this);
}

//==============================================================================
void GlCommandBufferHandle::finish()
{
	_get().getQueue().finishCommandBuffer(*this);
}

//==============================================================================
void GlCommandBufferHandle::setClearColor(F32 r, F32 g, F32 b, F32 a)
{
	ANKI_STATE_CMD_4(F32, glClearColor, r, g, b, a);
}

//==============================================================================
void GlCommandBufferHandle::setClearDepth(F32 value)
{
	ANKI_STATE_CMD_1(F32, glClearDepth, value);
}

//==============================================================================
void GlCommandBufferHandle::setClearStencil(U32 value)
{
	ANKI_STATE_CMD_1(U32, glClearStencil, value);
}

//==============================================================================
void GlCommandBufferHandle::clearBuffers(U32 mask)
{
	ANKI_STATE_CMD_1(U32, glClear, mask);
}

//==============================================================================
void GlCommandBufferHandle::setViewport(U16 minx, U16 miny, U16 maxx, U16 maxy)
{
	class Command: public GlCommand
	{
	public:
		Array<U16, 4> m_value;
		
		Command(U16 a, U16 b, U16 c, U16 d)
		{
			m_value = {{a, b, c, d}};
		}

		Error operator()(GlCommandBuffer* commands)
		{
			GlState& state = commands->getQueue().getState();

			if(state.m_viewport[0] != m_value[0] 
				|| state.m_viewport[1] != m_value[1]
				|| state.m_viewport[2] != m_value[2] 
				|| state.m_viewport[3] != m_value[3])
			{
				glViewport(m_value[0], m_value[1], m_value[2], m_value[3]);

				state.m_viewport = m_value;
			}

			return ErrorCode::NONE;
		}
	};

	_pushBackNewCommand<Command>(minx, miny, maxx, maxy);
}

//==============================================================================
void GlCommandBufferHandle::setColorWriteMask(
	Bool red, Bool green, Bool blue, Bool alpha)
{
	ANKI_STATE_CMD_4(Bool8, glColorMask, red, green, blue, alpha);
}

//==============================================================================
void GlCommandBufferHandle::enableDepthTest(Bool enable)
{
	ANKI_STATE_CMD_ENABLE(GL_DEPTH_TEST, enable);
}

//==============================================================================
void GlCommandBufferHandle::setDepthFunction(GLenum func)
{
	ANKI_STATE_CMD_1(GLenum, glDepthFunc, func);
}

//==============================================================================
void GlCommandBufferHandle::setDepthWriteMask(Bool write)
{
	ANKI_STATE_CMD_1(Bool8, glDepthMask, write);
}

//==============================================================================
void GlCommandBufferHandle::enableStencilTest(Bool enable)
{
	ANKI_STATE_CMD_ENABLE(GL_STENCIL_TEST, enable);
}

//==============================================================================
void GlCommandBufferHandle::setStencilFunction(
	GLenum function, U32 reference, U32 mask)
{
	ANKI_STATE_CMD_3(U32, glStencilFunc, function, reference, mask);
}

//==============================================================================
void GlCommandBufferHandle::setStencilPlaneMask(U32 mask)
{
	ANKI_STATE_CMD_1(U32, glStencilMask, mask);
}

//==============================================================================
void GlCommandBufferHandle::setStencilOperations(GLenum stencFail, GLenum depthFail, 
	GLenum depthPass)
{
	ANKI_STATE_CMD_3(GLenum, glStencilOp, stencFail, depthFail, depthPass);
}

//==============================================================================
void GlCommandBufferHandle::enableBlend(Bool enable)
{
	ANKI_STATE_CMD_ENABLE(GL_BLEND, enable);
}

//==============================================================================
void GlCommandBufferHandle::setBlendEquation(GLenum equation)
{
	ANKI_STATE_CMD_1(GLenum, glBlendEquation, equation);
}

//==============================================================================
void GlCommandBufferHandle::setBlendFunctions(GLenum sfactor, GLenum dfactor)
{
	class Command: public GlCommand
	{
	public:
		GLenum m_sfactor;
		GLenum m_dfactor;

		Command(GLenum sfactor, GLenum dfactor)
			: m_sfactor(sfactor), m_dfactor(dfactor)
		{}

		Error operator()(GlCommandBuffer* commands)
		{
			GlState& state = commands->getQueue().getState();

			if(state.m_blendSfunc != m_sfactor 
				|| state.m_blendDfunc != m_dfactor)
			{
				glBlendFunc(m_sfactor, m_dfactor);

				state.m_blendSfunc = m_sfactor;
				state.m_blendDfunc = m_dfactor;
			}

			return ErrorCode::NONE;
		}
	};

	_pushBackNewCommand<Command>(sfactor, dfactor);
}

//==============================================================================
void GlCommandBufferHandle::setBlendColor(F32 r, F32 g, F32 b, F32 a)
{
	ANKI_STATE_CMD_4(F32, glBlendColor, r, g, b, a);
}

//==============================================================================
void GlCommandBufferHandle::enablePrimitiveRestart(Bool enable)
{
	ANKI_STATE_CMD_ENABLE(GL_PRIMITIVE_RESTART, enable);
}

//==============================================================================
void GlCommandBufferHandle::setPatchVertexCount(U32 count)
{
	ANKI_STATE_CMD_2(GLint, glPatchParameteri, GL_PATCH_VERTICES, count);
}

//==============================================================================
void GlCommandBufferHandle::enableCulling(Bool enable)
{
	ANKI_STATE_CMD_ENABLE(GL_CULL_FACE, enable);
}

//==============================================================================
void GlCommandBufferHandle::setCullFace(GLenum mode)
{
	ANKI_STATE_CMD_1(GLenum, glCullFace, mode);
}

//==============================================================================
void GlCommandBufferHandle::setPolygonOffset(F32 factor, F32 units)
{
	ANKI_STATE_CMD_2(F32, glPolygonOffset, factor, units);
}

//==============================================================================
void GlCommandBufferHandle::enablePolygonOffset(Bool enable)
{
	ANKI_STATE_CMD_ENABLE(GL_POLYGON_OFFSET_FILL, enable);
}

//==============================================================================
void GlCommandBufferHandle::setPolygonMode(GLenum face, GLenum mode)
{
	ANKI_STATE_CMD_2(GLenum, glPolygonMode, face, mode);
}

//==============================================================================
void GlCommandBufferHandle::enablePointSize(Bool enable)
{
	ANKI_STATE_CMD_ENABLE(GL_PROGRAM_POINT_SIZE, enable);
}

//==============================================================================
class BindTexturesCommand: public GlCommand
{
public:
	static const U MAX_BIND_TEXTURES = 8;

	Array<GlTextureHandle, MAX_BIND_TEXTURES> m_texes;
	U32 m_texCount;
	U32 m_first;

	BindTexturesCommand(
		GlTextureHandle textures[], U count, U32 first)
	:	m_first(first)
	{
		m_texCount = count;
		GlTextureHandle* t = textures;
		while(count-- != 0)
		{
			m_texes[count] = *t;
			++t;
		}
	}

	Error operator()(GlCommandBuffer* commands)
	{
		Array<GLuint, MAX_BIND_TEXTURES> names;

		U count = m_texCount;
		U i = 0;
		while(count-- != 0)
		{
			names[i++] = m_texes[count]._get().getGlName();
		}

		glBindTextures(m_first, m_texCount, &names[0]);

		return ErrorCode::NONE;
	}
};

void GlCommandBufferHandle::bindTextures(U32 first, 
	GlTextureHandle textures[], U32 count)
{
	ANKI_ASSERT(count > 0);

	_pushBackNewCommand<BindTexturesCommand>(&textures[0], count, first);
}

//==============================================================================
class DrawElementsCondCommand: public GlCommand
{
public:
	GLenum m_mode;
	U8 m_indexSize;
	GlDrawElementsIndirectInfo m_info;
	GlOcclusionQueryHandle m_query;

	DrawElementsCondCommand(
		GLenum mode,
		U8 indexSize,
		GlDrawElementsIndirectInfo& info,
		GlOcclusionQueryHandle query = GlOcclusionQueryHandle())
	:	m_mode(mode),
		m_indexSize(indexSize),
		m_info(info),
		m_query(query)
	{}

	Error operator()(GlCommandBuffer*)
	{
		ANKI_ASSERT(m_indexSize != 0);

		GLenum indicesType = 0;
		switch(m_indexSize)
		{
		case 1:
			indicesType = GL_UNSIGNED_BYTE;
			break;
		case 2:
			indicesType = GL_UNSIGNED_SHORT;
			break;
		case 4:
			indicesType = GL_UNSIGNED_INT;
			break;
		default:
			ANKI_ASSERT(0);
			break;
		};

		if(!m_query.isCreated() || !m_query._get().skipDrawcall())
		{
			glDrawElementsInstancedBaseVertexBaseInstance(
				m_mode,
				m_info.m_count,
				indicesType,
				(const void*)(PtrSize)(m_info.m_firstIndex * m_indexSize),
				m_info.m_instanceCount,
				m_info.m_baseVertex,
				m_info.m_baseInstance);

			ANKI_COUNTER_INC(GL_DRAWCALLS_COUNT, (U64)1);
		}

		return ErrorCode::NONE;
	}
};

void GlCommandBufferHandle::drawElements(
	GLenum mode, U8 indexSize, U32 count, U32 instanceCount, U32 firstIndex,
	U32 baseVertex, U32 baseInstance)
{
	GlDrawElementsIndirectInfo info(count, instanceCount, firstIndex, 
		baseVertex, baseInstance);

	_pushBackNewCommand<DrawElementsCondCommand>(mode, indexSize, info);
}

//==============================================================================
class DrawArraysCondCommand: public GlCommand
{
public:
	GLenum m_mode;
	GlDrawArraysIndirectInfo m_info;
	GlOcclusionQueryHandle m_query;

	DrawArraysCondCommand(
		GLenum mode,
		GlDrawArraysIndirectInfo& info, 
		GlOcclusionQueryHandle query = GlOcclusionQueryHandle())
	:	m_mode(mode),
		m_info(info),
		m_query(query)
	{}

	Error operator()(GlCommandBuffer*)
	{
		if(!m_query.isCreated() || !m_query._get().skipDrawcall())
		{
			glDrawArraysInstancedBaseInstance(
				m_mode,
				m_info.m_first,
				m_info.m_count,
				m_info.m_instanceCount,
				m_info.m_baseInstance);

			ANKI_COUNTER_INC(GL_DRAWCALLS_COUNT, (U64)1);
		}

		return ErrorCode::NONE;
	}
};

void GlCommandBufferHandle::drawArrays(
	GLenum mode, U32 count, U32 instanceCount, U32 first, U32 baseInstance)
{
	GlDrawArraysIndirectInfo info(count, instanceCount, first, baseInstance);

	_pushBackNewCommand<DrawArraysCondCommand>(mode, info);
}

//==============================================================================
void GlCommandBufferHandle::drawElementsConditional(
	GlOcclusionQueryHandle& query,
	GLenum mode, U8 indexSize, U32 count, U32 instanceCount, U32 firstIndex,
	U32 baseVertex, U32 baseInstance)
{
	GlDrawElementsIndirectInfo info(count, instanceCount, firstIndex, 
		baseVertex, baseInstance);

	_pushBackNewCommand<DrawElementsCondCommand>(mode, indexSize, info, query);
}

//==============================================================================
void GlCommandBufferHandle::drawArraysConditional(
	GlOcclusionQueryHandle& query,
	GLenum mode, U32 count, U32 instanceCount, U32 first, U32 baseInstance)
{
	GlDrawArraysIndirectInfo info(count, instanceCount, first, baseInstance);

	_pushBackNewCommand<DrawArraysCondCommand>(mode, info, query);
}

//==============================================================================
class CopyBuffTex: public GlCommand
{
public:
	GlTextureHandle m_tex;
	GlBufferHandle m_buff;

	CopyBuffTex(GlTextureHandle& from, GlBufferHandle& to)
	:	m_tex(from),
		m_buff(to)
	{}

	Error operator()(GlCommandBuffer* cmd)
	{
		GlTexture& tex = m_tex._get();
		GlBuffer& buff = m_buff._get();

		// Bind
		GLuint copyFbo = cmd->getQueue().getCopyFbo();
		glBindFramebuffer(GL_FRAMEBUFFER, copyFbo);

		// Attach texture
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			tex.getTarget(), tex.getGlName(), 0);

		// Set draw buffers
		GLuint drawBuff = GL_COLOR_ATTACHMENT0;
		glDrawBuffers(1, &drawBuff);

		// Bind buffer
		ANKI_ASSERT(m_buff.getTarget() == GL_PIXEL_PACK_BUFFER);
		buff.bind();

		// Read pixels
		GLuint format, type;
		if(tex.getInternalFormat() == GL_RG32UI)
		{
			format = GL_RG_INTEGER;
			type = GL_UNSIGNED_INT;
		}
		else if(tex.getInternalFormat() == GL_RG32F)
		{
			format = GL_RG;
			type = GL_FLOAT;
		}
		else
		{
			ANKI_ASSERT(0 && "Not implemented");
		}

		glReadPixels(0, 0, tex.getWidth(), tex.getHeight(), 
			format, type, nullptr);

		// End
		buff.unbind();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return ErrorCode::NONE;
	}
};

void GlCommandBufferHandle::copyTextureToBuffer(
	GlTextureHandle& from, GlBufferHandle& to)
{
	_pushBackNewCommand<CopyBuffTex>(from, to);
}

} // end namespace anki
