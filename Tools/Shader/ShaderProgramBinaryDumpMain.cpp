// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/ShaderCompiler/ShaderCompiler.h>
#include <AnKi/ShaderCompiler/ShaderDump.h>
#include <AnKi/Util/ThreadHive.h>
#include <AnKi/Util/System.h>
#include <ThirdParty/SpirvCross/spirv.hpp>

using namespace anki;

static const char* kUsage = R"(Dump the shader binary to stdout
Usage: %s [options] input_shader_program_binary
Options:
-stats <0|1>  : Print performance statistics for all shaders. Default 0
-binary <0|1> : Print the whole shader program binary. Default 1
-glsl <0|1>   : Print GLSL. Default 1
-spirv <0|1>  : Print SPIR-V. Default 0
-v            : Verbose log
)";

static Error parseCommandLineArgs(WeakArray<char*> argv, Bool& dumpStats, Bool& dumpBinary, Bool& glsl, Bool& spirv, String& filename)
{
	// Parse config
	if(argv.getSize() < 2)
	{
		return Error::kUserData;
	}

	dumpStats = false;
	dumpBinary = true;
	glsl = true;
	spirv = false;
	filename = argv[argv.getSize() - 1];

	for(U32 i = 1; i < argv.getSize() - 1; i++)
	{
		if(CString(argv[i]) == "-stats")
		{
			++i;
			if(i >= argv.getSize())
			{
				return Error::kUserData;
			}

			if(CString(argv[i]) == "1")
			{
				dumpStats = true;
			}
			else if(CString(argv[i]) == "0")
			{
				dumpStats = false;
			}
			else
			{
				return Error::kUserData;
			}
		}
		else if(CString(argv[i]) == "-binary")
		{
			++i;
			if(i >= argv.getSize())
			{
				return Error::kUserData;
			}

			if(CString(argv[i]) == "1")
			{
				dumpBinary = true;
			}
			else if(CString(argv[i]) == "0")
			{
				dumpBinary = false;
			}
			else
			{
				return Error::kUserData;
			}
		}
		else if(CString(argv[i]) == "-glsl")
		{
			++i;
			if(i >= argv.getSize())
			{
				return Error::kUserData;
			}

			if(CString(argv[i]) == "1")
			{
				glsl = true;
			}
			else if(CString(argv[i]) == "0")
			{
				glsl = false;
			}
			else
			{
				return Error::kUserData;
			}
		}
		else if(CString(argv[i]) == "-spirv")
		{
			++i;
			if(i >= argv.getSize())
			{
				return Error::kUserData;
			}

			if(CString(argv[i]) == "1")
			{
				spirv = true;
			}
			else if(CString(argv[i]) == "0")
			{
				spirv = false;
			}
			else
			{
				return Error::kUserData;
			}
		}
		else if(CString(argv[i]) == "-v")
		{
			Logger::getSingleton().enableVerbosity(true);
		}
	}

	if(spirv || glsl)
	{
		dumpBinary = true;
	}

	return Error::kNone;
}

Error dump(CString fname, Bool bDumpStats, Bool dumpBinary, Bool glsl, Bool spirv)
{
	ShaderBinary* binary;
	ANKI_CHECK(deserializeShaderBinaryFromFile(fname, binary, ShaderCompilerMemoryPool::getSingleton()));

	class Dummy
	{
	public:
		ShaderBinary* m_binary;

		~Dummy()
		{
			ShaderCompilerMemoryPool::getSingleton().free(m_binary);
		}
	} dummy{binary};

	if(dumpBinary)
	{
		ShaderDumpOptions options;
		options.m_writeGlsl = glsl;
		options.m_writeSpirv = spirv;
		options.m_maliStats = options.m_amdStats = bDumpStats;

		ShaderCompilerString txt;
		dumpShaderBinary(options, *binary, txt);

		printf("%s\n", txt.cstr());
	}

	return Error::kNone;
}

ANKI_MAIN_FUNCTION(myMain)
int myMain(int argc, char** argv)
{
	class Dummy
	{
	public:
		~Dummy()
		{
			DefaultMemoryPool::freeSingleton();
			ShaderCompilerMemoryPool::freeSingleton();
		}
	} dummy;

	DefaultMemoryPool::allocateSingleton(allocAligned, nullptr);
	ShaderCompilerMemoryPool::allocateSingleton(allocAligned, nullptr);

	String filename;
	Bool dumpStats;
	Bool dumpBinary;
	Bool glsl;
	Bool spirv;
	if(parseCommandLineArgs(WeakArray<char*>(argv, argc), dumpStats, dumpBinary, glsl, spirv, filename))
	{
		ANKI_LOGE(kUsage, argv[0]);
		return 1;
	}

	const Error err = dump(filename, dumpStats, dumpBinary, glsl, spirv);
	if(err)
	{
		ANKI_LOGE("Can't dump due to an error. Bye");
		return 1;
	}

	return 0;
}
