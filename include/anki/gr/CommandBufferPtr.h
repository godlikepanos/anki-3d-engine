// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/gr/GrPtr.h"

namespace anki {

/// @addtogroup graphics
/// @{

/// Command buffer.
class CommandBufferPtr: public GrPtr<CommandBufferImpl>
{
public:
	using Base = GrPtr<CommandBufferImpl>;
	using UserCallback = Error(*)(void*);

	CommandBufferPtr();

	~CommandBufferPtr();

	/// Create command buffer
	void create(GrManager* manager,
		CommandBufferInitHints hints = CommandBufferInitHints());

	/// Add a user command at the end of the command buffer
	void pushBackUserCommand(UserCallback callback, void* data);

	/// Add another command buffer for execution
	void pushBackOtherCommandBuffer(CommandBufferPtr& commands);

	/// Flush command buffer for deferred execution.
	void flush();

	/// Flush and wait to finish
	void finish();

	/// Compute initialization hints
	CommandBufferInitHints computeInitHints() const;

	/// Update dynamic uniforms.
	void updateDynamicUniforms(void* data, U32 size);

	/// @name State manipulation
	/// @{

	/// Bind vertex buffer.
	void bindVertexBuffer(U32 bindingPoint, BufferPtr buff, PtrSize offset);

	/// Bind index buffer.
	void bindIndexBuffer(BufferPtr buff, U32 indexSize);

	/// Set the viewport
	void setViewport(U16 minx, U16 miny, U16 maxx, U16 maxy);

	/// Bind many textures
	/// @param first The unit where the first texture will be bound.
	/// @param textures The array of textures.
	/// @param count The count of textures
	void bindTextures(U32 first, TexturePtr textures[], U32 count);
	/// @}

	/// @name Drawcalls
	/// @{
	void drawElements(GLenum mode, U8 indexSize,
		U32 count, U32 instanceCount = 1, U32 firstIndex = 0,
		U32 baseVertex = 0, U32 baseInstance = 0);

	void drawArrays(GLenum mode, U32 count, U32 instanceCount = 1,
		U32 first = 0, U32 baseInstance = 0);

	void drawElementsConditional(OcclusionQueryPtr& query,
		GLenum mode, U8 indexSize,
		U32 count, U32 instanceCount = 1, U32 firstIndex = 0,
		U32 baseVertex = 0, U32 baseInstance = 0);

	void drawArraysConditional(OcclusionQueryPtr& query,
		GLenum mode, U32 count, U32 instanceCount = 1,
		U32 first = 0, U32 baseInstance = 0);

	void dispatchCompute(U32 groupCountX, U32 groupCountY, U32 groupCountZ);
	/// @}

	/// @name Other operations
	/// @{
	void copyTextureToBuffer(TexturePtr& from, BufferPtr& To);
	/// @}
};
/// @}

} // end namespace anki

