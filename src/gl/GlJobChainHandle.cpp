#include "anki/gl/GlJobChainHandle.h"
#include "anki/gl/GlManager.h"
#include "anki/gl/GlSyncHandles.h"
#include "anki/gl/GlFramebuffer.h"

namespace anki {

//==============================================================================
// Macros because we are bored to type
#define ANKI_STATE_JOB_0(type_, glfunc_) \
	class Job: public GlJob \
	{ \
	public: \
		Job() = default \
		void operator()(GlJobChain*) \
		{ \
			glfunc_(); \
		} \
	}; \
	_pushBackNewJob<Job>(value_)

#define ANKI_STATE_JOB_1(type_, glfunc_, value_) \
	class Job: public GlJob \
	{ \
	public: \
		type_ m_value; \
		Job(type_ v) \
			: m_value(v) \
		{} \
		void operator()(GlJobChain*) \
		{ \
			glfunc_(m_value); \
		} \
	}; \
	_pushBackNewJob<Job>(value_)

#define ANKI_STATE_JOB_2(type_, glfunc_, a_, b_) \
	class Job: public GlJob \
	{ \
	public: \
		Array<type_, 2> m_value; \
		Job(type_ a, type_ b) \
		{ \
			m_value = {{a, b}}; \
		} \
		void operator()(GlJobChain*) \
		{ \
			glfunc_(m_value[0], m_value[1]); \
		} \
	}; \
	_pushBackNewJob<Job>(a_, b_)

#define ANKI_STATE_JOB_3(type_, glfunc_, a_, b_, c_) \
	class Job: public GlJob \
	{ \
	public: \
		Array<type_, 3> m_value; \
		Job(type_ a, type_ b, type_ c) \
		{ \
			m_value = {{a, b, c}}; \
		} \
		void operator()(GlJobChain*) \
		{ \
			glfunc_(m_value[0], m_value[1], m_value[2]); \
		} \
	}; \
	_pushBackNewJob<Job>(a_, b_, c_)

#define ANKI_STATE_JOB_4(type_, glfunc_, a_, b_, c_, d_) \
	class Job: public GlJob \
	{ \
	public: \
		Array<type_, 4> m_value; \
		Job(type_ a, type_ b, type_ c, type_ d) \
		{ \
			m_value = {{a, b, c, d}}; \
		} \
		void operator()(GlJobChain*) \
		{ \
			glfunc_(m_value[0], m_value[1], m_value[2], m_value[3]); \
		} \
	}; \
	_pushBackNewJob<Job>(a_, b_, c_, d_)

#define ANKI_STATE_JOB_ENABLE(enum_, enable_) \
	class Job: public GlJob \
	{ \
	public: \
		Bool8 m_enable; \
		Job(Bool enable) \
			: m_enable(enable) \
		{} \
		void operator()(GlJobChain*) \
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
	_pushBackNewJob<Job>(enable_)

//==============================================================================
GlJobChainHandle::GlJobChainHandle()
{}

//==============================================================================
GlJobChainHandle::GlJobChainHandle(GlManager* gl, GlJobChainInitHints hints)
{
	ANKI_ASSERT(!isCreated());
	ANKI_ASSERT(gl);

	typedef GlGlobalHeapAllocator<GlJobChain> Alloc;
	Alloc alloc = gl->_getAllocator();

	*static_cast<Base*>(this) = Base(
		gl,
		alloc, 
		GlHandleDefaultDeleter<GlJobChain, Alloc>(), 
		&gl->_getJobManager(), 
		hints);
}

//==============================================================================
GlJobChainHandle::~GlJobChainHandle()
{}

//==============================================================================
void GlJobChainHandle::pushBackUserJob(UserCallback callback, void* data)
{
	class Job: public GlJob
	{
	public:
		UserCallback m_callback;
		void* m_userData;

		Job(UserCallback callback, void* userData)
			: m_callback(callback), m_userData(userData)
		{
			ANKI_ASSERT(m_callback);
		}

		void operator()(GlJobChain* jobs)
		{
			(*m_callback)(m_userData);
		}
	};

	_pushBackNewJob<Job>(callback, data);
}

//==============================================================================
void GlJobChainHandle::pushBackOtherJobChain(GlJobChainHandle& jobs)
{
	class Job: public GlJob
	{
	public:
		GlJobChainHandle m_jobs;

		Job(GlJobChainHandle& jobs)
			: m_jobs(jobs)
		{}

		void operator()(GlJobChain*)
		{
			m_jobs._executeAllJobs();
		}
	};

	jobs._get().makeImmutable();
	_pushBackNewJob<Job>(jobs);
}

//==============================================================================
void GlJobChainHandle::flush()
{
	_get().getJobManager().flushJobChain(*this);
}

//==============================================================================
void GlJobChainHandle::finish()
{
	_get().getJobManager().finishJobChain(*this);
}

//==============================================================================
void GlJobChainHandle::setClearColor(F32 r, F32 g, F32 b, F32 a)
{
	ANKI_STATE_JOB_4(F32, glClearColor, r, g, b, a);
}

//==============================================================================
void GlJobChainHandle::setClearDepth(F32 value)
{
	ANKI_STATE_JOB_1(F32, glClearDepth, value);
}

//==============================================================================
void GlJobChainHandle::setClearStencil(U32 value)
{
	ANKI_STATE_JOB_1(U32, glClearStencil, value);
}

//==============================================================================
void GlJobChainHandle::clearBuffers(U32 mask)
{
	ANKI_STATE_JOB_1(U32, glClear, mask);
}

//==============================================================================
void GlJobChainHandle::setViewport(U16 minx, U16 miny, U16 maxx, U16 maxy)
{
	class Job: public GlJob
	{
	public:
		Array<U16, 4> m_value;
		
		Job(U16 a, U16 b, U16 c, U16 d)
		{
			m_value = {{a, b, c, d}};
		}

		void operator()(GlJobChain* jobs)
		{
			GlState& state = jobs->getJobManager().getState();

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

	_pushBackNewJob<Job>(minx, miny, maxx, maxy);
}

//==============================================================================
void GlJobChainHandle::setColorWriteMask(
	Bool red, Bool green, Bool blue, Bool alpha)
{
	ANKI_STATE_JOB_4(Bool8, glColorMask, red, green, blue, alpha);
}

//==============================================================================
void GlJobChainHandle::enableDepthTest(Bool enable)
{
	ANKI_STATE_JOB_ENABLE(GL_DEPTH_TEST, enable);
}

//==============================================================================
void GlJobChainHandle::setDepthFunction(GLenum func)
{
	ANKI_STATE_JOB_1(GLenum, glDepthFunc, func);
}

//==============================================================================
void GlJobChainHandle::setDepthWriteMask(Bool write)
{
	ANKI_STATE_JOB_1(Bool8, glDepthMask, write);
}

//==============================================================================
void GlJobChainHandle::enableStencilTest(Bool enable)
{
	ANKI_STATE_JOB_ENABLE(GL_STENCIL_TEST, enable);
}

//==============================================================================
void GlJobChainHandle::setStencilFunction(
	GLenum function, U32 reference, U32 mask)
{
	ANKI_STATE_JOB_3(U32, glStencilFunc, function, reference, mask);
}

//==============================================================================
void GlJobChainHandle::setStencilPlaneMask(U32 mask)
{
	ANKI_STATE_JOB_1(U32, glStencilMask, mask);
}

//==============================================================================
void GlJobChainHandle::setStencilOperations(GLenum stencFail, GLenum depthFail, 
	GLenum depthPass)
{
	ANKI_STATE_JOB_3(GLenum, glStencilOp, stencFail, depthFail, depthPass);
}

//==============================================================================
void GlJobChainHandle::enableBlend(Bool enable)
{
	ANKI_STATE_JOB_ENABLE(GL_BLEND, enable);
}

//==============================================================================
void GlJobChainHandle::setBlendEquation(GLenum equation)
{
	ANKI_STATE_JOB_1(GLenum, glBlendEquation, equation);
}

//==============================================================================
void GlJobChainHandle::setBlendFunctions(GLenum sfactor, GLenum dfactor)
{
	class Job: public GlJob
	{
	public:
		GLenum m_sfactor;
		GLenum m_dfactor;

		Job(GLenum sfactor, GLenum dfactor)
			: m_sfactor(sfactor), m_dfactor(dfactor)
		{}

		void operator()(GlJobChain* jobs)
		{
			GlState& state = jobs->getJobManager().getState();

			if(state.m_blendSfunc != m_sfactor 
				|| state.m_blendDfunc != m_dfactor)
			{
				glBlendFunc(m_sfactor, m_dfactor);

				state.m_blendSfunc = m_sfactor;
				state.m_blendDfunc = m_dfactor;
			}
		}
	};

	_pushBackNewJob<Job>(sfactor, dfactor);
}

//==============================================================================
void GlJobChainHandle::setBlendColor(F32 r, F32 g, F32 b, F32 a)
{
	ANKI_STATE_JOB_4(F32, glBlendColor, r, g, b, a);
}

//==============================================================================
void GlJobChainHandle::enablePrimitiveRestart(Bool enable)
{
	ANKI_STATE_JOB_ENABLE(GL_PRIMITIVE_RESTART, enable);
}

//==============================================================================
void GlJobChainHandle::setPatchVertexCount(U32 count)
{
	ANKI_STATE_JOB_2(GLint, glPatchParameteri, GL_PATCH_VERTICES, count);
}

//==============================================================================
void GlJobChainHandle::enableCulling(Bool enable)
{
	ANKI_STATE_JOB_ENABLE(GL_CULL_FACE, enable);
}

//==============================================================================
void GlJobChainHandle::setCullFace(GLenum mode)
{
	ANKI_STATE_JOB_1(GLenum, glCullFace, mode);
}

//==============================================================================
void GlJobChainHandle::setPolygonOffset(F32 factor, F32 units)
{
	ANKI_STATE_JOB_2(F32, glPolygonOffset, factor, units);
}

//==============================================================================
void GlJobChainHandle::enablePolygonOffset(Bool enable)
{
	ANKI_STATE_JOB_ENABLE(GL_POLYGON_OFFSET_FILL, enable);
}

} // end namespace anki
