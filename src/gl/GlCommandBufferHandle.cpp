// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlCommandBufferHandle.h"
#include "anki/gl/GlDevice.h"
#include "anki/gl/GlSyncHandles.h"
#include "anki/gl/GlFramebuffer.h"
#include "anki/gl/GlTextureHandle.h"
#include "anki/gl/GlTexture.h"
#include "anki/util/Vector.h"
#include <utility>

namespace anki {

//==============================================================================
// Macros because we are bored to type
#define ANKI_STATE_CMD_0(type_, glfunc_) \
	class Command: public GlCommand \
	{ \
	public: \
		Command() = default \
		void operator()(GlCommandBuffer*) \
		{ \
			glfunc_(); \
		} \
	}; \
	_pushBackNewCommand<Command>(value_)

#define ANKI_STATE_CMD_1(type_, glfunc_, value_) \
	class Command: public GlCommand \
	{ \
	public: \
		type_ m_value; \
		Command(type_ v) \
			: m_value(v) \
		{} \
		void operator()(GlCommandBuffer*) \
		{ \
			glfunc_(m_value); \
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
		void operator()(GlCommandBuffer*) \
		{ \
			glfunc_(m_value[0], m_value[1]); \
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
		void operator()(GlCommandBuffer*) \
		{ \
			glfunc_(m_value[0], m_value[1], m_value[2]); \
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
		void operator()(GlCommandBuffer*) \
		{ \
			glfunc_(m_value[0], m_value[1], m_value[2], m_value[3]); \
		} \
	}; \
	_pushBackNewCommand<Command>(a_, b_, c_, d_)

#define ANKI_STATE_CMD_ENABLE(enum_, enable_) \
	class Command: public GlCommand \
	{ \
	public: \
		Bool8 m_enable; \
		Command(Bool enable) \
			: m_enable(enable) \
		{} \
		void operator()(GlCommandBuffer*) \
		{ \
			if(m_enable) \
			{ \
				glEnable(enum_); \
			} \
			else \
			{ \
				glDisable(enum_); \
			} \
		} \
	}; \
	_pushBackNewCommand<Command>(enable_)

//==============================================================================
GlCommandBufferHandle::GlCommandBufferHandle()
{}

//==============================================================================
GlCommandBufferHandle::GlCommandBufferHandle(GlDevice* gl, 
	GlCommandBufferInitHints hints)
{
	ANKI_ASSERT(!isCreated());
	ANKI_ASSERT(gl);

	typedef GlGlobalHeapAllocator<GlCommandBuffer> Alloc;
	Alloc alloc = gl->_getAllocator();

	*static_cast<Base*>(this) = Base(
		gl,
		alloc, 
		GlHandleDefaultDeleter<GlCommandBuffer, Alloc>(), 
		&gl->_getQueue(), 
		hints);
}

//==============================================================================
GlCommandBufferHandle::~GlCommandBufferHandle()
{}

//==============================================================================
void GlCommandBufferHandle::pushBackUserCommand(UserCallback callback, void* data)
{
	class Command: public GlCommand
	{
	public:
		UserCallback m_callback;
		void* m_userData;

		Command(UserCallback callback, void* userData)
			: m_callback(callback), m_userData(userData)
		{
			ANKI_ASSERT(m_callback);
		}

		void operator()(GlCommandBuffer* commands)
		{
			(*m_callback)(m_userData);
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
			: m_commands(commands)
		{}

		void operator()(GlCommandBuffer*)
		{
			m_commands._executeAllCommands();
		}
	};

	commands._get().makeImmutable();
	_pushBackNewCommand<Command>(commands);
}

//==============================================================================
void GlCommandBufferHandle::flush()
{
	_get().getQueue().flushCommandChain(*this);
}

//==============================================================================
void GlCommandBufferHandle::finish()
{
	_get().getQueue().finishCommandChain(*this);
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

		void operator()(GlCommandBuffer* commands)
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

		void operator()(GlCommandBuffer* commands)
		{
			GlState& state = commands->getQueue().getState();

			if(state.m_blendSfunc != m_sfactor 
				|| state.m_blendDfunc != m_dfactor)
			{
				glBlendFunc(m_sfactor, m_dfactor);

				state.m_blendSfunc = m_sfactor;
				state.m_blendDfunc = m_dfactor;
			}
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
void GlCommandBufferHandle::bindTextures(U32 first, 
	const std::initializer_list<GlTextureHandle>& textures)
{
	using Vec = Vector<GlTextureHandle, GlCommandBufferAllocator<GlTextureHandle>>;
		
	class Command: public GlCommand
	{
	public:
		Vec m_texes;
		U32 m_first;

		Command(Vec& texes, U32 first)
			: m_first(first)
		{
			m_texes = std::move(texes);
		}

		void operator()(GlCommandBuffer* commands)
		{
			Array<GLuint, 16> names;

			U count = 0;
			for(GlTextureHandle& t : m_texes)
			{
				names[count++] = t._get().getGlName();
			}

			ANKI_ASSERT(count > 0);
			glBindTextures(m_first, count, &names[0]);
		}
	};

	Vec texes(_getAllocator());
	texes.reserve(textures.size());

	for(const GlTextureHandle& t : textures)
	{
		texes.push_back(t);
	}

	_pushBackNewCommand<Command>(texes, first);
}

} // end namespace anki
