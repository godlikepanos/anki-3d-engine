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

/// All program data in a struct to ease moving it
class GlProgramData: public NonCopyable
{
public:
	template<typename T>
	using ProgramVector = Vector<T, GlGlobalHeapAllocator<T>>;

	template<typename T>
	using ProgramDictionary = 
		Dictionary<T, GlGlobalHeapAllocator<std::pair<CString, T>>>;

	GlProgramData(const GlGlobalHeapAllocator<U8>& alloc)
	:	m_variables(alloc), 
		m_variablesDict(10, DictionaryHasher(), DictionaryEqual(), alloc), 
		m_blocks(alloc), 
		m_blocksDict(10, DictionaryHasher(), DictionaryEqual(), alloc),
		m_names(nullptr),
		m_prog(nullptr)
	{}

	~GlProgramData() noexcept
	{
		if(m_names)
		{
			auto alloc = m_variables.get_allocator();
			alloc.deallocate(m_names, 0);
		}
	}

	ProgramVector<GlProgramVariable> m_variables;
	ProgramDictionary<GlProgramVariable*> m_variablesDict;

	ProgramVector<GlProgramBlock> m_blocks;
	ProgramDictionary<GlProgramBlock*> m_blocksDict;

	/// Keep all the names of blocks and variables in a single place
	char* m_names;

	/// As always keep the father
	GlProgram* m_prog;
};

/// Shader program. It only contains a single shader and it can be combined 
/// with other programs in a program pipiline.
class GlProgram: public GlObject
{
	friend class GlProgramVariable;
	friend class GlProgramBlock;

public:
	using Base = GlObject;

	/// @name Constructors/Destructor
	/// @{
	GlProgram()
	:	m_data(nullptr)
	{}

	GlProgram(GlProgram&& b)
	{
		*this = std::move(b);
	}

	/// Create the program
	/// @param shaderType The type of the shader in the program
	/// @param source The shader's source
	/// @param alloc The allocator to be used for internally
	GlProgram(
		GLenum shaderType, 
		const CString& source, 
		const GlGlobalHeapAllocator<U8>& alloc, 
		const CString& cacheDir)
	:	m_data(nullptr)
	{
		create(shaderType, source, alloc, cacheDir);
	}

	~GlProgram()
	{
		destroy();
	}
	/// @}

	/// Move
	GlProgram& operator=(GlProgram&& b);

	GLenum getType() const
	{
		ANKI_ASSERT(isCreated() && m_data);
		return m_type;
	}

	const GlProgramData::ProgramVector<GlProgramVariable>& getVariables() const
	{
		ANKI_ASSERT(isCreated() && m_data);
		return m_data->m_variables;
	}

	const GlProgramData::ProgramVector<GlProgramBlock>& getBlocks() const
	{
		ANKI_ASSERT(isCreated() && m_data);
		return m_data->m_blocks;
	}

	const GlProgramVariable& findVariable(const CString& name) const;
	const GlProgramBlock& findBlock(const CString& name) const;

	const GlProgramVariable* tryFindVariable(const CString& name) const;
	const GlProgramBlock* tryFindBlock(const CString& name) const;

private:
	GLenum m_type;
	GlProgramData* m_data;

	void create(GLenum type, const CString& source, 
		const GlGlobalHeapAllocator<U8>& alloc, const CString& cacheDir);
	void createInternal(GLenum type, const CString& source, 
		const GlGlobalHeapAllocator<U8>& alloc, const CString& cacheDir);
	void destroy();

	/// Query the program for blocks
	void initBlocksOfType(GLenum programInterface, U count, U index,
		char*& namesPtr, U& namesLen);

	/// Query the program for variables
	void initVariablesOfType(
		GLenum programInterface, U count, U index, U blkIndex,
		char*& namesPtr, U& namesLen);
};

/// @}

} // end namespace anki

#endif

