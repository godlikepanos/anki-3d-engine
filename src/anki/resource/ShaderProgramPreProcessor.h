// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/Common.h>
#include <anki/util/StringList.h>
#include <anki/util/WeakArray.h>
#include <anki/gr/Common.h>

namespace anki
{

// Forward
class ShaderProgramPreprocessor;

/// @addtogroup resource
/// @{

/// @memberof ShaderProgramPreprocessor
class ShaderProgramPreprocessorMutator
{
	friend ShaderProgramPreprocessor;

public:
	using ValueType = I32;

	ShaderProgramPreprocessorMutator(GenericMemoryPoolAllocator<U8> alloc)
		: m_name(alloc)
		, m_values(alloc)
	{
	}

	CString getName() const
	{
		return m_name.toCString();
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
	Bool8 m_instanced = false;
};

/// @memberof ShaderProgramPreprocessor
class ShaderProgramPreprocessorInput
{
	friend ShaderProgramPreprocessor;

public:
	ShaderProgramPreprocessorInput(GenericMemoryPoolAllocator<U8> alloc)
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

	CString getPreprocessorCondition() const
	{
		return (m_preproc) ? m_preproc.toCString() : CString();
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
	Bool8 m_const = false;
	Bool8 m_instanced = false;
};

/// This is a special preprocessor that run before the usual preprocessor. Its purpose is to add some meta information
/// in the shader programs.
///
/// It supports the following expressions
/// #include {<> | ""}
/// #pragma once
/// #pragma anki mutator [instanced] NAME VALUE0 [VALUE1 [VALUE2] ...]
/// #pragma anki input [const | instanced] TYPE NAME ["preprocessor expression"]
/// #pragma anki start {vert | tessc | tesse | geom | frag | comp}
/// #pragma anki end
/// #pragma anki descriptor_set <number>
class ShaderProgramPreprocessor : public NonCopyable
{
public:
	ShaderProgramPreprocessor(CString fname, ResourceFilesystem* fsystem, GenericMemoryPoolAllocator<U8> alloc)
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

	~ShaderProgramPreprocessor()
	{
	}

	ANKI_USE_RESULT Error parse();

	CString getSource() const
	{
		ANKI_ASSERT(!m_finalSource.isEmpty());
		return m_finalSource.toCString();
	}

	ConstWeakArray<ShaderProgramPreprocessorMutator> getMutators() const
	{
		return m_mutators;
	}

	ConstWeakArray<ShaderProgramPreprocessorInput> getInputs() const
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
	using Mutator = ShaderProgramPreprocessorMutator;
	using Input = ShaderProgramPreprocessorInput;

	static const U32 MAX_INCLUDE_DEPTH = 8;

	GenericMemoryPoolAllocator<U8> m_alloc;
	StringAuto m_fname;
	ResourceFilesystem* m_fsystem = nullptr;

	StringListAuto m_lines; ///< The code.
	StringListAuto m_globalsLines;
	StringListAuto m_uboStructLines;
	StringAuto m_finalSource;

	DynamicArrayAuto<Mutator> m_mutators;
	DynamicArrayAuto<Input> m_inputs;

	ShaderTypeBit m_shaderTypes = ShaderTypeBit::NONE;
	Bool8 m_insideShader = false;
	U32 m_set = 0;
	U32 m_instancedMutatorIdx = MAX_U32;
	Bool8 m_foundInstancedInput = false;

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
