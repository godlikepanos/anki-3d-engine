// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/ShaderCompiler/ShaderCompiler.h>
#include <AnKi/Util.h>
using namespace anki;

static constexpr const char* kUsage = R"(Compile an AnKi shader program
Usage: %s [options] input_shader_program_file
Options:
-o <name of output>  : The name of the output binary
-j <thread count>    : Number of threads. Defaults to system's max
-I <include path>    : The path of the #include files
-D<define_name:val>  : Extra defines to pass to the compiler
-spirv               : Compile SPIR-V
-dxil                : Compile DXIL
-g                   : Include debug info
-sm                  : Shader mode. "6_7" or "6_8". Default "6_8"
)";

class CmdLineArgs
{
public:
	String m_inputFname;
	String m_outFname;
	String m_includePath;
	U32 m_threadCount = getCpuCoresCount();
	DynamicArray<String> m_defineNames;
	DynamicArray<ShaderCompilerDefine> m_defines;
	Bool m_spirv = false;
	Bool m_dxil = false;
	Bool m_debugInfo = false;
	ShaderModel m_sm = ShaderModel::k6_8;
};

static Error parseCommandLineArgs(int argc, char** argv, CmdLineArgs& info)
{
	// Parse config
	if(argc < 2)
	{
		return Error::kUserData;
	}

	for(I i = 1; i < argc - 1; i++)
	{
		if(strcmp(argv[i], "-o") == 0)
		{
			++i;

			if(i < argc)
			{
				if(std::strlen(argv[i]) > 0)
				{
					info.m_outFname.sprintf("%s", argv[i]);
				}
				else
				{
					return Error::kUserData;
				}
			}
			else
			{
				return Error::kUserData;
			}
		}
		else if(strcmp(argv[i], "-j") == 0)
		{
			++i;

			if(i < argc)
			{
				ANKI_CHECK(CString(argv[i]).toNumber(info.m_threadCount));
			}
			else
			{
				return Error::kUserData;
			}
		}
		else if(strcmp(argv[i], "-I") == 0)
		{
			++i;

			if(i < argc)
			{
				if(std::strlen(argv[i]) > 0)
				{
					info.m_includePath.sprintf("%s", argv[i]);
				}
				else
				{
					return Error::kUserData;
				}
			}
			else
			{
				return Error::kUserData;
			}
		}
		else if(CString(argv[i]).find("-D") == 0)
		{
			CString a = argv[i];
			if(a.getLength() < 5)
			{
				return Error::kUserData;
			}

			const String arg(a.getBegin() + 2, a.getEnd());
			StringList tokens;
			tokens.splitString(arg, '=');

			if(tokens.getSize() != 2)
			{
				return Error::kUserData;
			}

			info.m_defineNames.emplaceBack(tokens.getFront());

			I32 val;
			const Error err = (tokens.getBegin() + 1)->toNumber(val);
			if(err)
			{
				return Error::kUserData;
			}

			info.m_defines.emplaceBack(ShaderCompilerDefine{info.m_defineNames.getBack().toCString(), val});
		}
		else if(strcmp(argv[i], "-spirv") == 0)
		{
			info.m_spirv = true;
		}
		else if(strcmp(argv[i], "-dxil") == 0)
		{
			info.m_dxil = true;
		}
		else if(strcmp(argv[i], "-g") == 0)
		{
			info.m_debugInfo = true;
		}
		else if(strcmp(argv[i], "-sm") == 0)
		{
			++i;

			if(i < argc)
			{
				if(argv[i] == CString("6_7"))
				{
					info.m_sm = ShaderModel::k6_7;
				}
				else if(argv[i] == CString("6_8"))
				{
					info.m_sm = ShaderModel::k6_8;
				}
				else
				{
					return Error::kUserData;
				}
			}
			else
			{
				return Error::kUserData;
			}
		}
		else
		{
			return Error::kUserData;
		}
	}

	info.m_inputFname = argv[argc - 1];

	return Error::kNone;
}

static Error work(const CmdLineArgs& info)
{
	HeapMemoryPool pool(allocAligned, nullptr, "ProgramPool");

	// Load interface
	class FSystem : public ShaderCompilerFilesystemInterface
	{
	public:
		CString m_includePath;
		U32 m_fileReadCount = 0;

		Error readAllTextInternal(CString filename, ShaderCompilerString& txt)
		{
			String fname;

			// The first file is the input file. Don't append the include path to it
			if(m_fileReadCount == 0)
			{
				fname.sprintf("%s", filename.cstr());
			}
			else
			{
				fname.sprintf("%s/%s", m_includePath.cstr(), filename.cstr());
			}
			++m_fileReadCount;

			File file;
			ANKI_CHECK(file.open(fname, FileOpenFlag::kRead));
			ANKI_CHECK(file.readAllText(txt));
			return Error::kNone;
		}

		Error readAllText(CString filename, ShaderCompilerString& txt) final
		{
			const Error err = readAllTextInternal(filename, txt);
			if(err)
			{
				ANKI_LOGE("Failed to read file: %s", filename.cstr());
			}

			return err;
		}
	} fsystem;
	fsystem.m_includePath = info.m_includePath;

	// Threading interface
	class TaskManager : public ShaderCompilerAsyncTaskInterface
	{
	public:
		UniquePtr<ThreadJobManager, SingletonMemoryPoolDeleter<DefaultMemoryPool>> m_jobManager;

		void enqueueTask(void (*callback)(void* userData), void* userData) final
		{
			m_jobManager->dispatchTask([callback, userData]([[maybe_unused]] U32 threadIdx) {
				callback(userData);
			});
		}

		Error joinTasks()
		{
			m_jobManager->waitForAllTasksToFinish();
			return Error::kNone;
		}
	} taskManager;
	taskManager.m_jobManager.reset((info.m_threadCount) ? newInstance<ThreadJobManager>(DefaultMemoryPool::getSingleton(), info.m_threadCount, true)
														: nullptr);

	// Compile
	ShaderBinary* binary = nullptr;
	ANKI_CHECK(compileShaderProgram(info.m_inputFname, info.m_spirv, info.m_debugInfo, info.m_sm, fsystem, nullptr,
									(info.m_threadCount) ? &taskManager : nullptr, info.m_defines, binary));

	class Dummy
	{
	public:
		ShaderBinary* m_binary;

		~Dummy()
		{
			freeShaderBinary(m_binary);
		}
	} dummy{binary};

	// Store the binary
	{
		File file;
		ANKI_CHECK(file.open(info.m_outFname, FileOpenFlag::kWrite | FileOpenFlag::kBinary));

		BinarySerializer serializer;
		ANKI_CHECK(serializer.serialize(*binary, ShaderCompilerMemoryPool::getSingleton(), file));
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

	CmdLineArgs info;
	if(parseCommandLineArgs(argc, argv, info))
	{
		ANKI_LOGE(kUsage, argv[0]);
		return 1;
	}

	if(info.m_spirv == info.m_dxil)
	{
		ANKI_LOGE(kUsage, argv[0]);
		return 1;
	}

	if(info.m_outFname.isEmpty())
	{
		info.m_outFname = getFilename(info.m_inputFname);
		info.m_outFname += "bin";
	}

	if(info.m_includePath.isEmpty())
	{
		info.m_includePath = "./";
	}

	if(work(info))
	{
		ANKI_LOGE("Compilation failed");
		return 1;
	}

	return 0;
}
