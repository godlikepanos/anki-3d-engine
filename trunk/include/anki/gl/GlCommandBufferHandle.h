// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_COMMAND_BUFFER_HANDLE_H
#define ANKI_GL_GL_COMMAND_BUFFER_HANDLE_H

#include "anki/gl/GlHandle.h"
#include "anki/gl/GlCommandBuffer.h"

namespace anki {

// Forward
class GlDevice;
class GlTextureHandle;

/// @addtogroup opengl_other
/// @{

/// Command buffer handle
class GlCommandBufferHandle: public GlHandle<GlCommandBuffer>
{
public:
	using Base = GlHandle<GlCommandBuffer>;
	using UserCallback = void(*)(void*);

	GlCommandBufferHandle();

	/// Create command buffer
	explicit GlCommandBufferHandle(GlDevice* gl, 
		GlCommandBufferInitHints hints = GlCommandBufferInitHints());

	~GlCommandBufferHandle();

	/// Add a user command at the end of the command buffer
	void pushBackUserCommand(UserCallback callback, void* data);

	/// Add another command buffer for execution
	void pushBackOtherCommandBuffer(GlCommandBufferHandle& commands);

	/// Flush command buffer for deffered deletion
	void flush();

	/// Flush and wait to finish
	void finish();

	/// Compute initialization hints
	GlCommandBufferInitHints computeInitHints() const
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
	/// @param first The unit where the first texture will be bound
	/// @param textures A list of textures to bind
	void bindTextures(U32 first, 
		const std::initializer_list<GlTextureHandle>& textures);
	/// @}

	/// @name Drawcalls
	/// @{
	void drawElements(GLenum mode, U8 indexSize, 
		U32 count, U32 instanceCount = 1, U32 firstIndex = 0,
		U32 baseVertex = 0, U32 baseInstance = 0);

	void drawArrays(GLenum mode, U32 count, U32 instanceCount = 1,
		U32 first = 0, U32 baseInstance = 0);
	/// @}

	/// @privatesection
	/// @{
	GlCommandBufferAllocator<U8> _getAllocator() const
	{
		return _get().getAllocator();
	}

	GlGlobalHeapAllocator<U8> _getGlobalAllocator() const
	{
		return _get().getGlobalAllocator();
	}

	GlQueue& _getQueue()
	{
		return _get().getQueue();
	}

	const GlQueue& _getQueue() const
	{
		return _get().getQueue();
	}

	/// Add a new command to the list
	template<typename TCommand, typename... TArgs>
	void _pushBackNewCommand(TArgs&&... args)
	{
		_get().template pushBackNewCommand<TCommand>(
			std::forward<TArgs>(args)...);
	}

	/// Execute all commands
	void _executeAllCommands()
	{
		_get().executeAllCommands();
	}
	/// @}
};

/// @}

} // end namespace anki

#endif

