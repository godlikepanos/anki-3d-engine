// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_COMMAND_BUFFER_HANDLE_H
#define ANKI_GR_COMMAND_BUFFER_HANDLE_H

#include "anki/gr/GrHandle.h"

namespace anki {

/// @addtogroup graphics
/// @{

/// Command buffer handle
class CommandBufferHandle: public GrHandle<CommandBufferImpl>
{
public:
	using Base = GrHandle<CommandBufferImpl>;
	using UserCallback = Error(*)(void*);

	CommandBufferHandle();

	~CommandBufferHandle();

	/// Create command buffer
	ANKI_USE_RESULT Error create(GrManager* manager, 
		CommandBufferInitHints hints = CommandBufferInitHints());

	/// Add a user command at the end of the command buffer
	void pushBackUserCommand(UserCallback callback, void* data);

	/// Add another command buffer for execution
	void pushBackOtherCommandBuffer(CommandBufferHandle& commands);

	/// Flush command buffer for deffered execution.
	void flush();

	/// Flush and wait to finish
	void finish();

	/// Compute initialization hints
	CommandBufferInitHints computeInitHints() const;

	/// @name State manipulation
	/// @{

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

	/// Set polygon mode
	void setPolygonMode(GLenum face, GLenum mode);

	/// Enable/diable point size in vertex/geometry shaders.
	void enablePointSize(Bool enable);

	/// Bind many textures
	/// @param first The unit where the first texture will be bound.
	/// @param textures The array of textures.
	/// @param count The count of textures
	void bindTextures(U32 first, TextureHandle textures[], U32 count);
	/// @}

	/// @name Drawcalls
	/// @{
	void drawElements(GLenum mode, U8 indexSize, 
		U32 count, U32 instanceCount = 1, U32 firstIndex = 0,
		U32 baseVertex = 0, U32 baseInstance = 0);

	void drawArrays(GLenum mode, U32 count, U32 instanceCount = 1,
		U32 first = 0, U32 baseInstance = 0);

	void drawElementsConditional(OcclusionQueryHandle& query, 
		GLenum mode, U8 indexSize, 
		U32 count, U32 instanceCount = 1, U32 firstIndex = 0,
		U32 baseVertex = 0, U32 baseInstance = 0);

	void drawArraysConditional(OcclusionQueryHandle& query,
		GLenum mode, U32 count, U32 instanceCount = 1,
		U32 first = 0, U32 baseInstance = 0);

	void dispatchCompute(U32 groupCountX, U32 groupCountY, U32 groupCountZ);
	/// @}

	/// @name Other operations
	/// @{
	void copyTextureToBuffer(TextureHandle& from, BufferHandle& To);
	/// @}
};
/// @}

} // end namespace anki

#endif

