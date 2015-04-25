// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/CommandBufferHandle.h"
#include "anki/gr/gl/CommandBufferImpl.h"
#include "anki/gr/GrManager.h"
#include "anki/gr/gl/GrManagerImpl.h"
#include "anki/gr/gl/RenderingThread.h"
#include "anki/gr/gl/FramebufferImpl.h"
#include "anki/gr/TextureHandle.h"
#include "anki/gr/gl/TextureImpl.h"
#include "anki/gr/BufferHandle.h"
#include "anki/gr/gl/BufferImpl.h"
#include "anki/gr/OcclusionQueryHandle.h"
#include "anki/gr/gl/OcclusionQueryImpl.h"
#include "anki/core/Counters.h"
#include <utility>

namespace anki {

//==============================================================================
// Macros because we are bored to type
#define ANKI_STATE_CMD_0(type_, glfunc_) \
	class Command final: public GlCommand \
	{ \
	public: \
		Command() = default \
		Error operator()(CommandBufferImpl*) \
		{ \
			glfunc_(); \
			return ErrorCode::NONE; \
		} \
	}; \
	get().pushBackNewCommand<Command>(value_)

#define ANKI_STATE_CMD_1(type_, glfunc_, value_) \
	class Command: public GlCommand \
	{ \
	public: \
		type_ m_value; \
		Command(type_ v) \
		:	m_value(v) \
		{} \
		Error operator()(CommandBufferImpl*) \
		{ \
			glfunc_(m_value); \
			return ErrorCode::NONE; \
		} \
	}; \
	get().pushBackNewCommand<Command>(value_)

#define ANKI_STATE_CMD_2(type_, glfunc_, a_, b_) \
	class Command: public GlCommand \
	{ \
	public: \
		Array<type_, 2> m_value; \
		Command(type_ a, type_ b) \
		{ \
			m_value = {{a, b}}; \
		} \
		Error operator()(CommandBufferImpl*) \
		{ \
			glfunc_(m_value[0], m_value[1]); \
			return ErrorCode::NONE; \
		} \
	}; \
	get().pushBackNewCommand<Command>(a_, b_)

#define ANKI_STATE_CMD_3(type_, glfunc_, a_, b_, c_) \
	class Command: public GlCommand \
	{ \
	public: \
		Array<type_, 3> m_value; \
		Command(type_ a, type_ b, type_ c) \
		{ \
			m_value = {{a, b, c}}; \
		} \
		Error operator()(CommandBufferImpl*) \
		{ \
			glfunc_(m_value[0], m_value[1], m_value[2]); \
			return ErrorCode::NONE; \
		} \
	}; \
	get().pushBackNewCommand<Command>(a_, b_, c_)

#define ANKI_STATE_CMD_4(type_, glfunc_, a_, b_, c_, d_) \
	class Command: public GlCommand \
	{ \
	public: \
		Array<type_, 4> m_value; \
		Command(type_ a, type_ b, type_ c, type_ d) \
		{ \
			m_value = {{a, b, c, d}}; \
		} \
		Error operator()(CommandBufferImpl*) \
		{ \
			glfunc_(m_value[0], m_value[1], m_value[2], m_value[3]); \
			return ErrorCode::NONE; \
		} \
	}; \
	get().pushBackNewCommand<Command>(a_, b_, c_, d_)

#define ANKI_STATE_CMD_ENABLE(enum_, enable_) \
	class Command: public GlCommand \
	{ \
	public: \
		Bool8 m_enable; \
		Command(Bool enable) \
		:	m_enable(enable) \
		{} \
		Error operator()(CommandBufferImpl*) \
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
	get().pushBackNewCommand<Command>(enable_)

//==============================================================================
CommandBufferHandle::CommandBufferHandle()
{}

//==============================================================================
CommandBufferHandle::~CommandBufferHandle()
{}

//==============================================================================
Error CommandBufferHandle::create(GrManager* manager, 
	CommandBufferInitHints hints)
{
	ANKI_ASSERT(!isCreated());
	ANKI_ASSERT(manager);

	Base::create(
		*manager,
		GrHandleDefaultDeleter<CommandBufferImpl>());

	get().create(hints);

	return ErrorCode::NONE;
}

//==============================================================================
CommandBufferInitHints CommandBufferHandle::computeInitHints() const
{
	return get().computeInitHints();
}

//==============================================================================
void CommandBufferHandle::pushBackUserCommand(
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

		Error operator()(CommandBufferImpl* commands)
		{
			return (*m_callback)(m_userData);
		}
	};

	get().pushBackNewCommand<Command>(callback, data);
}

//==============================================================================
void CommandBufferHandle::pushBackOtherCommandBuffer(
	CommandBufferHandle& commands)
{
	class Command: public GlCommand
	{
	public:
		CommandBufferHandle m_commands;

		Command(CommandBufferHandle& commands)
		:	m_commands(commands)
		{}

		Error operator()(CommandBufferImpl*)
		{
			return m_commands.get().executeAllCommands();
		}
	};

	commands.get().makeImmutable();
	get().pushBackNewCommand<Command>(commands);
}

//==============================================================================
void CommandBufferHandle::flush()
{
	get().getManager().getImplementation().
		getRenderingThread().flushCommandBuffer(*this);
}

//==============================================================================
void CommandBufferHandle::finish()
{
	get().getManager().getImplementation().getRenderingThread().
		finishCommandBuffer(*this);
}

//==============================================================================
void CommandBufferHandle::setViewport(U16 minx, U16 miny, U16 maxx, U16 maxy)
{
	class Command: public GlCommand
	{
	public:
		Array<U16, 4> m_value;
		
		Command(U16 a, U16 b, U16 c, U16 d)
		{
			m_value = {{a, b, c, d}};
		}

		Error operator()(CommandBufferImpl* commands)
		{
			GlState& state = commands->getManager().getImplementation().
				getRenderingThread().getState();

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

	get().pushBackNewCommand<Command>(minx, miny, maxx, maxy);
}

//==============================================================================
void CommandBufferHandle::setColorWriteMask(
	Bool red, Bool green, Bool blue, Bool alpha)
{
	ANKI_STATE_CMD_4(Bool8, glColorMask, red, green, blue, alpha);
}

//==============================================================================
void CommandBufferHandle::enableDepthTest(Bool enable)
{
	ANKI_STATE_CMD_ENABLE(GL_DEPTH_TEST, enable);
}

//==============================================================================
void CommandBufferHandle::setDepthFunction(GLenum func)
{
	ANKI_STATE_CMD_1(GLenum, glDepthFunc, func);
}

//==============================================================================
void CommandBufferHandle::setDepthWriteMask(Bool write)
{
	ANKI_STATE_CMD_1(Bool8, glDepthMask, write);
}

//==============================================================================
void CommandBufferHandle::enableStencilTest(Bool enable)
{
	ANKI_STATE_CMD_ENABLE(GL_STENCIL_TEST, enable);
}

//==============================================================================
void CommandBufferHandle::setStencilFunction(
	GLenum function, U32 reference, U32 mask)
{
	ANKI_STATE_CMD_3(U32, glStencilFunc, function, reference, mask);
}

//==============================================================================
void CommandBufferHandle::setStencilPlaneMask(U32 mask)
{
	ANKI_STATE_CMD_1(U32, glStencilMask, mask);
}

//==============================================================================
void CommandBufferHandle::setStencilOperations(GLenum stencFail, GLenum depthFail, 
	GLenum depthPass)
{
	ANKI_STATE_CMD_3(GLenum, glStencilOp, stencFail, depthFail, depthPass);
}

//==============================================================================
void CommandBufferHandle::enableBlend(Bool enable)
{
	ANKI_STATE_CMD_ENABLE(GL_BLEND, enable);
}

//==============================================================================
void CommandBufferHandle::setBlendEquation(GLenum equation)
{
	ANKI_STATE_CMD_1(GLenum, glBlendEquation, equation);
}

//==============================================================================
void CommandBufferHandle::setBlendFunctions(GLenum sfactor, GLenum dfactor)
{
	class Command: public GlCommand
	{
	public:
		GLenum m_sfactor;
		GLenum m_dfactor;

		Command(GLenum sfactor, GLenum dfactor)
			: m_sfactor(sfactor), m_dfactor(dfactor)
		{}

		Error operator()(CommandBufferImpl* commands)
		{
			GlState& state = commands->getManager().getImplementation().
				getRenderingThread().getState();

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

	get().pushBackNewCommand<Command>(sfactor, dfactor);
}

//==============================================================================
void CommandBufferHandle::setBlendColor(F32 r, F32 g, F32 b, F32 a)
{
	ANKI_STATE_CMD_4(F32, glBlendColor, r, g, b, a);
}

//==============================================================================
void CommandBufferHandle::enablePrimitiveRestart(Bool enable)
{
	ANKI_STATE_CMD_ENABLE(GL_PRIMITIVE_RESTART, enable);
}

//==============================================================================
void CommandBufferHandle::setPatchVertexCount(U32 count)
{
	ANKI_STATE_CMD_2(GLint, glPatchParameteri, GL_PATCH_VERTICES, count);
}

//==============================================================================
void CommandBufferHandle::enableCulling(Bool enable)
{
	ANKI_STATE_CMD_ENABLE(GL_CULL_FACE, enable);
}

//==============================================================================
void CommandBufferHandle::setCullFace(GLenum mode)
{
	ANKI_STATE_CMD_1(GLenum, glCullFace, mode);
}

//==============================================================================
void CommandBufferHandle::setPolygonOffset(F32 factor, F32 units)
{
	ANKI_STATE_CMD_2(F32, glPolygonOffset, factor, units);
}

//==============================================================================
void CommandBufferHandle::enablePolygonOffset(Bool enable)
{
	ANKI_STATE_CMD_ENABLE(GL_POLYGON_OFFSET_FILL, enable);
}

//==============================================================================
void CommandBufferHandle::setPolygonMode(GLenum face, GLenum mode)
{
	ANKI_STATE_CMD_2(GLenum, glPolygonMode, face, mode);
}

//==============================================================================
void CommandBufferHandle::enablePointSize(Bool enable)
{
	ANKI_STATE_CMD_ENABLE(GL_PROGRAM_POINT_SIZE, enable);
}

//==============================================================================
class BindTexturesCommand: public GlCommand
{
public:
	static const U MAX_BIND_TEXTURES = 8;

	Array<TextureHandle, MAX_BIND_TEXTURES> m_texes;
	U32 m_texCount;
	U32 m_first;

	BindTexturesCommand(
		TextureHandle textures[], U count, U32 first)
	:	m_first(first)
	{
		m_texCount = count;
		TextureHandle* t = textures;
		while(count-- != 0)
		{
			m_texes[count] = *t;
			++t;
		}
	}

	Error operator()(CommandBufferImpl* commands)
	{
		Array<GLuint, MAX_BIND_TEXTURES> names;

		U count = m_texCount;
		U i = 0;
		while(count-- != 0)
		{
			names[i++] = m_texes[count].get().getGlName();
		}

		glBindTextures(m_first, m_texCount, &names[0]);

		return ErrorCode::NONE;
	}
};

void CommandBufferHandle::bindTextures(U32 first, 
	TextureHandle textures[], U32 count)
{
	ANKI_ASSERT(count > 0);

	get().pushBackNewCommand<BindTexturesCommand>(&textures[0], count, first);
}

//==============================================================================
class DrawElementsCondCommand: public GlCommand
{
public:
	GLenum m_mode;
	U8 m_indexSize;
	GlDrawElementsIndirectInfo m_info;
	OcclusionQueryHandle m_query;

	DrawElementsCondCommand(
		GLenum mode,
		U8 indexSize,
		GlDrawElementsIndirectInfo& info,
		OcclusionQueryHandle query = OcclusionQueryHandle())
	:	m_mode(mode),
		m_indexSize(indexSize),
		m_info(info),
		m_query(query)
	{}

	Error operator()(CommandBufferImpl*)
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

		if(!m_query.isCreated() || !m_query.get().skipDrawcall())
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

void CommandBufferHandle::drawElements(
	GLenum mode, U8 indexSize, U32 count, U32 instanceCount, U32 firstIndex,
	U32 baseVertex, U32 baseInstance)
{
	GlDrawElementsIndirectInfo info(count, instanceCount, firstIndex, 
		baseVertex, baseInstance);

	get().pushBackNewCommand<DrawElementsCondCommand>(mode, indexSize, info);
}

//==============================================================================
class DrawArraysCondCommand: public GlCommand
{
public:
	GLenum m_mode;
	GlDrawArraysIndirectInfo m_info;
	OcclusionQueryHandle m_query;

	DrawArraysCondCommand(
		GLenum mode,
		GlDrawArraysIndirectInfo& info, 
		OcclusionQueryHandle query = OcclusionQueryHandle())
	:	m_mode(mode),
		m_info(info),
		m_query(query)
	{}

	Error operator()(CommandBufferImpl*)
	{
		if(!m_query.isCreated() || !m_query.get().skipDrawcall())
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

void CommandBufferHandle::drawArrays(
	GLenum mode, U32 count, U32 instanceCount, U32 first, U32 baseInstance)
{
	GlDrawArraysIndirectInfo info(count, instanceCount, first, baseInstance);

	get().pushBackNewCommand<DrawArraysCondCommand>(mode, info);
}

//==============================================================================
void CommandBufferHandle::drawElementsConditional(
	OcclusionQueryHandle& query,
	GLenum mode, U8 indexSize, U32 count, U32 instanceCount, U32 firstIndex,
	U32 baseVertex, U32 baseInstance)
{
	GlDrawElementsIndirectInfo info(count, instanceCount, firstIndex, 
		baseVertex, baseInstance);

	get().pushBackNewCommand<DrawElementsCondCommand>(mode, indexSize, info, query);
}

//==============================================================================
void CommandBufferHandle::drawArraysConditional(
	OcclusionQueryHandle& query,
	GLenum mode, U32 count, U32 instanceCount, U32 first, U32 baseInstance)
{
	GlDrawArraysIndirectInfo info(count, instanceCount, first, baseInstance);

	get().pushBackNewCommand<DrawArraysCondCommand>(mode, info, query);
}

//==============================================================================
class CopyBuffTex: public GlCommand
{
public:
	TextureHandle m_tex;
	BufferHandle m_buff;

	CopyBuffTex(TextureHandle& from, BufferHandle& to)
	:	m_tex(from),
		m_buff(to)
	{}

	Error operator()(CommandBufferImpl* cmd)
	{
		TextureImpl& tex = m_tex.get();
		BufferImpl& buff = m_buff.get();

		// Bind
		GLuint copyFbo = cmd->getManager().getImplementation().
			getRenderingThread().getCopyFbo();
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
		GLuint format = GL_NONE, type = GL_NONE;
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

void CommandBufferHandle::copyTextureToBuffer(
	TextureHandle& from, BufferHandle& to)
{
	get().pushBackNewCommand<CopyBuffTex>(from, to);
}

//==============================================================================
class DispatchCommand: public GlCommand
{
public:
	Array<U32, 3> m_size;

	DispatchCommand(U32 x, U32 y, U32 z)
	:	m_size({x, y, z})
	{}

	Error operator()(CommandBufferImpl*)
	{
		glDispatchCompute(m_size[0], m_size[1], m_size[2]);
		return ErrorCode::NONE;
	}
};

void CommandBufferHandle::dispatchCompute(
	U32 groupCountX, U32 groupCountY, U32 groupCountZ)
{
	get().pushBackNewCommand<DispatchCommand>(
		groupCountX, groupCountY, groupCountZ);
}

} // end namespace anki
