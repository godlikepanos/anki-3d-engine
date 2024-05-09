// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/ShaderCompiler/Dxc.h>
#include <AnKi/Util/Process.h>
#include <AnKi/Util/Filesystem.h>
#include <AnKi/Util/File.h>
#include <AnKi/Util/HighRezTimer.h>

namespace anki {

static Atomic<U32> g_nextFileId = {1};

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
	case ShaderType::kTask:
		return "as_6_7";
		break;
	case ShaderType::kMesh:
		return "ms_6_7";
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

static Error compileHlsl(CString src, ShaderType shaderType, Bool compileWith16bitTypes, Bool spirv, ShaderCompilerDynamicArray<U8>& bin,
						 ShaderCompilerString& errorMessage)
{
	Array<U64, 3> toHash = {g_nextFileId.fetchAdd(1), getCurrentProcessId(), getRandom() & kMaxU32};
	const U64 rand = computeHash(&toHash[0], sizeof(toHash));

	String tmpDir;
	ANKI_CHECK(getTempDirectory(tmpDir));

	// Store HLSL to a file
	ShaderCompilerString hlslFilename;
	hlslFilename.sprintf("%s/%" PRIu64 ".hlsl", tmpDir.cstr(), rand);

	File hlslFile;
	ANKI_CHECK(hlslFile.open(hlslFilename, FileOpenFlag::kWrite));
	CleanupFile hlslFileCleanup(hlslFilename);
	ANKI_CHECK(hlslFile.writeText(src));
	hlslFile.close();

	// Call DXC
	ShaderCompilerString binFilename;
	binFilename.sprintf("%s/%" PRIu64 ".spvdxil", tmpDir.cstr(), rand);

	ShaderCompilerDynamicArray<ShaderCompilerString> dxcArgs;
	dxcArgs.emplaceBack("-Fo");
	dxcArgs.emplaceBack(binFilename);
	dxcArgs.emplaceBack("-Wall");
	dxcArgs.emplaceBack("-Wextra");
	dxcArgs.emplaceBack("-Wno-conversion");
	dxcArgs.emplaceBack("-Werror");
	dxcArgs.emplaceBack("-Wfatal-errors");
	dxcArgs.emplaceBack("-Wundef");
	dxcArgs.emplaceBack("-Wno-unused-const-variable");
	dxcArgs.emplaceBack("-Wno-unused-parameter");
	dxcArgs.emplaceBack("-Wno-unneeded-internal-declaration");
	dxcArgs.emplaceBack("-HV");
	dxcArgs.emplaceBack("2021");
	dxcArgs.emplaceBack("-E");
	dxcArgs.emplaceBack("main");
	dxcArgs.emplaceBack("-T");
	dxcArgs.emplaceBack(profile(shaderType));
	if(spirv)
	{
		dxcArgs.emplaceBack("-spirv");
		dxcArgs.emplaceBack("-fspv-target-env=vulkan1.1spirv1.4");
		// dxcArgs.emplaceBack("-fvk-support-nonzero-base-instance"); // Match DX12's behavior, SV_INSTANCEID starts from zero
	}
	else
	{
		dxcArgs.emplaceBack("-Wno-ignored-attributes"); // TODO rm that eventually
		dxcArgs.emplaceBack("-Wno-inline-asm"); // TODO rm that eventually
	}
	// dxcArgs.emplaceBack("-Zi"); // Debug info
	dxcArgs.emplaceBack(hlslFilename);

	if(compileWith16bitTypes)
	{
		dxcArgs.emplaceBack("-enable-16bit-types");
	}

	ShaderCompilerDynamicArray<CString> dxcArgs2;
	dxcArgs2.resize(dxcArgs.getSize());
	for(U32 i = 0; i < dxcArgs.getSize(); ++i)
	{
		dxcArgs2[i] = dxcArgs[i];
	}

	while(true)
	{
		I32 exitCode;
		ShaderCompilerString stdOut;

#if ANKI_OS_WINDOWS
		CString dxcBin = ANKI_SOURCE_DIRECTORY "/ThirdParty/Bin/Windows64/dxc.exe";
#elif ANKI_OS_LINUX
		CString dxcBin = ANKI_SOURCE_DIRECTORY "/ThirdParty/Bin/Linux64/dxc";
#else
		CString dxcBin = "N/A";
#endif

		// Run once without stdout or stderr. Because if you do the process library will crap out after a while
		ANKI_CHECK(Process::callProcess(dxcBin, dxcArgs2, nullptr, nullptr, exitCode));

		if(exitCode != 0)
		{
			// There was an error, run again just to get the stderr
			String errorMessageTmp;
			const Error err = Process::callProcess(dxcBin, dxcArgs2, nullptr, &errorMessageTmp, exitCode);
			(void)err; // Shoudn't throw an error
			errorMessage = errorMessageTmp;

			if(!errorMessage.isEmpty() && errorMessage.find("The process cannot access the file because") != CString::kNpos)
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

	CleanupFile binFileCleanup(binFilename);

	// Read the spirv back
	File binFile;
	ANKI_CHECK(binFile.open(binFilename, FileOpenFlag::kRead));
	bin.resize(U32(binFile.getSize()));
	ANKI_CHECK(binFile.read(&bin[0], bin.getSizeInBytes()));
	binFile.close();

	return Error::kNone;
}

Error compileHlslToSpirv(CString src, ShaderType shaderType, Bool compileWith16bitTypes, ShaderCompilerDynamicArray<U8>& spirv,
						 ShaderCompilerString& errorMessage)
{
	return compileHlsl(src, shaderType, compileWith16bitTypes, true, spirv, errorMessage);
}

Error compileHlslToDxil(CString src, ShaderType shaderType, Bool compileWith16bitTypes, ShaderCompilerDynamicArray<U8>& dxil,
						ShaderCompilerString& errorMessage)
{
	return compileHlsl(src, shaderType, compileWith16bitTypes, false, dxil, errorMessage);
}

} // end namespace anki
