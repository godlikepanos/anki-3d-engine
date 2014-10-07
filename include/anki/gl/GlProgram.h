// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_PROGRAM_H
#define ANKI_GL_GL_PROGRAM_H

#include "anki/gl/GlObject.h"
#include "anki/util/Dictionary.h"
#include "anki/util/Vector.h"

namespace anki {

// Forward
class GlProgram;

/// @addtogroup opengl_private
/// @{

/// Shader program. It only contains a single shader and it can be combined 
/// with other programs in a program pipiline.
class GlProgram: public GlObject
{
	friend class GlProgramVariable;
	friend class GlProgramBlock;

public:
	using Base = GlObject;

	template<typename T>
	using ProgramVector = Vector<T, GlAllocator<T>>;

	template<typename T>
	using ProgramDictionary = 
		Dictionary<T, GlAllocator<std::pair<CString, T>>>;

	GlProgram() = default;

	~GlProgram()
	{
		destroy();
	}

	/// Create the program
	/// @param shaderType The type of the shader in the program
	/// @param source The shader's source
	/// @param alloc The allocator to be used for internally
	ANKI_USE_RESULT Error create(
		GLenum shaderType, 
		const CString& source, 
		GlAllocator<U8>& alloc, 
		const CString& cacheDir);

	GLenum getType() const
	{
		ANKI_ASSERT(isCreated());
		return m_type;
	}

	const ProgramVector<GlProgramVariable>& getVariables() const
	{
		ANKI_ASSERT(isCreated());
		return m_variables;
	}

	const ProgramVector<GlProgramBlock>& getBlocks() const
	{
		ANKI_ASSERT(isCreated());
		return m_blocks;
	}

	const GlProgramVariable* tryFindVariable(const CString& name) const;
	const GlProgramBlock* tryFindBlock(const CString& name) const;

private:
	GLenum m_type;
	ProgramVector<GlProgramVariable> m_variables;
	ProgramDictionary<GlProgramVariable*> m_variablesDict;

	ProgramVector<GlProgramBlock> m_blocks;
	ProgramDictionary<GlProgramBlock*> m_blocksDict;

	/// Keep all the names of blocks and variables in a single place
	char* m_names = nullptr;

	void destroy();

	/// Query the program for blocks
	ANKI_USE_RESULT Error initBlocksOfType(GLenum programInterface, 
		U count, U index, char*& namesPtr, U& namesLen);

	/// Query the program for variables
	ANKI_USE_RESULT Error initVariablesOfType(
		GLenum programInterface, U count, U index, U blkIndex,
		char*& namesPtr, U& namesLen);

	ANKI_USE_RESULT Error populateVariablesAndBlock(GlAllocator<U8>& alloc);
};

/// @}

} // end namespace anki

#endif

