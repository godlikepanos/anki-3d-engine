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
class ShaderProgramPrePreprocessor;

/// @addtogroup resource
/// @{

/// @memberof ShaderProgramPrePreprocessor
class ShaderProgramPrePreprocessorMutator
{
	friend ShaderProgramPrePreprocessor;

public:
	ShaderProgramPrePreprocessorMutator(GenericMemoryPoolAllocator<U8> alloc)
		: m_name(alloc)
		, m_values(alloc)
	{
	}

	CString getName() const
	{
		return m_name.toCString();
	}

	ConstWeakArray<U32> getValues() const
	{
		return m_values;
	}

private:
	StringAuto m_name;
	DynamicArrayAuto<U32> m_values;
	Bool8 m_instanced = false;
	Bool8 m_const = false;
};

/// @memberof ShaderProgramPrePreprocessor
class ShaderProgramPrePreprocessorInput
{
	friend ShaderProgramPrePreprocessor;

public:
	ShaderProgramPrePreprocessorInput(GenericMemoryPoolAllocator<U8> alloc)
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
		return m_preproc.toCString();
	}

private:
	StringAuto m_name;
	StringAuto m_preproc;
	ShaderVariableDataType m_dataType = ShaderVariableDataType::NONE;
	Bool8 m_consts = false;
	Bool8 m_instanced = false;
	U8 m_texBinding = MAX_U8;
};

/// XXX
/// #include {<> | ""}
/// #pragma once
/// #pragma anki mutator [instanced] NAME VALUE0 [VALUE1 [VALUE2] ...]
/// #pragma anki input [const | instanced] TYPE NAME ["preprocessor expression"]
/// #pragma anki start {vert | tessc | tesse | geom | frag | comp}
/// #pragma anki end
class ShaderProgramPrePreprocessor : public NonCopyable
{
public:
	ShaderProgramPrePreprocessor(CString fname, ResourceFilesystem* fsystem, GenericMemoryPoolAllocator<U8> alloc);

	~ShaderProgramPrePreprocessor();

	ANKI_USE_RESULT Error parse();

private:
	using Mutator = ShaderProgramPrePreprocessorMutator;
	using Input = ShaderProgramPrePreprocessorInput;

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

	ShaderTypeBit m_shaderTypes;
	Bool8 m_insideShader = false;
	U32 m_lastTexBinding = 0;
	U32 m_set = 0; // TODO
	Bool8 m_foundInstancedMutator = false;
	Bool8 m_foundInstancedInput = false;

	ANKI_USE_RESULT Error parseFile(CString fname, U32 depth);
	ANKI_USE_RESULT Error parseLine(CString line, CString fname, Bool& foundPragmaOnce, U32 depth);
	ANKI_USE_RESULT Error parsePragmaMutator(
		const StringAuto* begin, const StringAuto* end, CString line, CString fname);
	ANKI_USE_RESULT Error parsePragmaInput(const StringAuto* begin, const StringAuto* end, CString line, CString fname);
	ANKI_USE_RESULT Error parsePragmaStart(const StringAuto* begin, const StringAuto* end, CString line, CString fname);
	ANKI_USE_RESULT Error parsePragmaEnd(const StringAuto* begin, const StringAuto* end, CString line, CString fname);

	void tokenizeLine(CString line, DynamicArrayAuto<StringAuto>& tokens);
};
/// @}

} // end namespace anki
