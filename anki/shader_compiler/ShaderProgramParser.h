// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/shader_compiler/Common.h>
#include <anki/util/StringList.h>
#include <anki/util/WeakArray.h>
#include <anki/util/DynamicArray.h>
#include <anki/gr/utils/Functions.h>

namespace anki
{

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
	ShaderProgramParserMutator(GenericMemoryPoolAllocator<U8> alloc)
		: m_name(alloc)
		, m_values(alloc)
	{
	}

	CString getName() const
	{
		return m_name;
	}

	ConstWeakArray<MutatorValue> getValues() const
	{
		return m_values;
	}

private:
	StringAuto m_name;
	DynamicArrayAuto<MutatorValue> m_values;
};

/// @memberof ShaderProgramParser
class ShaderProgramParserVariant
{
	friend class ShaderProgramParser;

public:
	~ShaderProgramParserVariant()
	{
		for(String& s : m_sources)
		{
			s.destroy(m_alloc);
		}
	}

	CString getSource(ShaderType type) const
	{
		return m_sources[type];
	}

private:
	GenericMemoryPoolAllocator<U8> m_alloc;
	Array<String, U32(ShaderType::COUNT)> m_sources;
};

/// This is a special preprocessor that run before the usual preprocessor. Its purpose is to add some meta information
/// in the shader programs.
///
/// It supports the following expressions:
/// #include {<> | ""}
/// #pragma once
/// #pragma anki mutator NAME VALUE0 [VALUE1 [VALUE2] ...]
/// #pragma anki rewrite_mutation NAME_A VALUE0 NAME_B VALUE1 [NAME_C VALUE3...] to
///                               NAME_A VALUE4 NAME_B VALUE5 [NAME_C VALUE6...]
/// #pragma anki start {vert | tessc | tesse | geom | frag | comp | rgen | ahit | chit | miss | int | call}
/// #pragma anki end
/// #pragma anki library "name"
/// #pragma anki ray_type NUMBER
///
/// Only the "anki input" should be in an ifdef-like guard. For everything else it's ignored.
class ShaderProgramParser : public NonCopyable
{
public:
	ShaderProgramParser(CString fname, ShaderProgramFilesystemInterface* fsystem, GenericMemoryPoolAllocator<U8> alloc,
						const GpuDeviceCapabilities& gpuCapabilities, const BindlessLimits& bindlessLimits);

	~ShaderProgramParser();

	/// Parse the file and its includes.
	ANKI_USE_RESULT Error parse();

	/// Given a mutation convert it to something acceptable. This will reduce the variants.
	/// @return true if the mutation was rewritten.
	Bool rewriteMutation(WeakArray<MutatorValue> mutation) const;

	/// Get the source (and a few more things) given a list of mutators.
	ANKI_USE_RESULT Error generateVariant(ConstWeakArray<MutatorValue> mutation,
										  ShaderProgramParserVariant& variant) const;

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

	/// Generates the common header that will be used by all AnKi shaders.
	static void generateAnkiShaderHeader(ShaderType shaderType, const GpuDeviceCapabilities& caps,
										 const BindlessLimits& limits, StringAuto& header);

private:
	using Mutator = ShaderProgramParserMutator;

	class MutationRewrite
	{
	public:
		class Record
		{
		public:
			U32 m_mutatorIndex = MAX_U32;
			MutatorValue m_valueFrom = getMaxNumericLimit<MutatorValue>();
			MutatorValue m_valueTo = getMaxNumericLimit<MutatorValue>();

			Bool operator!=(const Record& b) const
			{
				return !(m_mutatorIndex == b.m_mutatorIndex && m_valueFrom == b.m_valueFrom
						 && m_valueTo == b.m_valueTo);
			}
		};

		DynamicArrayAuto<Record> m_records;

		MutationRewrite(GenericMemoryPoolAllocator<U8> alloc)
			: m_records(alloc)
		{
		}
	};

	static const U32 MAX_INCLUDE_DEPTH = 8;

	GenericMemoryPoolAllocator<U8> m_alloc;
	StringAuto m_fname;
	ShaderProgramFilesystemInterface* m_fsystem = nullptr;

	StringListAuto m_codeLines = {m_alloc}; ///< The code.
	StringAuto m_codeSource = {m_alloc};
	U64 m_codeSourceHash = 0;

	DynamicArrayAuto<Mutator> m_mutators = {m_alloc};
	DynamicArrayAuto<MutationRewrite> m_mutationRewrites = {m_alloc};

	ShaderTypeBit m_shaderTypes = ShaderTypeBit::NONE;
	Bool m_insideShader = false;
	GpuDeviceCapabilities m_gpuCapabilities;
	BindlessLimits m_bindlessLimits;

	StringAuto m_libName = {m_alloc};
	U32 m_rayType = MAX_U32;

	ANKI_USE_RESULT Error parseFile(CString fname, U32 depth);
	ANKI_USE_RESULT Error parseLine(CString line, CString fname, Bool& foundPragmaOnce, U32 depth);
	ANKI_USE_RESULT Error parseInclude(const StringAuto* begin, const StringAuto* end, CString line, CString fname,
									   U32 depth);
	ANKI_USE_RESULT Error parsePragmaMutator(const StringAuto* begin, const StringAuto* end, CString line,
											 CString fname);
	ANKI_USE_RESULT Error parsePragmaStart(const StringAuto* begin, const StringAuto* end, CString line, CString fname);
	ANKI_USE_RESULT Error parsePragmaEnd(const StringAuto* begin, const StringAuto* end, CString line, CString fname);
	ANKI_USE_RESULT Error parsePragmaRewriteMutation(const StringAuto* begin, const StringAuto* end, CString line,
													 CString fname);
	ANKI_USE_RESULT Error parsePragmaLibraryName(const StringAuto* begin, const StringAuto* end, CString line,
												 CString fname);
	ANKI_USE_RESULT Error parsePragmaRayType(const StringAuto* begin, const StringAuto* end, CString line,
											 CString fname);

	void tokenizeLine(CString line, DynamicArrayAuto<StringAuto>& tokens) const;

	static Bool tokenIsComment(CString token)
	{
		return token.getLength() >= 2 && token[0] == '/' && (token[1] == '/' || token[1] == '*');
	}

	static Bool mutatorHasValue(const ShaderProgramParserMutator& mutator, MutatorValue value);
};
/// @}

} // end namespace anki
