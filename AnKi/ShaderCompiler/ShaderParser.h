// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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
class ShaderParser;

/// @addtogroup shader_compiler
/// @{

/// @memberof ShaderParser
class ShaderParserMutator
{
public:
	ShaderCompilerString m_name;
	ShaderCompilerDynamicArray<MutatorValue> m_values;
};

/// @memberof ShaderParser
class ShaderParserGhostStructMember
{
public:
	ShaderCompilerString m_name;
	ShaderVariableDataType m_type;
	U32 m_offset = kMaxU32;
};

/// @memberof ShaderParser
class ShaderParserGhostStruct
{
public:
	ShaderCompilerDynamicArray<ShaderParserGhostStructMember> m_members;
	ShaderCompilerString m_name;
};

/// @memberof ShaderParser
class ShaderParserTechnique
{
public:
	ShaderCompilerString m_name;
	ShaderTypeBit m_shaderTypes = ShaderTypeBit::kNone;
	Array<U64, U32(ShaderType::kCount)> m_activeMutators = {};
};

/// This is a special preprocessor that run before the usual preprocessor. Its purpose is to add some meta information
/// in the shader programs.
///
/// It supports the following expressions:
/// #include {<> | ""}
/// #pragma once
/// #pragma anki mutator NAME VALUE0 [VALUE1 [VALUE2 ...]]
/// #pragma anki skip_mutation MUTATOR0 VALUE0 [MUTATOR1 VALUE1 [MUTATOR2 VALUE2 ...]]
/// #pragma anki 16bit // Works only in HLSL. Gain 16bit types but loose min16xxx types
/// #pragma anki technique_start STAGE [NAME] [uses_mutators [USES_MUTATOR1 [USES_MUTATOR2 ...]]]
/// #pragma anki technique_end STAGE [NAME]
///
/// #pragma anki struct NAME
/// #	pragma anki member TYPE NAME
/// 	...
/// #pragma anki struct_end
///
/// None of the pragmas should be in an ifdef-like guard. It's ignored.
class ShaderParser
{
public:
	ShaderParser(CString fname, ShaderCompilerFilesystemInterface* fsystem, ConstWeakArray<ShaderCompilerDefine> defines);

	ShaderParser(const ShaderParser&) = delete; // Non-copyable

	~ShaderParser();

	ShaderParser& operator=(const ShaderParser&) = delete; // Non-copyable

	/// Parse the file and its includes.
	Error parse();

	/// Returns true if the mutation should be skipped.
	Bool skipMutation(ConstWeakArray<MutatorValue> mutation) const;

	/// Get the source (and a few more things) given a list of mutators.
	void generateVariant(ConstWeakArray<MutatorValue> mutation, const ShaderParserTechnique& technique, ShaderType shaderType,
						 ShaderCompilerString& source) const;

	ConstWeakArray<ShaderParserMutator> getMutators() const
	{
		return m_mutators;
	}

	U64 getHash() const
	{
		ANKI_ASSERT(m_hash != 0);
		return m_hash;
	}

	ConstWeakArray<ShaderParserGhostStruct> getGhostStructs() const
	{
		return m_ghostStructs;
	}

	ConstWeakArray<ShaderParserTechnique> getTechniques() const
	{
		return m_techniques;
	}

	Bool compileWith16bitTypes() const
	{
		return m_16bitTypes;
	}

	/// Generates the common header that will be used by all AnKi shaders.
	static void generateAnkiShaderHeader(ShaderType shaderType, ShaderCompilerString& header);

private:
	using Mutator = ShaderParserMutator;
	using Member = ShaderParserGhostStructMember;
	using GhostStruct = ShaderParserGhostStruct;
	using Technique = ShaderParserTechnique;

	class PartialMutationSkip
	{
	public:
		ShaderCompilerDynamicArray<MutatorValue> m_partialMutation;
	};

	class TechniqueExtra
	{
	public:
		Array<ShaderCompilerStringList, U32(ShaderType::kCount)> m_sourceLines;
		Array<ShaderCompilerString, U32(ShaderType::kCount)> m_sources;
	};

	static constexpr U32 kMaxIncludeDepth = 8;

	ShaderCompilerString m_fname;
	ShaderCompilerFilesystemInterface* m_fsystem = nullptr;

	ShaderCompilerDynamicArray<ShaderCompilerString> m_defineNames;
	ShaderCompilerDynamicArray<I32> m_defineValues;

	U64 m_hash = 0;

	ShaderCompilerStringList m_commonSourceLines; ///< Common code until now.

	ShaderCompilerDynamicArray<Technique> m_techniques;
	ShaderCompilerDynamicArray<TechniqueExtra> m_techniqueExtras;
	U32 m_insideTechniqueIdx = kMaxU32;
	ShaderType m_insideTechniqueShaderType = ShaderType::kCount;

	ShaderCompilerDynamicArray<Mutator> m_mutators;
	ShaderCompilerDynamicArray<PartialMutationSkip> m_skipMutations;

	ShaderCompilerDynamicArray<GhostStruct> m_ghostStructs;
	Bool m_insideStruct = false;

	Bool m_16bitTypes = false;

	ShaderCompilerStringList& getAppendSourceList()
	{
		return (insideTechnique()) ? m_techniqueExtras[m_insideTechniqueIdx].m_sourceLines[m_insideTechniqueShaderType] : m_commonSourceLines;
	}

	Bool insideTechnique() const
	{
		return m_insideTechniqueIdx < kMaxU32;
	}

	Error parseFile(CString fname, U32 depth);
	Error parseLine(CString line, CString fname, Bool& foundPragmaOnce, U32 depth, U32 lineNumber);
	Error parseInclude(const ShaderCompilerString* begin, const ShaderCompilerString* end, CString line, CString fname, U32 depth);
	Error parsePragmaMutator(const ShaderCompilerString* begin, const ShaderCompilerString* end, CString line, CString fname);
	Error parsePragmaTechniqueStart(const ShaderCompilerString* begin, const ShaderCompilerString* end, CString line, CString fname);
	Error parsePragmaTechniqueEnd(const ShaderCompilerString* begin, const ShaderCompilerString* end, CString line, CString fname);
	Error parsePragmaSkipMutation(const ShaderCompilerString* begin, const ShaderCompilerString* end, CString line, CString fname);
	Error parsePragmaStructBegin(const ShaderCompilerString* begin, const ShaderCompilerString* end, CString line, CString fname);
	Error parsePragmaStructEnd(const ShaderCompilerString* begin, const ShaderCompilerString* end, CString line, CString fname);
	Error parsePragmaMember(const ShaderCompilerString* begin, const ShaderCompilerString* end, CString line, CString fname);
	Error parsePragma16bit(const ShaderCompilerString* begin, const ShaderCompilerString* end, CString line, CString fname);

	void tokenizeLine(CString line, ShaderCompilerDynamicArray<ShaderCompilerString>& tokens) const;

	static Bool tokenIsComment(CString token)
	{
		return token.getLength() >= 2 && token[0] == '/' && (token[1] == '/' || token[1] == '*');
	}

	static Bool mutatorHasValue(const ShaderParserMutator& mutator, MutatorValue value);

	static ShaderCompilerString sanitizeFilename(CString fname)
	{
		ShaderCompilerString s = fname;
		s.replaceAll("\\", "\\\\");
		return s;
	}

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
