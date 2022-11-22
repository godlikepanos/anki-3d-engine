// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/ShaderCompiler/Dxc.h>
#include <AnKi/Util/Process.h>
#include <AnKi/Util/Filesystem.h>
#include <AnKi/Util/File.h>

namespace anki {

namespace {

class CleanupFile
{
public:
	StringRaii m_fileToDelete;

	CleanupFile(BaseMemoryPool* pool, CString filename)
		: m_fileToDelete(pool, filename)
	{
	}

	~CleanupFile()
	{
		if(!m_fileToDelete.isEmpty())
		{
			const Error err = removeFile(m_fileToDelete);
			if(!err)
			{
				ANKI_SHADER_COMPILER_LOGV("Deleted %s", m_fileToDelete.cstr());
			}
		}
	}
};

} // end anonymous namespace

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
	srand(U32(time(0)));
	const I32 r = rand();

	StringRaii tmpDir(&tmpPool);
	ANKI_CHECK(getTempDirectory(tmpDir));

	// Store src to a file
	StringRaii hlslFilename(&tmpPool);
	hlslFilename.sprintf("%s/%d.hlsl", tmpDir.cstr(), r);

	File hlslFile;
	ANKI_CHECK(hlslFile.open(hlslFilename, FileOpenFlag::kWrite));
	CleanupFile hlslFileCleanup(&tmpPool, hlslFilename);
	ANKI_CHECK(hlslFile.writeText(src));
	hlslFile.close();

	// Call DXC
	StringRaii spvFilename(&tmpPool);
	spvFilename.sprintf("%s/%d.spv", tmpDir.cstr(), r);

	DynamicArrayRaii<StringRaii> dxcArgs(&tmpPool);
	dxcArgs.emplaceBack(&tmpPool, "-Wall");
	dxcArgs.emplaceBack(&tmpPool, "-Wextra");
	dxcArgs.emplaceBack(&tmpPool, "-Wconversion");
	dxcArgs.emplaceBack(&tmpPool, "-Werror");
	dxcArgs.emplaceBack(&tmpPool, "-Wfatal-errors");
	dxcArgs.emplaceBack(&tmpPool, "-Wno-unused-const-variable");
	dxcArgs.emplaceBack(&tmpPool, "-enable-16bit-types");
	dxcArgs.emplaceBack(&tmpPool, "-HV");
	dxcArgs.emplaceBack(&tmpPool, "2021");
	dxcArgs.emplaceBack(&tmpPool, "-E");
	dxcArgs.emplaceBack(&tmpPool, "main");
	dxcArgs.emplaceBack(&tmpPool, "-Fo");
	dxcArgs.emplaceBack(&tmpPool, spvFilename);
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

	Process proc;
	ANKI_CHECK(proc.start(ANKI_SOURCE_DIRECTORY "/ThirdParty/Bin/Windows64/dxc.exe", dxcArgs2));

	I32 exitCode;
	ProcessStatus status;
	ANKI_CHECK(proc.wait(60.0_sec, &status, &exitCode));

	if(!(status == ProcessStatus::kNotRunning && exitCode == 0))
	{
		if(exitCode != 0)
		{
			ANKI_CHECK(proc.readFromStderr(errorMessage));
		}

		if(errorMessage.isEmpty())
		{
			errorMessage = "Unknown error";
		}

		return Error::kFunctionFailed;
	}

	CleanupFile spvFileCleanup(&tmpPool, spvFilename);

	// Read the spirv back
	File spvFile;
	ANKI_CHECK(spvFile.open(spvFilename, FileOpenFlag::kRead));
	spirv.resize(U32(spvFile.getSize()));
	ANKI_CHECK(spvFile.read(&spirv[0], spirv.getSizeInBytes()));

	return Error::kNone;
}

} // end namespace anki
