// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/ShaderCompiler/ShaderProgramCompiler.h>
#include <AnKi/ShaderCompiler/MaliOfflineCompiler.h>

using namespace anki;

static const char* kUsage = R"(Dump the shader binary to stdout
Usage: %s [options] input_shader_program_binary
Options:
-stats : Print performance statistics for all shaders. By default it doesn't
)";

static Error parseCommandLineArgs(int argc, char** argv, Bool& dumpStats, StringRaii& filename)
{
	// Parse config
	if(argc < 2)
	{
		return Error::kUserData;
	}

	dumpStats = false;
	filename = argv[argc - 1];

	for(I i = 1; i < argc - 1; i++)
	{
		if(strcmp(argv[i], "-stats") == 0)
		{
			dumpStats = true;
		}
	}

	return Error::kNone;
}

Error dumpStats(const ShaderProgramBinary& bin)
{
	HeapMemoryPool pool(allocAligned, nullptr);

	printf("\nMali offline compiler stats:\n");
	fflush(stdout);

	class Stats
	{
	public:
		F64 m_fma;
		F64 m_cvt;
		F64 m_sfu;
		F64 m_loadStore;
		F64 m_varying;
		F64 m_texture;
		F64 m_workRegisters;
		F64 m_fp16ArithmeticPercentage;

		Stats(F64 v)
		{
			m_fma = m_cvt = m_sfu = m_loadStore = m_varying = m_texture = m_workRegisters = m_fp16ArithmeticPercentage =
				v;
		}
	};

	class StageStats
	{
	public:
		Stats m_avgStats{0.0};
		Stats m_maxStats{-1.0};
		Stats m_minStats{kMaxF64};
		U32 m_spillingCount = 0;
		U32 m_count = 0;
	};

	Array<StageStats, U32(ShaderType::kCount)> allStats;

	for(const ShaderProgramBinaryVariant& variant : bin.m_variants)
	{
		for(ShaderType shaderType : EnumIterable<ShaderType>())
		{
			if(variant.m_codeBlockIndices[shaderType] == kMaxU32)
			{
				continue;
			}

			const ShaderProgramBinaryCodeBlock& codeBlock = bin.m_codeBlocks[variant.m_codeBlockIndices[shaderType]];

			MaliOfflineCompilerOut maliocOut;
			const Error err = runMaliOfflineCompiler(
#if ANKI_OS_LINUX
				ANKI_SOURCE_DIRECTORY "/ThirdParty/Bin/Linux64/MaliOfflineCompiler/malioc",
#elif ANKI_OS_WINDOWS
				ANKI_SOURCE_DIRECTORY "/ThirdParty/Bin/Linux64/MaliOfflineCompiler/malioc.exe",
#else
#	error "Not supported"
#endif
				codeBlock.m_binary, shaderType, pool, maliocOut);

			if(err)
			{
				ANKI_LOGE("Mali offline compiler failed");
				return Error::kFunctionFailed;
			}

			// Appends stats
			StageStats& stage = allStats[shaderType];

			if(maliocOut.m_spilling)
			{
				++stage.m_spillingCount;
			}

			++stage.m_count;

			stage.m_avgStats.m_fma += maliocOut.m_fma;
			stage.m_avgStats.m_cvt += maliocOut.m_cvt;
			stage.m_avgStats.m_sfu += maliocOut.m_sfu;
			stage.m_avgStats.m_loadStore += maliocOut.m_loadStore;
			stage.m_avgStats.m_varying += maliocOut.m_varying;
			stage.m_avgStats.m_texture += maliocOut.m_texture;
			stage.m_avgStats.m_workRegisters += maliocOut.m_workRegisters;
			stage.m_avgStats.m_fp16ArithmeticPercentage += maliocOut.m_fp16ArithmeticPercentage;

			stage.m_maxStats.m_fma = max<F64>(stage.m_maxStats.m_fma, maliocOut.m_fma);
			stage.m_maxStats.m_cvt = max<F64>(stage.m_maxStats.m_cvt, maliocOut.m_cvt);
			stage.m_maxStats.m_sfu = max<F64>(stage.m_maxStats.m_sfu, maliocOut.m_sfu);
			stage.m_maxStats.m_loadStore = max<F64>(stage.m_maxStats.m_loadStore, maliocOut.m_loadStore);
			stage.m_maxStats.m_varying = max<F64>(stage.m_maxStats.m_varying, maliocOut.m_varying);
			stage.m_maxStats.m_texture = max<F64>(stage.m_maxStats.m_texture, maliocOut.m_texture);
			stage.m_maxStats.m_workRegisters = max<F64>(stage.m_maxStats.m_workRegisters, maliocOut.m_workRegisters);
			stage.m_maxStats.m_fp16ArithmeticPercentage =
				max<F64>(stage.m_maxStats.m_fp16ArithmeticPercentage, maliocOut.m_fp16ArithmeticPercentage);

			stage.m_minStats.m_fma = min<F64>(stage.m_minStats.m_fma, maliocOut.m_fma);
			stage.m_minStats.m_cvt = min<F64>(stage.m_minStats.m_cvt, maliocOut.m_cvt);
			stage.m_minStats.m_sfu = min<F64>(stage.m_minStats.m_sfu, maliocOut.m_sfu);
			stage.m_minStats.m_loadStore = min<F64>(stage.m_minStats.m_loadStore, maliocOut.m_loadStore);
			stage.m_minStats.m_varying = min<F64>(stage.m_minStats.m_varying, maliocOut.m_varying);
			stage.m_minStats.m_texture = min<F64>(stage.m_minStats.m_texture, maliocOut.m_texture);
			stage.m_minStats.m_workRegisters = min<F64>(stage.m_minStats.m_workRegisters, maliocOut.m_workRegisters);
			stage.m_minStats.m_fp16ArithmeticPercentage =
				min<F64>(stage.m_minStats.m_fp16ArithmeticPercentage, maliocOut.m_fp16ArithmeticPercentage);
		}
	}

	for(ShaderType shaderType : EnumIterable<ShaderType>())
	{
		const StageStats& stage = allStats[shaderType];
		if(stage.m_count == 0)
		{
			continue;
		}

		printf("Stage %u\n", U32(shaderType));
		printf("\tSpilling count %u\n", stage.m_spillingCount);

		const Stats& avg = stage.m_avgStats;
		printf("\tAvarage: Regs %f FMA %f CVT %f SFU %f LS %f VAR %f TEX %f FP16 %f%%\n",
			   avg.m_workRegisters / F64(stage.m_count), avg.m_fma / F64(stage.m_count), avg.m_cvt / F64(stage.m_count),
			   avg.m_sfu / F64(stage.m_count), avg.m_loadStore / F64(stage.m_count), avg.m_varying / F64(stage.m_count),
			   avg.m_texture / F64(stage.m_count), avg.m_fp16ArithmeticPercentage / F64(stage.m_count));

		const Stats& maxs = stage.m_maxStats;
		printf("\tMax: Regs %f FMA %f CVT %f SFU %f LS %f VAR %f TEX %f FP16 %f%%\n", maxs.m_workRegisters, maxs.m_fma,
			   maxs.m_cvt, maxs.m_sfu, maxs.m_loadStore, maxs.m_varying, maxs.m_texture,
			   maxs.m_fp16ArithmeticPercentage);
	}

	return Error::kNone;
}

Error dump(CString fname, Bool bDumpStats)
{
	HeapMemoryPool pool(allocAligned, nullptr);

	ShaderProgramBinaryWrapper binw(&pool);
	ANKI_CHECK(binw.deserializeFromFile(fname));

	StringRaii txt(&pool);
	dumpShaderProgramBinary(binw.getBinary(), txt);

	printf("%s\n", txt.cstr());

	if(bDumpStats)
	{
		ANKI_CHECK(dumpStats(binw.getBinary()));
	}

	return Error::kNone;
}

int main(int argc, char** argv)
{
	HeapMemoryPool pool(allocAligned, nullptr);
	StringRaii filename(&pool);
	Bool dumpStats;
	if(parseCommandLineArgs(argc, argv, dumpStats, filename))
	{
		ANKI_LOGE(kUsage, argv[0]);
		return 1;
	}

	const Error err = dump(filename, dumpStats);
	if(err)
	{
		ANKI_LOGE("Can't dump due to an error. Bye");
		return 1;
	}

	return 0;
}
