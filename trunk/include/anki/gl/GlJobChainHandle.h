// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_JOB_CHAIN_HANDLE_H
#define ANKI_GL_GL_JOB_CHAIN_HANDLE_H

#include "anki/gl/GlHandle.h"
#include "anki/gl/GlJobChain.h"

namespace anki {

// Forward
class GlManager;
class GlTextureHandle;

/// @addtogroup opengl_other
/// @{

/// Job chain handle
class GlJobChainHandle: public GlHandle<GlJobChain>
{
public:
	typedef GlHandle<GlJobChain> Base;
	using UserCallback = void(*)(void*);

	/// @name Constructors/Destructor
	/// @{
	GlJobChainHandle();

	/// Create job chain
	explicit GlJobChainHandle(GlManager* gl, 
		GlJobChainInitHints hints = GlJobChainInitHints());

	~GlJobChainHandle();
	/// @}

	/// Add a user job at the end of the chain
	void pushBackUserJob(UserCallback callback, void* data);

	/// Add another job chain for execution
	void pushBackOtherJobChain(GlJobChainHandle& jobs);

	/// Flush chain for deffered deletion
	void flush();

	/// Flush and wait to finish
	void finish();

	/// Compute initialization hints
	GlJobChainInitHints computeInitHints() const
	{
		return _get().computeInitHints();
	}

	/// @name State manipulation
	/// @{

	/// Set clear color buffers value
	void setClearColor(F32 r, F32 g, F32 b, F32 a);

	/// Set clear depth buffer value
	void setClearDepth(F32 value);

	/// Set clear stencil buffer value
	void setClearStencil(U32 value);

	/// Clear color/depth/stencil buffers
	void clearBuffers(U32 mask);

	/// Set the viewport
	void setViewport(U16 minx, U16 miny, U16 maxx, U16 maxy);

	/// Set the color mask
	void setColorWriteMask(Bool red, Bool green, Bool blue, Bool alpha);

	/// Enable/disable depth test
	void enableDepthTest(Bool enable);

	/// Set depth compare function
	void setDepthFunction(GLenum func);

	/// Set depth write mask
	void setDepthWriteMask(Bool write);

	/// Enable or note stencil test
	void enableStencilTest(Bool enable);

	/// Set stencil function. Equivalent to glStencilFunc
	void setStencilFunction(GLenum function, U32 reference, U32 mask);

	/// Set the stencil mask. Equivalent to glStencilMask
	void setStencilPlaneMask(U32 mask);

	/// Set the operations of stencil fail, depth fail, depth pass. Equivalent
	/// to glStencilOp
	void setStencilOperations(GLenum stencFail, GLenum depthFail, 
		GLenum depthPass);

	/// Enable or not blending. Equivalent to glEnable/glDisable(GL_BLEND)
	void enableBlend(Bool enable);

	/// Set blend equation. Equivalent to glBlendEquation
	void setBlendEquation(GLenum equation);

	/// Set the blend functions. Equivalent to glBlendFunc
	void setBlendFunctions(GLenum sfactor, GLenum dfactor);

	/// Set the blend color. Equivalent to glBlendColor
	void setBlendColor(F32 r, F32 g, F32 b, F32 a);

	/// Enable primitive restart
	void enablePrimitiveRestart(Bool enable);

	/// Set patch number of vertices
	void setPatchVertexCount(U32 count);

	/// Enable or not face culling. Equal to glEnable(GL_CULL_FASE)
	void enableCulling(Bool enable);

	/// Set the faces to cull. Works when culling is enabled. Equal to
	/// glCullFace(x)
	void setCullFace(GLenum mode);

	/// Set the polygon offset. Equal to glPolygonOffset() plus 
	/// glEnable(GL_POLYGON_OFFSET_FILL)
	void setPolygonOffset(F32 factor, F32 units);

	/// Enable/disable polygon offset
	void enablePolygonOffset(Bool enable);

	/// Bind many textures
	void bindTextures(U32 first, 
		const std::initializer_list<GlTextureHandle>& textures);
	/// @}

	/// @privatesection
	/// @{
	GlJobChainAllocator<U8> _getAllocator() const
	{
		return _get().getAllocator();
	}

	GlGlobalHeapAllocator<U8> _getGlobalAllocator() const
	{
		return _get().getGlobalAllocator();
	}

	GlJobManager& _getJobManager()
	{
		return _get().getJobManager();
	}

	const GlJobManager& _getJobManager() const
	{
		return _get().getJobManager();
	}

	/// Add a new job to the list
	template<typename TJob, typename... TArgs>
	void _pushBackNewJob(TArgs&&... args)
	{
		_get().template pushBackNewJob<TJob>(std::forward<TArgs>(args)...);
	}

	/// Execute all jobs
	void _executeAllJobs()
	{
		_get().executeAllJobs();
	}
	/// @}
};

/// @}

} // end namespace anki

#endif

