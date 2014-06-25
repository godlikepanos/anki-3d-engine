// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_COMMON_H
#define ANKI_GL_COMMON_H

#include "anki/Config.h"
#include "anki/util/Allocator.h"
#include "anki/util/NonCopyable.h"
#include "anki/util/Assert.h"
#include "anki/util/Array.h"
#include "anki/util/StdTypes.h"
#include "anki/Math.h"

#include <atomic>
#include <string>

#if ANKI_GL == ANKI_GL_DESKTOP
#	if ANKI_OS == ANKI_OS_WINDOWS && !defined(GLEW_STATIC)
#		define GLEW_STATIC
#	endif
#	include <GL/glew.h>
#	if !defined(ANKI_GLEW_H)
#		error "Wrong GLEW included"
#	endif
#elif ANKI_GL == ANKI_GL_ES
#	include <GLES3/gl3.h>
#else
#	error "See file"
#endif

#define ANKI_JOB_MANAGER_DISABLE_ASYNC 0

namespace anki {

// Forward
class GlProgramData;
class GlProgramBlock;

/// @addtogroup opengl_private
/// @{

/// The type of the allocator of GlJobChain
template<typename T>
using GlJobChainAllocator = ChainAllocator<T>;

/// The type of the allocator for heap allocations
template<typename T>
using GlGlobalHeapAllocator = HeapAllocator<T>;

/// Texture filtering method
enum class GlTextureFilter: U8
{
	NEAREST,
	LINEAR,
	TRILINEAR
};

/// Split the initializer for re-using parts of it
class GlTextureInitializerBase
{
public:
	U32 m_width = 0;
	U32 m_height = 0;
	U32 m_depth = 0; ///< Relevant only for 3D and 2DArray textures
	GLenum m_target = GL_TEXTURE_2D;
	GLenum m_internalFormat = GL_NONE;
	GLenum m_format = GL_NONE;
	GLenum m_type = GL_NONE;
	U32 m_mipmapsCount = 0;
	GlTextureFilter m_filterType = GlTextureFilter::NEAREST;
	Bool8 m_repeat = false;
	I32 m_anisotropyLevel = 0;
	Bool8 m_genMipmaps = false;
	U32 m_samples = 1;
};

/// @}

/// @addtogroup opengl_other

/// A function that returns an index from a shader type GLenum
U computeShaderTypeIndex(const GLenum glType);

/// A function that returns a GLenum from an index
GLenum computeGlShaderType(const U idx, GLbitfield* bit = nullptr);

/// GL generic callback
using GlCallback = void(*)(void*);

/// @}

/// @addtogroup opengl_containers
/// @{

/// Shader program variable. The type is attribute or uniform
class GlProgramVariable
{
	friend class GlProgram;

public:
	/// Shader program variable type
	enum class Type: U8
	{
		INPUT, ///< Attribute on vertex
		UNIFORM,
		BUFFER
	};

	/// @name Accessors
	/// @{
	Type getType() const
	{
		return m_type;
	}

	const char* getName() const
	{
		return &m_name[0];
	}

	GLenum getDataType() const
	{
		ANKI_ASSERT(m_dataType != GL_NONE);
		return m_dataType;
	}

	PtrSize getArraySize() const
	{
		ANKI_ASSERT(m_arrSize != 0);
		return m_arrSize;
	}

	GLint getLocation() const
	{
		ANKI_ASSERT(m_loc != -1);
		return m_loc;
	}

	U32 getTextureUnit() const
	{
		ANKI_ASSERT(m_texUnit >= 0);
		return (U32)m_texUnit;
	}

	/// @return The block or nullptr if it doesn't belong to a block
	const GlProgramBlock* getBlock() const;
	/// @}

	/// @name Block setters
	/// Write a client memory that represents the interface block
	/// @{
	void writeClientMemory(void* buffBase, U32 buffSize,
		const F32 arr[], U32 size) const;

	void writeClientMemory(void* buffBase, U32 buffSize,
		const Vec2 arr[], U32 size) const;

	void writeClientMemory(void* buffBase, U32 buffSize,
		const Vec3 arr[], U32 size) const;

	void writeClientMemory(void* buffBase, U32 buffSize,
		const Vec4 arr[], U32 size) const;

	void writeClientMemory(void* buffBase, U32 buffSize,
		const Mat3 arr[], U32 size) const;

	void writeClientMemory(void* buffBase, U32 buffSize,
		const Mat4 arr[], U32 size) const;
	/// @}

private:
	Type m_type; ///< It's type
	char* m_name; ///< The name inside the shader program
	const GlProgramData* m_progData = nullptr; ///< Know your father

	GLenum m_dataType = GL_NONE; ///< GL_FLOAT or GL_FLOAT_VEC2 etc.
	U8 m_arrSize = 0; ///< Its 1 if it is a single or >1 if it is an array

	I32 m_loc = -1; ///< For uniforms and attributes

	/// @name For variables in interface blocks
	/// @{

	/// Stride between the each array element if the variable is array
	I32 m_arrStride = -1;

	I32 m_offset = -1; ///< Offset inside the block

	/// Identifying the stride between columns of a column-major matrix or rows 
	/// of a row-major matrix
	I32 m_matrixStride = -1;

	I16 m_blockIdx = -1; ///< Interface block
	/// @}

	I32 m_texUnit = -1; ///< Explicit unit for samplers

	/// Do common checks
	template<typename T>
	void writeClientMemorySanityChecks(void* buffBase, U32 buffSize,
		const T arr[], U32 size) const;

	/// Do the actual job of setClientMemory
	template<typename T>
	void writeClientMemoryInternal(
		void* buff, U32 buffSize, const T arr[], U32 size) const;

	/// Do the actual job of setClientMemory for matrices
	template<typename Mat, typename Vec>
	void writeClientMemoryInternalMatrix(void* buff, U32 buffSize,
		const Mat arr[], U32 size) const;
};

/// Interface shader block
class GlProgramBlock
{
	friend class GlProgram;

public:
	static const U32 MAX_VARIABLES_PER_BLOCK = 16;

	/// Interface shader block type
	enum class Type: U8
	{
		UNIFORM,
		SHADER_STORAGE
	};

	/// @name Accessors
	/// @{
	Type getType() const
	{
		return m_type;
	}

	PtrSize getSize() const
	{
		return m_size;
	}

	const char* getName() const
	{
		return &m_name[0];
	}

	U32 getBinding() const
	{
		return m_bindingPoint;
	}
	/// @}

private:
	Type m_type;
	char* m_name;
	const GlProgramData* m_progData = nullptr; ///< Know your father
	/// Keep the indices as U16 to save memory
	Array<U16, MAX_VARIABLES_PER_BLOCK> m_variableIdx; 
	U32 m_size; ///< In bytes
	U32 m_bindingPoint;
	U8 m_variablesCount = 0;
};

/// @}

} // end namespace anki

#endif
