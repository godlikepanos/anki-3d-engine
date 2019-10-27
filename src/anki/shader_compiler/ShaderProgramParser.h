// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/StringList.h>
#include <anki/util/WeakArray.h>
#include <anki/util/DynamicArray.h>
#include <anki/util/BitSet.h>
#include <anki/gr/Common.h>

namespace anki
{

// Forward
class ShaderProgramParser;
class ShaderProgramParserOutput;

/// @addtogroup resource
/// @{

/// @memberof ShaderProgramParser
class ShaderProgramParserMutator
{
	friend ShaderProgramParser;

public:
	using ValueType = I32;

	ShaderProgramParserMutator(GenericMemoryPoolAllocator<U8> alloc)
		: m_name(alloc)
		, m_values(alloc)
	{
	}

	CString getName() const
	{
		return m_name;
	}

	ConstWeakArray<I32> getValues() const
	{
		return m_values;
	}

	Bool isInstanced() const
	{
		return m_instanced;
	}

private:
	StringAuto m_name;
	DynamicArrayAuto<I32> m_values;
	Bool m_instanced = false;
};

/// @memberof ShaderProgramParser
class ShaderProgramParserInput
{
	friend ShaderProgramParser;
	friend ShaderProgramParserOutput;

public:
	ShaderProgramParserInput(GenericMemoryPoolAllocator<U8> alloc)
		: m_name(alloc)
		, m_preproc(alloc)
	{
	}

	CString getName() const
	{
		return m_name.toCString();
	}

	ShaderVariableDataType getDataType() const
	{
		return m_dataType;
	}

	Bool isInstanced() const
	{
		return m_instanced;
	}

	Bool isConstant() const
	{
		return m_const;
	}

private:
	StringAuto m_name;
	StringAuto m_preproc;
	ShaderVariableDataType m_dataType = ShaderVariableDataType::NONE;
	U32 m_idx = MAX_U32; ///< Index inside an array.
	Bool m_const = false;
	Bool m_instanced = false;
};

/// @memberof ShaderProgramParser
class ShaderProgramParserOutput
{
public:
	ShaderProgramParserOutput(GenericMemoryPoolAllocator<U8> alloc)
		: m_source(alloc)
	{
	}

	CString getSource() const
	{
		return m_source;
	}

	Bool inputIsActive(const ShaderProgramParserInput& in)
	{
		return m_activeInputMask.get(in.m_idx);
	}

private:
	StringAuto m_source;
	BitSet<128> m_activeInputMask = {false};
};

/// @memberof ShaderProgramParser
class ShaderProgramParserMutatorState
{
public:
	const ShaderProgramParserMutator* m_mutator = nullptr;
	ShaderProgramParserMutator::ValueType m_value = MAX_I32;
};

/// An interface used by the ShaderProgramParser to abstract file loading.
/// @memberof ShaderProgramParser
class ShaderProgramParserFilesystemInterface
{
public:
	ANKI_USE_RESULT virtual Error readAllText(CString filename, StringAuto& txt) = 0;
};

/// This is a special preprocessor that run before the usual preprocessor. Its purpose is to add some meta information
/// in the shader programs.
///
/// It supports the following expressions:
/// #include {<> | ""}
/// #pragma once
/// #pragma anki mutator [instanced] NAME VALUE0 [VALUE1 [VALUE2] ...]
/// #pragma anki input [const | instanced] TYPE NAME
/// #pragma anki start {vert | tessc | tesse | geom | frag | comp}
/// #pragma anki end
/// #pragma anki descriptor_set <number>
///
/// Only the "anki input" should be in an ifdef-like guard.
class ShaderProgramParser : public NonCopyable
{
public:
	ShaderProgramParser(
		CString fname, ShaderProgramParserFilesystemInterface* fsystem, GenericMemoryPoolAllocator<U8> alloc)
		: m_alloc(alloc)
		, m_fname(alloc, fname)
		, m_fsystem(fsystem)
		, m_lines(alloc)
		, m_globalsLines(alloc)
		, m_uboStructLines(alloc)
		, m_finalSource(alloc)
		, m_mutators(alloc)
		, m_inputs(alloc)
	{
	}

	~ShaderProgramParser()
	{
	}

	/// Parse the file and its includes.
	ANKI_USE_RESULT Error parse();

	/// Get the source (and a few more things) given a list of mutators.
	ShaderProgramParserOutput generateSource(ConstWeakArray<ShaderProgramParserMutatorState> mutatorStates) const;

	ConstWeakArray<ShaderProgramParserMutator> getMutators() const
	{
		return m_mutators;
	}

	ConstWeakArray<ShaderProgramParserInput> getInputs() const
	{
		return m_inputs;
	}

	ShaderTypeBit getShaderStages() const
	{
		return m_shaderTypes;
	}

	U32 getDescritproSet() const
	{
		return m_set;
	}

private:
	using Mutator = ShaderProgramParserMutator;
	using Input = ShaderProgramParserInput;

	static const U32 MAX_INCLUDE_DEPTH = 8;

	GenericMemoryPoolAllocator<U8> m_alloc;
	StringAuto m_fname;
	ShaderProgramParserFilesystemInterface* m_fsystem = nullptr;

	StringListAuto m_lines; ///< The code.
	StringListAuto m_globalsLines;
	StringListAuto m_uboStructLines;
	StringAuto m_finalSource;

	DynamicArrayAuto<Mutator> m_mutators;
	DynamicArrayAuto<Input> m_inputs;

	ShaderTypeBit m_shaderTypes = ShaderTypeBit::NONE;
	Bool m_insideShader = false;
	U32 m_set = 0;
	U32 m_instancedMutatorIdx = MAX_U32;
	Bool m_foundInstancedInput = false; // TODO need that?

	ANKI_USE_RESULT Error parseFile(CString fname, U32 depth);
	ANKI_USE_RESULT Error parseLine(CString line, CString fname, Bool& foundPragmaOnce, U32 depth);
	ANKI_USE_RESULT Error parseInclude(
		const StringAuto* begin, const StringAuto* end, CString line, CString fname, U32 depth);
	ANKI_USE_RESULT Error parsePragmaMutator(
		const StringAuto* begin, const StringAuto* end, CString line, CString fname);
	ANKI_USE_RESULT Error parsePragmaInput(const StringAuto* begin, const StringAuto* end, CString line, CString fname);
	ANKI_USE_RESULT Error parsePragmaStart(const StringAuto* begin, const StringAuto* end, CString line, CString fname);
	ANKI_USE_RESULT Error parsePragmaEnd(const StringAuto* begin, const StringAuto* end, CString line, CString fname);
	ANKI_USE_RESULT Error parsePragmaDescriptorSet(
		const StringAuto* begin, const StringAuto* end, CString line, CString fname);

	void tokenizeLine(CString line, DynamicArrayAuto<StringAuto>& tokens);

	static Bool tokenIsComment(CString token)
	{
		return token.getLength() >= 2 && token[0] == '/' && (token[1] == '/' || token[1] == '*');
	}
};
/// @}

} // end namespace anki
