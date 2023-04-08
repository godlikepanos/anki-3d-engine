// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/ShaderCompiler/Common.h>
#include <AnKi/Util/StringList.h>
#include <AnKi/Util/WeakArray.h>
#include <AnKi/Util/DynamicArray.h>

namespace anki {

// Forward
class ShaderProgramParser;
class ShaderProgramParserVariant;

/// @addtogroup shader_compiler
/// @{

/// @memberof ShaderProgramParser
class ShaderProgramParserMutator
{
	friend ShaderProgramParser;

public:
	CString getName() const
	{
		return m_name;
	}

	ConstWeakArray<MutatorValue> getValues() const
	{
		return m_values;
	}

private:
	String m_name;
	DynamicArray<MutatorValue> m_values;
};

/// @memberof ShaderProgramParser
class ShaderProgramParserMember
{
public:
	String m_name;
	ShaderVariableDataType m_type;
	U32 m_dependentMutator = kMaxU32;
	MutatorValue m_mutatorValue = 0;
};

/// @memberof ShaderProgramParser
class ShaderProgramParserGhostStruct
{
public:
	DynamicArray<ShaderProgramParserMember> m_members;
	String m_name;
};

/// @memberof ShaderProgramParser
class ShaderProgramParserVariant
{
	friend class ShaderProgramParser;

public:
	CString getSource(ShaderType type) const
	{
		return m_sources[type];
	}

private:
	Array<String, U32(ShaderType::kCount)> m_sources;
};

/// This is a special preprocessor that run before the usual preprocessor. Its purpose is to add some meta information
/// in the shader programs.
///
/// It supports the following expressions:
/// #include {<> | ""}
/// #pragma once
/// #pragma anki mutator NAME VALUE0 [VALUE1 [VALUE2] ...]
/// #pragma anki start {vert | tessc | tesse | geom | frag | comp | rgen | ahit | chit | miss | int | call}
/// #pragma anki end
/// #pragma anki library "name"
/// #pragma anki ray_type NUMBER
/// #pragma anki reflect NAME
/// #pragma anki skip_mutation MUTATOR0 VALUE0 MUTATOR1 VALUE1 [MUTATOR2 VALUE2 ...]
/// #pragma anki 16bit // Works only in HLSL. Gain 16bit types but loose min16xxx types
///
/// #pragma anki struct NAME
/// #	pragma anki member [ANKI_RP] TYPE NAME [if MUTATOR_NAME is MUTATOR_VALUE]
/// 	...
/// #pragma anki struct end
///
/// None of the pragmas should be in an ifdef-like guard. It's ignored.
class ShaderProgramParser
{
public:
	ShaderProgramParser(CString fname, ShaderProgramFilesystemInterface* fsystem,
						const ShaderCompilerOptions& compilerOptions);

	ShaderProgramParser(const ShaderProgramParser&) = delete; // Non-copyable

	~ShaderProgramParser();

	ShaderProgramParser& operator=(const ShaderProgramParser&) = delete; // Non-copyable

	/// Parse the file and its includes.
	Error parse();

	/// Returns true if the mutation should be skipped.
	Bool skipMutation(ConstWeakArray<MutatorValue> mutation) const;

	/// Get the source (and a few more things) given a list of mutators.
	Error generateVariant(ConstWeakArray<MutatorValue> mutation, ShaderProgramParserVariant& variant) const;

	ConstWeakArray<ShaderProgramParserMutator> getMutators() const
	{
		return m_mutators;
	}

	ShaderTypeBit getShaderTypes() const
	{
		return m_shaderTypes;
	}

	U64 getHash() const
	{
		ANKI_ASSERT(m_codeSourceHash != 0);
		return m_codeSourceHash;
	}

	CString getLibraryName() const
	{
		return m_libName;
	}

	U32 getRayType() const
	{
		return m_rayType;
	}

	const StringList& getSymbolsToReflect() const
	{
		return m_symbolsToReflect;
	}

	ConstWeakArray<ShaderProgramParserGhostStruct> getGhostStructs() const
	{
		return m_ghostStructs;
	}

	Bool compileWith16bitTypes() const
	{
		return m_16bitTypes;
	}

	/// Generates the common header that will be used by all AnKi shaders.
	static void generateAnkiShaderHeader(ShaderType shaderType, const ShaderCompilerOptions& compilerOptions,
										 String& header);

private:
	using Mutator = ShaderProgramParserMutator;
	using Member = ShaderProgramParserMember;
	using GhostStruct = ShaderProgramParserGhostStruct;

	class PartialMutationSkip
	{
	public:
		DynamicArray<MutatorValue> m_partialMutation;
	};

	static constexpr U32 kMaxIncludeDepth = 8;

	String m_fname;
	ShaderProgramFilesystemInterface* m_fsystem = nullptr;

	StringList m_codeLines; ///< The code.
	String m_codeSource;
	U64 m_codeSourceHash = 0;

	DynamicArray<Mutator> m_mutators;
	DynamicArray<PartialMutationSkip> m_skipMutations;

	ShaderTypeBit m_shaderTypes = ShaderTypeBit::kNone;
	Bool m_insideShader = false;
	ShaderCompilerOptions m_compilerOptions;

	String m_libName;
	U32 m_rayType = kMaxU32;

	StringList m_symbolsToReflect;

	DynamicArray<GhostStruct> m_ghostStructs;
	Bool m_insideStruct = false;

	Bool m_16bitTypes = false;

	Error parseFile(CString fname, U32 depth);
	Error parseLine(CString line, CString fname, Bool& foundPragmaOnce, U32 depth, U32 lineNumber);
	Error parseInclude(const String* begin, const String* end, CString line, CString fname, U32 depth);
	Error parsePragmaMutator(const String* begin, const String* end, CString line, CString fname);
	Error parsePragmaStart(const String* begin, const String* end, CString line, CString fname);
	Error parsePragmaEnd(const String* begin, const String* end, CString line, CString fname);
	Error parsePragmaSkipMutation(const String* begin, const String* end, CString line, CString fname);
	Error parsePragmaLibraryName(const String* begin, const String* end, CString line, CString fname);
	Error parsePragmaRayType(const String* begin, const String* end, CString line, CString fname);
	Error parsePragmaReflect(const String* begin, const String* end, CString line, CString fname);
	Error parsePragmaStructBegin(const String* begin, const String* end, CString line, CString fname);
	Error parsePragmaStructEnd(const String* begin, const String* end, CString line, CString fname);
	Error parsePragmaMember(const String* begin, const String* end, CString line, CString fname);
	Error parsePragma16bit(const String* begin, const String* end, CString line, CString fname);

	void tokenizeLine(CString line, DynamicArray<String>& tokens) const;

	static Bool tokenIsComment(CString token)
	{
		return token.getLength() >= 2 && token[0] == '/' && (token[1] == '/' || token[1] == '*');
	}

	static Bool mutatorHasValue(const ShaderProgramParserMutator& mutator, MutatorValue value);

	Error checkNoActiveStruct() const
	{
		if(m_insideStruct)
		{
			ANKI_SHADER_COMPILER_LOGE("Unsupported \"pragma anki\" inside \"pragma anki struct\"");
			return Error::kUserData;
		}
		return Error::kNone;
	}

	Error checkActiveStruct() const
	{
		if(!m_insideStruct)
		{
			ANKI_SHADER_COMPILER_LOGE("Expected a \"pragma anki struct\" to open");
			return Error::kUserData;
		}
		return Error::kNone;
	}
};
/// @}

} // end namespace anki
