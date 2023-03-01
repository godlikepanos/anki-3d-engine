// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/ShaderCompiler/RadeonGpuAnalyzer.h>
#include <AnKi/Util/File.h>
#include <AnKi/Util/Filesystem.h>
#include <AnKi/Util/Process.h>
#include <AnKi/Util/StringList.h>

namespace anki {

static Atomic<U32> g_nextFileId = {1};

static CString getPipelineStageString(ShaderType shaderType)
{
	CString out;
	switch(shaderType)
	{
	case ShaderType::kVertex:
		out = "vert";
		break;
	case ShaderType::kFragment:
		out = "frag";
		break;
	case ShaderType::kCompute:
		out = "comp";
		break;
	default:
		ANKI_ASSERT(!"TODO");
	}

	return out;
}

Error runRadeonGpuAnalyzer(CString rgaExecutable, ConstWeakArray<U8> spirv, ShaderType shaderType,
						   BaseMemoryPool& tmpPool, RgaOutput& out)
{
	ANKI_ASSERT(spirv.getSize() > 0);
	const U32 rand = g_nextFileId.fetchAdd(1) + getCurrentProcessId();

	// Store SPIRV
	StringRaii tmpDir(&tmpPool);
	ANKI_CHECK(getTempDirectory(tmpDir));
	StringRaii spvFilename(&tmpPool);
	spvFilename.sprintf("%s/AnKiRgaInput_%u.spv", tmpDir.cstr(), rand);
	File spvFile;
	ANKI_CHECK(spvFile.open(spvFilename, FileOpenFlag::kWrite | FileOpenFlag::kBinary));
	ANKI_CHECK(spvFile.write(&spirv[0], spirv.getSizeInBytes()));
	spvFile.close();
	CleanupFile spvFileCleanup(spvFilename);

	// Call RGA
	StringRaii analysisFilename(&tmpPool);
	analysisFilename.sprintf("%s/AnKiRgaOutAnalysis_%u.csv", tmpDir.cstr(), rand);

	Array<CString, 7> args;
	args[0] = "-s";
	args[1] = "vk-spv-offline";
	args[2] = "-c";
	args[3] = "gfx1030"; // Target RDNA2
	args[4] = "-a";
	args[5] = analysisFilename;
	args[6] = spvFilename;

	I32 exitCode;
	ANKI_CHECK(Process::callProcess(rgaExecutable, args, nullptr, nullptr, exitCode));

	if(exitCode != 0)
	{
		ANKI_SHADER_COMPILER_LOGE("RGA failed with exit code %d", exitCode);
		return Error::kFunctionFailed;
	}

	// Construct the output filename
	StringRaii outFilename(&tmpPool);
	outFilename.sprintf("%s/gfx1030_AnKiRgaOutAnalysis_%u_%s.csv", tmpDir.cstr(), rand,
						getPipelineStageString(shaderType).cstr());

	CleanupFile rgaFileCleanup(outFilename);

	// Read the file
	File analysisFile;
	ANKI_CHECK(analysisFile.open(outFilename, FileOpenFlag::kRead));
	StringRaii analysisText(&tmpPool);
	ANKI_CHECK(analysisFile.readAllText(analysisText));
	analysisText.replaceAll("\r", "");
	analysisFile.close();

	// Parse the text
	StringListRaii lines(&tmpPool);
	lines.splitString(analysisText, '\n');
	if(lines.getSize() != 2)
	{
		ANKI_SHADER_COMPILER_LOGE("Error parsing RGA analysis file");
		return Error::kFunctionFailed;
	}

	StringListRaii tokens(&tmpPool);
	tokens.splitString(lines.getFront(), ',');

	StringListRaii values(&tmpPool);
	values.splitString(*(lines.getBegin() + 1), ',');

	if(tokens.getSize() != tokens.getSize())
	{
		ANKI_SHADER_COMPILER_LOGE("Error parsing RGA analysis file");
		return Error::kFunctionFailed;
	}

	out.m_vgprCount = kMaxU32;
	out.m_sgprCount = kMaxU32;
	out.m_isaSize = kMaxU32;
	auto valuesIt = values.getBegin();
	for(const String& token : tokens)
	{
		if(token.find("USED_VGPRs") != String::kNpos)
		{
			ANKI_CHECK((*valuesIt).toNumber(out.m_vgprCount));
		}
		else if(token.find("USED_SGPRs") != String::kNpos)
		{
			ANKI_CHECK((*valuesIt).toNumber(out.m_sgprCount));
		}
		else if(token.find("ISA_SIZE") != String::kNpos)
		{
			ANKI_CHECK((*valuesIt).toNumber(out.m_isaSize));
		}

		++valuesIt;
	}

	if(out.m_vgprCount == kMaxU32 || out.m_sgprCount == kMaxU32 || out.m_isaSize == kMaxU32)
	{
		ANKI_SHADER_COMPILER_LOGE("Error parsing RGA analysis file");
		return Error::kFunctionFailed;
	}

	return Error::kNone;
}

} // end namespace anki
