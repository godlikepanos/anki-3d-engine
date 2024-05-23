// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/ShaderCompiler/ShaderParser.h>

namespace anki {

#define ANKI_PP_ERROR_MALFORMED() \
	ANKI_SHADER_COMPILER_LOGE("%s: Malformed expression: %s", fname.cstr(), line.cstr()); \
	return Error::kUserData

#define ANKI_PP_ERROR_MALFORMED_MSG(msg_) \
	ANKI_SHADER_COMPILER_LOGE("%s: " msg_ ": %s", fname.cstr(), line.cstr()); \
	return Error::kUserData

inline constexpr Array<CString, U32(ShaderType::kCount)> kShaderStageNames = {{"VERTEX", "TESSELLATION_CONTROL", "TESSELLATION_EVALUATION",
																			   "GEOMETRY", "TASK", "MESH", "FRAGMENT", "COMPUTE", "RAY_GEN",
																			   "ANY_HIT", "CLOSEST_HIT", "MISS", "INTERSECTION", "CALLABLE"}};

inline constexpr char kShaderHeader[] = R"(#define ANKI_%s_SHADER 1
#define kMaxBindlessTextures %uu
#define kMaxBindlessReadonlyTextureBuffers %uu
)";

static ShaderType strToShaderType(CString str)
{
	ShaderType shaderType = ShaderType::kCount;
	if(str == "vert")
	{
		shaderType = ShaderType::kVertex;
	}
	else if(str == "tessc")
	{
		shaderType = ShaderType::kTessellationControl;
	}
	else if(str == "tesse")
	{
	}
	else if(str == "geom")
	{
		shaderType = ShaderType::kGeometry;
	}
	else if(str == "task")
	{
		shaderType = ShaderType::kTask;
	}
	else if(str == "mesh")
	{
		shaderType = ShaderType::kMesh;
	}
	else if(str == "frag")
	{
		shaderType = ShaderType::kFragment;
	}
	else if(str == "comp")
	{
		shaderType = ShaderType::kCompute;
	}
	else if(str == "rgen")
	{
		shaderType = ShaderType::kRayGen;
	}
	else if(str == "ahit")
	{
		shaderType = ShaderType::kAnyHit;
	}
	else if(str == "chit")
	{
		shaderType = ShaderType::kClosestHit;
	}
	else if(str == "miss")
	{
		shaderType = ShaderType::kMiss;
	}
	else if(str == "int")
	{
		shaderType = ShaderType::kIntersection;
	}
	else if(str == "call")
	{
		shaderType = ShaderType::kCallable;
	}
	else
	{
		shaderType = ShaderType::kCount;
	}

	return shaderType;
}

ShaderParser::ShaderParser(CString fname, ShaderCompilerFilesystemInterface* fsystem, ConstWeakArray<ShaderCompilerDefine> defines)
	: m_fname(fname)
	, m_fsystem(fsystem)
{
	for(const ShaderCompilerDefine& def : defines)
	{
		m_defineNames.emplaceBack(def.m_name);
		m_defineValues.emplaceBack(def.m_value);
	}
}

ShaderParser::~ShaderParser()
{
}

void ShaderParser::tokenizeLine(CString line, ShaderCompilerDynamicArray<ShaderCompilerString>& tokens) const
{
	ANKI_ASSERT(line.getLength() > 0);

	ShaderCompilerString l = line;

	// Replace all tabs with spaces
	for(char& c : l)
	{
		if(c == '\t')
		{
			c = ' ';
		}
	}

	// Split
	ShaderCompilerStringList spaceTokens;
	spaceTokens.splitString(l, ' ', false);

	// Create the array
	for(const ShaderCompilerString& s : spaceTokens)
	{
		tokens.emplaceBack(s);
	}
}

Error ShaderParser::parsePragmaTechniqueStart(const ShaderCompilerString* begin, const ShaderCompilerString* end, CString line, CString fname)
{
	ANKI_ASSERT(begin && end);

	const PtrSize tokenCount = end - begin;
	if(tokenCount == 0)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	const ShaderType shaderType = strToShaderType(*begin);
	if(shaderType == ShaderType::kCount)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	ShaderCompilerString techniqueName;
	++begin;
	if(begin == end)
	{
		techniqueName = "Unnamed";
	}
	else if(*begin == "uses_mutators")
	{
		techniqueName = "Unnamed";
	}
	else if(*begin != "uses_mutators")
	{
		techniqueName = *begin;
		++begin;
	}

	// Mutators
	U64 activeMutators = kMaxU64;
	if(begin != end)
	{
		if(*begin != "uses_mutators")
		{
			ANKI_PP_ERROR_MALFORMED();
		}
		++begin;

		activeMutators = 0;
		for(; begin != end; ++begin)
		{
			// Find mutator
			U32 count = 0;
			for(const Mutator& mutator : m_mutators)
			{
				if(mutator.m_name == *begin)
				{
					activeMutators |= 1_U64 << U64(count);
					break;
				}
				++count;
			}

			if(count == m_mutators.getSize())
			{
				ANKI_PP_ERROR_MALFORMED_MSG("Mutator not found");
			}
		}
	}

	// Checks
	if(insideTechnique())
	{
		ANKI_PP_ERROR_MALFORMED_MSG("Need to close the previous technique_start before starting a new one");
	}

	// Find the technique
	Technique* technique = nullptr;
	for(Technique& t : m_techniques)
	{
		if(t.m_name == techniqueName)
		{
			if(!!(t.m_shaderTypes & ShaderTypeBit(1 << shaderType)))
			{
				ANKI_PP_ERROR_MALFORMED_MSG("technique_start with the same name and type appeared more than once");
			}

			technique = &t;
			break;
		}
	}

	// Done
	TechniqueExtra* extra = nullptr;
	if(!technique)
	{
		technique = m_techniques.emplaceBack();
		technique->m_name = techniqueName;

		extra = m_techniqueExtras.emplaceBack();
	}
	else
	{
		const U32 idx = U32(technique - m_techniques.getBegin());
		extra = &m_techniqueExtras[idx];
	}

	technique->m_shaderTypes |= ShaderTypeBit(1 << shaderType);
	technique->m_activeMutators[shaderType] = activeMutators;

	ANKI_ASSERT(extra->m_sourceLines[shaderType].getSize() == 0);
	extra->m_sourceLines[shaderType] = m_commonSourceLines;

	m_insideTechniqueIdx = U32(technique - m_techniques.getBegin());
	m_insideTechniqueShaderType = shaderType;

	return Error::kNone;
}

Error ShaderParser::parsePragmaTechniqueEnd(const ShaderCompilerString* begin, const ShaderCompilerString* end, CString line, CString fname)
{
	ANKI_ASSERT(begin && end);

	// Check tokens
	if(begin >= end)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	const ShaderType shaderType = strToShaderType(*begin);
	if(shaderType == ShaderType::kCount)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	ShaderCompilerString techniqueName;
	++begin;
	if(begin == end)
	{
		// Last token
		techniqueName = "Unnamed";
	}
	else
	{
		techniqueName = *begin;

		++begin;
		if(begin != end)
		{
			ANKI_PP_ERROR_MALFORMED();
		}
	}

	// Checks
	if(!insideTechnique())
	{
		ANKI_PP_ERROR_MALFORMED_MSG("Forgot to insert a #pragma anki technique_start");
	}

	if(m_techniques[m_insideTechniqueIdx].m_name != techniqueName || m_insideTechniqueShaderType != shaderType)
	{
		ANKI_PP_ERROR_MALFORMED_MSG("name or type doesn't match the one in technique_start");
	}

	// Done
	m_insideTechniqueIdx = kMaxU32;
	m_insideTechniqueShaderType = ShaderType::kCount;

	return Error::kNone;
}

Error ShaderParser::parsePragmaMutator(const ShaderCompilerString* begin, const ShaderCompilerString* end, CString line, CString fname)
{
	ANKI_ASSERT(begin && end);

	if(begin >= end)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	m_mutators.emplaceBack();
	Mutator& mutator = m_mutators.getBack();

	// Name
	{
		if(begin >= end)
		{
			// Need to have a name
			ANKI_PP_ERROR_MALFORMED();
		}

		// Check for duplicate mutators
		for(U32 i = 0; i < m_mutators.getSize() - 1; ++i)
		{
			if(m_mutators[i].m_name == *begin)
			{
				ANKI_PP_ERROR_MALFORMED_MSG("Duplicate mutator");
			}
		}

		if(begin->getLength() > kMaxShaderBinaryNameLength)
		{
			ANKI_PP_ERROR_MALFORMED_MSG("Too big name");
		}

		mutator.m_name = *begin;
		++begin;
	}

	// Values
	{
		// Gather them
		for(; begin < end; ++begin)
		{
			MutatorValue value = 0;

			if(tokenIsComment(begin->toCString()))
			{
				break;
			}

			if(begin->toNumber(value))
			{
				ANKI_PP_ERROR_MALFORMED();
			}

			mutator.m_values.emplaceBack(value);
		}

		std::sort(mutator.m_values.getBegin(), mutator.m_values.getEnd());

		// Check for duplicates
		for(U32 i = 1; i < mutator.m_values.getSize(); ++i)
		{
			if(mutator.m_values[i - 1] == mutator.m_values[i])
			{
				ANKI_PP_ERROR_MALFORMED_MSG("Same value appeared more than once");
			}
		}
	}

	return Error::kNone;
}

Error ShaderParser::parsePragmaSkipMutation(const ShaderCompilerString* begin, const ShaderCompilerString* end, CString line, CString fname)
{
	ANKI_ASSERT(begin && end);

	// Some basic sanity checks
	const U tokenCount = U(end - begin);
	// One pair doesn't make sence so it's: mutator_name_0 + mutator_value_0 + mutator_name_1 + mutator_value_1
	constexpr U minTokenCount = 2 + 2;
	if(tokenCount < minTokenCount || (tokenCount % 2) != 0)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	PartialMutationSkip& skip = *m_skipMutations.emplaceBack();
	skip.m_partialMutation.resize(m_mutators.getSize(), std::numeric_limits<MutatorValue>::max());

	do
	{
		// Get mutator name
		const CString mutatorName = *begin;
		U32 mutatorIndex = kMaxU32;
		for(U32 i = 0; i < m_mutators.getSize(); ++i)
		{
			if(m_mutators[i].m_name == mutatorName)
			{
				mutatorIndex = i;
				break;
			}
		}

		if(mutatorIndex == kMaxU32)
		{
			ANKI_PP_ERROR_MALFORMED_MSG("Mutator not found");
		}

		// Get mutator value
		++begin;
		const CString valueStr = *begin;
		MutatorValue value;
		if(valueStr.toNumber(value))
		{
			ANKI_PP_ERROR_MALFORMED_MSG("Malformed mutator value");
		}

		if(!mutatorHasValue(m_mutators[mutatorIndex], value))
		{
			ANKI_PP_ERROR_MALFORMED_MSG("Mutator value incorrect");
		}

		skip.m_partialMutation[mutatorIndex] = value;

		++begin;
	} while(begin < end && !tokenIsComment(*begin));

	return Error::kNone;
}

Error ShaderParser::parseInclude(const ShaderCompilerString* begin, const ShaderCompilerString* end, CString line, CString fname, U32 depth)
{
	// Gather the path
	ShaderCompilerString path;
	for(; begin < end; ++begin)
	{
		path += *begin;
	}

	if(path.isEmpty())
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	// Check
	const char firstChar = path[0];
	const char lastChar = path[path.getLength() - 1];

	if((firstChar == '\"' && lastChar == '\"') || (firstChar == '<' && lastChar == '>'))
	{
		ShaderCompilerString fname2(path.begin() + 1, path.begin() + path.getLength() - 1);

		const Bool dontIgnore =
			fname2.find("AnKi/Shaders/") != ShaderCompilerString::kNpos || fname2.find("ThirdParty/") != ShaderCompilerString::kNpos;
		if(!dontIgnore)
		{
			// The shaders can't include C++ files. Ignore the include
			return Error::kNone;
		}

		if(parseFile(fname2, depth + 1))
		{
			ANKI_PP_ERROR_MALFORMED_MSG("Error parsing include. See previous errors");
		}
	}
	else
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	return Error::kNone;
}

Error ShaderParser::parseLine(CString line, CString fname, Bool& foundPragmaOnce, U32 depth, U32 lineNo)
{
	// Tokenize
	ShaderCompilerDynamicArray<ShaderCompilerString> tokens;
	tokenizeLine(line, tokens);
	ANKI_ASSERT(tokens.getSize() > 0);

	const ShaderCompilerString* token = tokens.getBegin();
	const ShaderCompilerString* end = tokens.getEnd();

	// Skip the hash
	Bool foundAloneHash = false;
	if(*token == "#")
	{
		++token;
		foundAloneHash = true;
	}

	if((token < end) && ((foundAloneHash && *token == "include") || *token == "#include"))
	{
		// We _must_ have an #include
		ANKI_CHECK(parseInclude(token + 1, end, line, fname, depth));

		getAppendSourceList().pushBackSprintf("#line %u \"%s\"", lineNo + 1, sanitizeFilename(fname).cstr());
	}
	else if((token < end) && ((foundAloneHash && *token == "pragma") || *token == "#pragma"))
	{
		// We may have a #pragma once or a #pragma anki or something else
		++token;

		if(*token == "once")
		{
			// Pragma once

			if(foundPragmaOnce)
			{
				ANKI_PP_ERROR_MALFORMED_MSG("Can't have more than one #pragma once per file");
			}

			if(token + 1 != end)
			{
				ANKI_PP_ERROR_MALFORMED();
			}

			// Add the guard unique for this file
			foundPragmaOnce = true;
			const U64 hash = fname.computeHash();
			getAppendSourceList().pushBackSprintf("#ifndef _ANKI_INCL_GUARD_%" PRIu64 "\n"
												  "#define _ANKI_INCL_GUARD_%" PRIu64,
												  hash, hash);

			getAppendSourceList().pushBackSprintf("#line %u \"%s\"", lineNo + 1, sanitizeFilename(fname).cstr());
		}
		else if(*token == "anki")
		{
			// Must be a #pragma anki

			++token;

			if(*token == "mutator")
			{
				ANKI_CHECK(checkNoActiveStruct());
				ANKI_CHECK(parsePragmaMutator(token + 1, end, line, fname));
			}
			else if(*token == "technique_start")
			{
				ANKI_CHECK(checkNoActiveStruct());
				ANKI_CHECK(parsePragmaTechniqueStart(token + 1, end, line, fname));
			}
			else if(*token == "technique_end")
			{
				ANKI_CHECK(checkNoActiveStruct());
				ANKI_CHECK(parsePragmaTechniqueEnd(token + 1, end, line, fname));
			}
			else if(*token == "skip_mutation")
			{
				ANKI_CHECK(checkNoActiveStruct());
				ANKI_CHECK(parsePragmaSkipMutation(token + 1, end, line, fname));
			}
			else if(*token == "struct")
			{
				ANKI_CHECK(checkNoActiveStruct());
				ANKI_CHECK(parsePragmaStructBegin(token + 1, end, line, fname));
			}
			else if(*token == "struct_end")
			{
				ANKI_CHECK(checkActiveStruct());
				ANKI_CHECK(parsePragmaStructEnd(token + 1, end, line, fname));
			}
			else if(*token == "member")
			{
				ANKI_CHECK(checkActiveStruct());
				ANKI_CHECK(parsePragmaMember(token + 1, end, line, fname));
			}
			else if(*token == "16bit")
			{
				ANKI_CHECK(parsePragma16bit(token + 1, end, line, fname));
			}
			else
			{
				ANKI_PP_ERROR_MALFORMED();
			}

			// For good measure
			getAppendSourceList().pushBackSprintf("#line %u \"%s\"", lineNo + 1, sanitizeFilename(fname).cstr());
		}
		else
		{
			// Some other pragma, ignore
			getAppendSourceList().pushBack(line);
		}
	}
	else
	{
		// Ignore
		getAppendSourceList().pushBack(line);
	}

	return Error::kNone;
}

Error ShaderParser::parsePragmaStructBegin(const ShaderCompilerString* begin, const ShaderCompilerString* end, CString line, CString fname)
{
	const U tokenCount = U(end - begin);
	if(tokenCount != 1)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	GhostStruct& gstruct = *m_ghostStructs.emplaceBack();
	gstruct.m_name = *begin;

	getAppendSourceList().pushBackSprintf("struct %s {", begin->cstr());

	ANKI_ASSERT(!m_insideStruct);
	m_insideStruct = true;

	return Error::kNone;
}

Error ShaderParser::parsePragmaMember(const ShaderCompilerString* begin, const ShaderCompilerString* end, CString line, CString fname)
{
	ANKI_ASSERT(m_insideStruct);
	const U tokenCount = U(end - begin);
	if(tokenCount != 2)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	GhostStruct& structure = m_ghostStructs.getBack();
	Member member;

	// Type
	const CString typeStr = *begin;
	member.m_type = ShaderVariableDataType::kNone;
	if(typeStr == "F32" || typeStr == "RF32")
	{
		member.m_type = ShaderVariableDataType::kF32;
	}
	else if(typeStr == "Vec2" || typeStr == "RVec2")
	{
		member.m_type = ShaderVariableDataType::kVec2;
	}
	else if(typeStr == "Vec3" || typeStr == "RVec3")
	{
		member.m_type = ShaderVariableDataType::kVec3;
	}
	else if(typeStr == "Vec4" || typeStr == "RVec4")
	{
		member.m_type = ShaderVariableDataType::kVec4;
	}
	else if(typeStr == "U32")
	{
		member.m_type = ShaderVariableDataType::kU32;
	}

	if(member.m_type == ShaderVariableDataType::kNone)
	{
		ANKI_PP_ERROR_MALFORMED_MSG("Unrecognized type");
	}

	// Name
	++begin;
	member.m_name = *begin;

	// Rest
	member.m_offset = (structure.m_members.getSize())
						  ? structure.m_members.getBack().m_offset + getShaderVariableDataTypeInfo(structure.m_members.getBack().m_type).m_size
						  : 0;

	getAppendSourceList().pushBackSprintf("#define %s_%s_OFFSETOF %u", structure.m_name.cstr(), member.m_name.cstr(), member.m_offset);
	getAppendSourceList().pushBackSprintf("\t%s %s;", typeStr.cstr(), member.m_name.cstr());

	structure.m_members.emplaceBack(std::move(member));

	return Error::kNone;
}

Error ShaderParser::parsePragmaStructEnd(const ShaderCompilerString* begin, const ShaderCompilerString* end, CString line, CString fname)
{
	ANKI_ASSERT(m_insideStruct);

	if(begin != end)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	GhostStruct& gstruct = m_ghostStructs.getBack();
	const CString structName = gstruct.m_name;

	if(gstruct.m_members.isEmpty())
	{
		ANKI_PP_ERROR_MALFORMED_MSG("Struct doesn't have any members");
	}

	getAppendSourceList().pushBack("};");

	for(U32 i = 0; i < gstruct.m_members.getSize(); ++i)
	{
		const Member& m = gstruct.m_members[i];

		// #	define XXX_LOAD()
		getAppendSourceList().pushBackSprintf("#\tdefine %s_%s_LOAD(buff, offset) buff.Load<%s>(%s_%s_OFFSETOF + (offset))%s", structName.cstr(),
											  m.m_name.cstr(), getShaderVariableDataTypeInfo(m.m_type).m_name, structName.cstr(), m.m_name.cstr(),
											  (i != gstruct.m_members.getSize() - 1) ? "," : "");
	}

	// Now define the structure LOAD in HLSL
	getAppendSourceList().pushBackSprintf("#define load%s(buff, offset) { \\", structName.cstr());
	for(U32 i = 0; i < gstruct.m_members.getSize(); ++i)
	{
		const Member& m = gstruct.m_members[i];
		getAppendSourceList().pushBackSprintf("\t%s_%s_LOAD(buff, offset) \\", structName.cstr(), m.m_name.cstr());
	}
	getAppendSourceList().pushBack("}");

	// Done
	m_insideStruct = false;
	return Error::kNone;
}

Error ShaderParser::parsePragma16bit(const ShaderCompilerString* begin, const ShaderCompilerString* end, CString line, CString fname)
{
	ANKI_ASSERT(begin && end);

	// Check tokens
	if(begin != end)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	m_16bitTypes = true;

	return Error::kNone;
}

Error ShaderParser::parseFile(CString fname, U32 depth)
{
	// First check the depth
	if(depth > kMaxIncludeDepth)
	{
		ANKI_SHADER_COMPILER_LOGE("The include depth is too high. Probably circular includance");
	}

	Bool foundPragmaOnce = false;

	// Load file in lines
	ShaderCompilerString txt;
	ANKI_CHECK(m_fsystem->readAllText(fname, txt));

	m_hash = (m_hash) ? computeHash(txt.cstr(), txt.getLength()) : appendHash(txt.cstr(), txt.getLength(), m_hash);

	ShaderCompilerStringList lines;
	lines.splitString(txt, '\n', true);
	if(lines.getSize() < 1)
	{
		ANKI_SHADER_COMPILER_LOGE("Source is empty");
	}

	getAppendSourceList().pushBackSprintf("#line 0 \"%s\"", sanitizeFilename(fname).cstr());

	// Parse lines
	U32 lineNo = 1;
	for(const ShaderCompilerString& line : lines)
	{
		if(line.isEmpty())
		{
			getAppendSourceList().pushBack(" ");
		}
		else if(line.find("pragma") != ShaderCompilerString::kNpos || line.find("include") != ShaderCompilerString::kNpos)
		{
			// Possibly a preprocessor directive we care
			ANKI_CHECK(parseLine(line.toCString(), fname, foundPragmaOnce, depth, lineNo));
		}
		else
		{
			// Just append the line
			getAppendSourceList().pushBack(line.toCString());
		}

		++lineNo;
	}

	if(foundPragmaOnce)
	{
		// Append the guard
		getAppendSourceList().pushBack("#endif // Include guard");
	}

	return Error::kNone;
}

Error ShaderParser::parse()
{
	ANKI_ASSERT(!m_fname.isEmpty());
	ANKI_ASSERT(m_commonSourceLines.isEmpty());

	const CString fname = m_fname;

	// Parse recursively
	ANKI_CHECK(parseFile(fname, 0));

	// Checks
	{
		if(m_techniques.getSize() == 0)
		{
			ANKI_SHADER_COMPILER_LOGE("No techniques were found");
			return Error::kUserData;
		}

		if(insideTechnique())
		{
			ANKI_SHADER_COMPILER_LOGE("Forgot to end a technique");
			return Error::kUserData;
		}

		if(m_insideStruct)
		{
			ANKI_SHADER_COMPILER_LOGE("Forgot to end a struct");
			return Error::kUserData;
		}
	}

	// Create the code lines for each technique
	for(U32 i = 0; i < m_techniques.getSize(); ++i)
	{
		for(ShaderType s : EnumIterable<ShaderType>())
		{
			if(m_techniqueExtras[i].m_sourceLines[s].getSize())
			{
				ANKI_ASSERT(!!(m_techniques[i].m_shaderTypes & ShaderTypeBit(1 << s)));
				m_techniqueExtras[i].m_sourceLines[s].join("\n", m_techniqueExtras[i].m_sources[s]);
				m_techniqueExtras[i].m_sourceLines[s].destroy(); // Free mem
			}
			else
			{
				ANKI_ASSERT(!(m_techniques[i].m_shaderTypes & ShaderTypeBit(1 << s)));
			}
		}
	}

	m_commonSourceLines.destroy(); // Free mem

	return Error::kNone;
}

void ShaderParser::generateAnkiShaderHeader(ShaderType shaderType, ShaderCompilerString& header)
{
	header.destroy();

	header += ShaderCompilerString().sprintf("#define kMaxBindlessTextures %uu\n"
											 "#define kMaxBindlessReadonlyTextureBuffers %uu\n",
											 kMaxBindlessTextures, kMaxBindlessReadonlyTextureBuffers);

	for(ShaderType type : EnumIterable<ShaderType>())
	{
		header += ShaderCompilerString().sprintf("#define ANKI_%s_SHADER %u\n", kShaderStageNames[type].cstr(), (shaderType == type) ? 1 : 0);
	}
}

void ShaderParser::generateVariant(ConstWeakArray<MutatorValue> mutation, const ShaderParserTechnique& technique, ShaderType shaderType,
								   ShaderCompilerString& source) const
{
	// Sanity checks
	ANKI_ASSERT(mutation.getSize() == m_mutators.getSize());
	for(U32 i = 0; i < mutation.getSize(); ++i)
	{
		ANKI_ASSERT(mutatorHasValue(m_mutators[i], mutation[i]) && "Value not found");
	}
	ANKI_ASSERT(!!(technique.m_shaderTypes & ShaderTypeBit(1 << shaderType)));

	source.destroy();

	ANKI_ASSERT(&technique >= m_techniques.getBegin() && &technique < m_techniques.getEnd());
	const U32 tIdx = U32(&technique - m_techniques.getBegin());

	for(U32 i = 0; i < m_defineNames.getSize(); ++i)
	{
		source += ShaderCompilerString().sprintf("#define %s %d\n", m_defineNames[i].cstr(), m_defineValues[i]);
	}

	for(U32 i = 0; i < mutation.getSize(); ++i)
	{
		if(!!(technique.m_activeMutators[shaderType] & (1_U64 << U64(i))))
		{
			source += ShaderCompilerString().sprintf("#define %s %d\n", m_mutators[i].m_name.cstr(), mutation[i]);
		}
	}

	for(U32 i = 0; i < m_techniques.getSize(); ++i)
	{
		source += ShaderCompilerString().sprintf("#define ANKI_TECHNIQUE_%s %u\n", m_techniques[i].m_name.cstr(), U32(tIdx == i));
	}

	ShaderCompilerString header;
	generateAnkiShaderHeader(shaderType, header);
	source += header;

	if(m_16bitTypes)
	{
		source += "#define ANKI_SUPPORTS_16BIT_TYPES 1\n";
	}
	else
	{
		source += "#define ANKI_SUPPORTS_16BIT_TYPES 0\n";
	}

	ANKI_ASSERT(m_techniqueExtras[tIdx].m_sources[shaderType].getLength() > 0);
	source += m_techniqueExtras[tIdx].m_sources[shaderType];
}

Bool ShaderParser::mutatorHasValue(const ShaderParserMutator& mutator, MutatorValue value)
{
	for(MutatorValue v : mutator.m_values)
	{
		if(value == v)
		{
			return true;
		}
	}

	return false;
}

Bool ShaderParser::skipMutation(ConstWeakArray<MutatorValue> mutation) const
{
	ANKI_ASSERT(mutation.getSize() == m_mutators.getSize());

	for(const PartialMutationSkip& skip : m_skipMutations)
	{
		Bool doSkip = true;
		for(U32 i = 0; i < m_mutators.getSize(); ++i)
		{
			if(skip.m_partialMutation[i] == std::numeric_limits<MutatorValue>::max())
			{
				// Don't care
				continue;
			}

			if(skip.m_partialMutation[i] != mutation[i])
			{
				doSkip = false;
				break;
			}
		}

		if(doSkip)
		{
			return true;
		}
	}

	return false;
}

} // end namespace anki
