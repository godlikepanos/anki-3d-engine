// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/ShaderCompiler/Dxc.h>
#include <AnKi/Util/Process.h>
#include <AnKi/Util/Filesystem.h>
#include <AnKi/Util/File.h>
#include <AnKi/Util/HighRezTimer.h>

namespace anki {

static CString profile(ShaderType shaderType)
{
	switch(shaderType)
	{
	case ShaderType::kVertex:
		return "vs_6_7";
		break;
	case ShaderType::kFragment:
		return "ps_6_7";
		break;
	case ShaderType::kTessellationEvaluation:
		return "ds_6_7";
		break;
	case ShaderType::kTessellationControl:
		return "ds_6_7";
		break;
	case ShaderType::kGeometry:
		return "gs_6_7";
		break;
	case ShaderType::kCompute:
		return "cs_6_7";
		break;
	case ShaderType::kRayGen:
	case ShaderType::kAnyHit:
	case ShaderType::kClosestHit:
	case ShaderType::kMiss:
	case ShaderType::kIntersection:
	case ShaderType::kCallable:
		return "lib_6_7";
		break;
	default:
		ANKI_ASSERT(0);
	};

	return "";
}

Error compileHlslToSpirv(CString src, ShaderType shaderType, BaseMemoryPool& tmpPool, DynamicArrayRaii<U8>& spirv,
						 StringRaii& errorMessage)
{
	const U64 rand = getRandom();

	StringRaii tmpDir(&tmpPool);
	ANKI_CHECK(getTempDirectory(tmpDir));

	// Store HLSL to a file
	StringRaii hlslFilename(&tmpPool);
	hlslFilename.sprintf("%s/%llu.hlsl", tmpDir.cstr(), rand);

	File hlslFile;
	ANKI_CHECK(hlslFile.open(hlslFilename, FileOpenFlag::kWrite));
	CleanupFile hlslFileCleanup(&tmpPool, hlslFilename);
	ANKI_CHECK(hlslFile.writeText(src));
	hlslFile.close();

	// Call DXC
	StringRaii spvFilename(&tmpPool);
	spvFilename.sprintf("%s/%llu.spv", tmpDir.cstr(), rand);

	DynamicArrayRaii<StringRaii> dxcArgs(&tmpPool);
	dxcArgs.emplaceBack(&tmpPool, "-Fo");
	dxcArgs.emplaceBack(&tmpPool, spvFilename);
	dxcArgs.emplaceBack(&tmpPool, "-Wall");
	dxcArgs.emplaceBack(&tmpPool, "-Wextra");
	dxcArgs.emplaceBack(&tmpPool, "-Wno-conversion");
	dxcArgs.emplaceBack(&tmpPool, "-Werror");
	dxcArgs.emplaceBack(&tmpPool, "-Wfatal-errors");
	dxcArgs.emplaceBack(&tmpPool, "-Wundef");
	dxcArgs.emplaceBack(&tmpPool, "-Wno-unused-const-variable");
	// dxcArgs.emplaceBack(&tmpPool, "-enable-16bit-types");
	dxcArgs.emplaceBack(&tmpPool, "-HV");
	dxcArgs.emplaceBack(&tmpPool, "2021");
	dxcArgs.emplaceBack(&tmpPool, "-E");
	dxcArgs.emplaceBack(&tmpPool, "main");
	dxcArgs.emplaceBack(&tmpPool, "-T");
	dxcArgs.emplaceBack(&tmpPool, profile(shaderType));
	dxcArgs.emplaceBack(&tmpPool, "-spirv");
	dxcArgs.emplaceBack(&tmpPool, "-fspv-target-env=vulkan1.1spirv1.4");
	dxcArgs.emplaceBack(&tmpPool, hlslFilename);

	DynamicArrayRaii<CString> dxcArgs2(&tmpPool, dxcArgs.getSize());
	for(U32 i = 0; i < dxcArgs.getSize(); ++i)
	{
		dxcArgs2[i] = dxcArgs[i];
	}

	while(true)
	{
		I32 exitCode;
		StringRaii stdOut(&tmpPool);

		// Run once without stdout or stderr. Because if you do the process library will crap out after a while
		ANKI_CHECK(Process::callProcess(ANKI_SOURCE_DIRECTORY "/ThirdParty/Bin/Windows64/dxc.exe", dxcArgs2, nullptr,
										nullptr, exitCode));

		if(exitCode != 0)
		{
			// There was an error, run again just to get the stderr
			ANKI_CHECK(Process::callProcess(ANKI_SOURCE_DIRECTORY "/ThirdParty/Bin/Windows64/dxc.exe", dxcArgs2,
											nullptr, &errorMessage, exitCode));

			if(!errorMessage.isEmpty()
			   && errorMessage.find("The process cannot access the file because") != CString::kNpos)
			{
				// DXC might fail to read the HLSL because the antivirus might be having a lock on it. Try again
				errorMessage.destroy();
				HighRezTimer::sleep(1.0_ms);
			}
			else if(errorMessage.isEmpty())
			{
				errorMessage = "Unknown error";
				return Error::kFunctionFailed;
			}
			else
			{
				// printf("%s\n", src.cstr());
				return Error::kFunctionFailed;
			}
		}
		else
		{
			break;
		}
	}

	CleanupFile spvFileCleanup(&tmpPool, spvFilename);

	// Read the spirv back
	File spvFile;
	ANKI_CHECK(spvFile.open(spvFilename, FileOpenFlag::kRead));
	spirv.resize(U32(spvFile.getSize()));
	ANKI_CHECK(spvFile.read(&spirv[0], spirv.getSizeInBytes()));
	spvFile.close();

	return Error::kNone;
}

} // end namespace anki