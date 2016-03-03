// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/gl/Common.h>
#include <anki/util/DArray.h>

namespace anki
{

// Forward
class ConfigSet;

/// @addtogroup opengl
/// @{

/// Knowing the ventor allows some optimizations
enum class GpuVendor : U8
{
	UNKNOWN,
	ARM,
	NVIDIA
};

/// Part of the global state. It's essentialy a cache of the state mainly used
/// for optimizations and other stuff
class GlState
{
public:
	// Spec limits
	static const U MAX_UNIFORM_BLOCK_SIZE = 16384;
	static const U MAX_STORAGE_BLOCK_SIZE = 2 << 27;

	I32 m_version = -1; ///< Minor major GL version. Something like 430
	GpuVendor m_gpu = GpuVendor::UNKNOWN;
	Bool8 m_registerMessages = false;

	GLuint m_defaultVao;

	/// @name Cached state
	/// @{
	Array<U16, 4> m_viewport = {{0, 0, 0, 0}};

	GLenum m_blendSfunc = GL_ONE;
	GLenum m_blendDfunc = GL_ZERO;

	GLuint m_crntPpline = 0;
	/// @}

	/// @name Pipeline state
	/// @{
	Array<GLsizei, MAX_VERTEX_ATTRIBUTES> m_vertexBindingStrides;
	GLenum m_topology = 0;
	U8 m_indexSize = 4;

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
	} m_stateHashes;

	Array2d<Bool, MAX_COLOR_ATTACHMENTS, 4> m_colorWriteMasks = {
		{{{true, true, true, true}},
			{{true, true, true, true}},
			{{true, true, true, true}},
			{{true, true, true, true}}}};
	Bool m_depthWriteMask = true;
	/// @}

	/// Ring buffers
	/// @{

	// Ring buffer for dynamic buffers.
	class DynamicBuffer
	{
	public:
		GLuint m_name = 0;
		U8* m_address = nullptr; ///< Host address of the buffer.
		PtrSize m_size = 0; ///< This is aligned compared to GLOBAL_XXX_SIZE
		U32 m_alignment = 0; ///< Always work in that alignment.
		U32 m_maxAllocationSize = 0; ///< For debugging.
		Atomic<PtrSize> m_currentOffset = {0};
		Atomic<PtrSize> m_bytesUsed = {0}; ///< Per frame. For debugging.
	};

	Array<DynamicBuffer, U(BufferUsage::COUNT)> m_dynamicBuffers;
	/// @}

	GlState(GrManager* manager)
		: m_manager(manager)
	{
	}

	/// Call this from the main thread.
	void init0(const ConfigSet& config);

	/// Call this from the rendering thread.
	void init1();

	/// Call this from the rendering thread.
	void destroy();

	/// Allocate memory for a dynamic buffer.
	void* allocateDynamicMemory(
		PtrSize size, BufferUsage usage, DynamicBufferToken& token);

	void checkDynamicMemoryConsumption();

private:
	GrManager* m_manager;

	class alignas(16) Aligned16Type
	{
		U8 _m_val[16] ANKI_UNUSED;
	};

	DArray<Aligned16Type> m_transferBuffer;

	void initDynamicBuffer(
		GLenum target, U32 aligment, U32 maxAllocationSize, BufferUsage usage);
};

//==============================================================================
inline void* GlState::allocateDynamicMemory(
	PtrSize originalSize, BufferUsage usage, DynamicBufferToken& token)
{
	ANKI_ASSERT(originalSize > 0);

	DynamicBuffer& buff = m_dynamicBuffers[usage];
	ANKI_ASSERT(buff.m_address);

	// Align size
	PtrSize size = getAlignedRoundUp(buff.m_alignment, originalSize);
	ANKI_ASSERT(size <= buff.m_maxAllocationSize && "Too high!");

	// Allocate
	PtrSize offset = 0;

	// Run in loop in case the begining of the range falls inside but the end
	// outside the buffer
	do
	{
		offset = buff.m_currentOffset.fetchAdd(size);
		offset = offset % buff.m_size;
	} while((offset + size) > buff.m_size);

	ANKI_ASSERT(isAligned(buff.m_alignment, buff.m_address + offset));
	ANKI_ASSERT((offset + size) <= buff.m_size);

	buff.m_bytesUsed.fetchAdd(size);

	// Encode token
	token.m_offset = offset;
	token.m_range = originalSize;

	return static_cast<void*>(buff.m_address + offset);
}
/// @}

} // end namespace anki
