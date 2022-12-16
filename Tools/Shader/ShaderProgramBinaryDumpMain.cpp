// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/ShaderCompiler/ShaderProgramCompiler.h>
#include <AnKi/ShaderCompiler/MaliOfflineCompiler.h>
#include <AnKi/ShaderCompiler/RadeonGpuAnalyzer.h>
#include <AnKi/Util/ThreadHive.h>
#include <AnKi/Util/System.h>

using namespace anki;

static const char* kUsage = R"(Dump the shader binary to stdout
Usage: %s [options] input_shader_program_binary
Options:
-stats : Print performance statistics for all shaders. By default it doesn't
)";

static Error parseCommandLineArgs(WeakArray<char*> argv, Bool& dumpStats, StringRaii& filename)
{
	// Parse config
	if(argv.getSize() < 2)
	{
		return Error::kUserData;
	}

	dumpStats = false;
	filename = argv[argv.getSize() - 1];

	for(U32 i = 1; i < argv.getSize() - 1; i++)
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

	printf("\nOffline compilers stats:\n");
	fflush(stdout);

	class Stats
	{
	public:
		class
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
		} m_arm;

		class
		{
		public:
			F64 m_vgprCount;
			F64 m_sgprCount;
			F64 m_isaSize;
		} m_amd;

		Stats(F64 v)
		{
			m_arm.m_fma = m_arm.m_cvt = m_arm.m_sfu = m_arm.m_loadStore = m_arm.m_varying = m_arm.m_texture =
				m_arm.m_workRegisters = m_arm.m_fp16ArithmeticPercentage = v;

			m_amd.m_vgprCount = m_amd.m_sgprCount = m_amd.m_isaSize = v;
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

	class Ctx
	{
	public:
		Array<StageStats, U32(ShaderType::kCount)> m_allStats;
		Mutex m_allStatsMtx;
		Atomic<U32> m_variantCount = {0};
		HeapMemoryPool* m_pool = nullptr;
		const ShaderProgramBinary* m_bin = nullptr;
		Atomic<I32> m_error = {0};
	};

	Ctx ctx;
	ctx.m_pool = &pool;
	ctx.m_bin = &bin;

	ThreadHive hive(8, &pool);

	ThreadHiveTaskCallback callback = [](void* userData, [[maybe_unused]] U32 threadId,
										 [[maybe_unused]] ThreadHive& hive,
										 [[maybe_unused]] ThreadHiveSemaphore* signalSemaphore) {
		Ctx& ctx = *static_cast<Ctx*>(userData);
		U32 variantIdx;

		while((variantIdx = ctx.m_variantCount.fetchAdd(1)) < ctx.m_bin->m_variants.getSize()
			  && ctx.m_error.load() == 0)
		{
			const ShaderProgramBinaryVariant& variant = ctx.m_bin->m_variants[variantIdx];

			for(ShaderType shaderType : EnumIterable<ShaderType>())
			{
				if(variant.m_codeBlockIndices[shaderType] == kMaxU32)
				{
					continue;
				}

				const ShaderProgramBinaryCodeBlock& codeBlock =
					ctx.m_bin->m_codeBlocks[variant.m_codeBlockIndices[shaderType]];

				// Arm stats
				MaliOfflineCompilerOut maliocOut;
				Error err = runMaliOfflineCompiler(
#if ANKI_OS_LINUX
					ANKI_SOURCE_DIRECTORY "/ThirdParty/Bin/Linux64/MaliOfflineCompiler/malioc",
#elif ANKI_OS_WINDOWS
					ANKI_SOURCE_DIRECTORY "/ThirdParty/Bin/Windows64/MaliOfflineCompiler/malioc.exe",
#else
#	error "Not supported"
#endif
					codeBlock.m_binary, shaderType, *ctx.m_pool, maliocOut);

				if(err)
				{
					ANKI_LOGE("Mali offline compiler failed");
					ctx.m_error.store(1);
					break;
				}

				// AMD
				RgaOutput rgaOut;
				err = runRadeonGpuAnalyzer(
#if ANKI_OS_LINUX
					ANKI_SOURCE_DIRECTORY "/ThirdParty/Bin/Linux64/RadeonGpuAnalyzer/rga",
#elif ANKI_OS_WINDOWS
					ANKI_SOURCE_DIRECTORY "/ThirdParty/Bin/Windows64/RadeonGpuAnalyzer/rga.exe",
#else
#	error "Not supported"
#endif
					codeBlock.m_binary, shaderType, *ctx.m_pool, rgaOut);

				if(err)
				{
					ANKI_LOGE("Radeon GPU Analyzer compiler failed");
					ctx.m_error.store(1);
					break;
				}

				// Appends stats
				LockGuard lock(ctx.m_allStatsMtx);

				StageStats& stage = ctx.m_allStats[shaderType];

				if(maliocOut.m_spilling)
				{
					++stage.m_spillingCount;
				}

				++stage.m_count;

				stage.m_avgStats.m_arm.m_fma += maliocOut.m_fma;
				stage.m_avgStats.m_arm.m_cvt += maliocOut.m_cvt;
				stage.m_avgStats.m_arm.m_sfu += maliocOut.m_sfu;
				stage.m_avgStats.m_arm.m_loadStore += maliocOut.m_loadStore;
				stage.m_avgStats.m_arm.m_varying += maliocOut.m_varying;
				stage.m_avgStats.m_arm.m_texture += maliocOut.m_texture;
				stage.m_avgStats.m_arm.m_workRegisters += maliocOut.m_workRegisters;
				stage.m_avgStats.m_arm.m_fp16ArithmeticPercentage += maliocOut.m_fp16ArithmeticPercentage;

				stage.m_maxStats.m_arm.m_fma = max<F64>(stage.m_maxStats.m_arm.m_fma, maliocOut.m_fma);
				stage.m_maxStats.m_arm.m_cvt = max<F64>(stage.m_maxStats.m_arm.m_cvt, maliocOut.m_cvt);
				stage.m_maxStats.m_arm.m_sfu = max<F64>(stage.m_maxStats.m_arm.m_sfu, maliocOut.m_sfu);
				stage.m_maxStats.m_arm.m_loadStore =
					max<F64>(stage.m_maxStats.m_arm.m_loadStore, maliocOut.m_loadStore);
				stage.m_maxStats.m_arm.m_varying = max<F64>(stage.m_maxStats.m_arm.m_varying, maliocOut.m_varying);
				stage.m_maxStats.m_arm.m_texture = max<F64>(stage.m_maxStats.m_arm.m_texture, maliocOut.m_texture);
				stage.m_maxStats.m_arm.m_workRegisters =
					max<F64>(stage.m_maxStats.m_arm.m_workRegisters, maliocOut.m_workRegisters);
				stage.m_maxStats.m_arm.m_fp16ArithmeticPercentage =
					max<F64>(stage.m_maxStats.m_arm.m_fp16ArithmeticPercentage, maliocOut.m_fp16ArithmeticPercentage);

				stage.m_minStats.m_arm.m_fma = min<F64>(stage.m_minStats.m_arm.m_fma, maliocOut.m_fma);
				stage.m_minStats.m_arm.m_cvt = min<F64>(stage.m_minStats.m_arm.m_cvt, maliocOut.m_cvt);
				stage.m_minStats.m_arm.m_sfu = min<F64>(stage.m_minStats.m_arm.m_sfu, maliocOut.m_sfu);
				stage.m_minStats.m_arm.m_loadStore =
					min<F64>(stage.m_minStats.m_arm.m_loadStore, maliocOut.m_loadStore);
				stage.m_minStats.m_arm.m_varying = min<F64>(stage.m_minStats.m_arm.m_varying, maliocOut.m_varying);
				stage.m_minStats.m_arm.m_texture = min<F64>(stage.m_minStats.m_arm.m_texture, maliocOut.m_texture);
				stage.m_minStats.m_arm.m_workRegisters =
					min<F64>(stage.m_minStats.m_arm.m_workRegisters, maliocOut.m_workRegisters);
				stage.m_minStats.m_arm.m_fp16ArithmeticPercentage =
					min<F64>(stage.m_minStats.m_arm.m_fp16ArithmeticPercentage, maliocOut.m_fp16ArithmeticPercentage);

				stage.m_avgStats.m_amd.m_vgprCount += F64(rgaOut.m_vgprCount);
				stage.m_avgStats.m_amd.m_sgprCount += F64(rgaOut.m_sgprCount);
				stage.m_avgStats.m_amd.m_isaSize += F64(rgaOut.m_isaSize);

				stage.m_minStats.m_amd.m_vgprCount = min(stage.m_minStats.m_amd.m_vgprCount, F64(rgaOut.m_vgprCount));
				stage.m_minStats.m_amd.m_sgprCount = min(stage.m_minStats.m_amd.m_sgprCount, F64(rgaOut.m_sgprCount));
				stage.m_minStats.m_amd.m_isaSize = min(stage.m_minStats.m_amd.m_isaSize, F64(rgaOut.m_isaSize));

				stage.m_maxStats.m_amd.m_vgprCount = max(stage.m_maxStats.m_amd.m_vgprCount, F64(rgaOut.m_vgprCount));
				stage.m_maxStats.m_amd.m_sgprCount = max(stage.m_maxStats.m_amd.m_sgprCount, F64(rgaOut.m_sgprCount));
				stage.m_maxStats.m_amd.m_isaSize = max(stage.m_maxStats.m_amd.m_isaSize, F64(rgaOut.m_isaSize));
			}

			if(variantIdx > 0 && ((variantIdx + 1) % 32) == 0)
			{
				printf("Processed %u out of %u variants\n", variantIdx + 1, ctx.m_bin->m_variants.getSize());
			}
		} // while
	};

	for(U32 i = 0; i < hive.getThreadCount(); ++i)
	{
		hive.submitTask(callback, &ctx);
	}

	hive.waitAllTasks();

	if(ctx.m_error.load() != 0)
	{
		return Error::kFunctionFailed;
	}

	for(ShaderType shaderType : EnumIterable<ShaderType>())
	{
		const StageStats& stage = ctx.m_allStats[shaderType];
		if(stage.m_count == 0)
		{
			continue;
		}

		printf("Stage %u\n", U32(shaderType));
		printf("  Arm shaders spilling regs %u\n", stage.m_spillingCount);

		const F64 countf = F64(stage.m_count);

		const Stats& avg = stage.m_avgStats;
		printf("  Average:\n");
		printf("    Arm: Regs %f FMA %f CVT %f SFU %f LS %f VAR %f TEX %f FP16 %f%%\n",
			   avg.m_arm.m_workRegisters / countf, avg.m_arm.m_fma / countf, avg.m_arm.m_cvt / countf,
			   avg.m_arm.m_sfu / countf, avg.m_arm.m_loadStore / countf, avg.m_arm.m_varying / countf,
			   avg.m_arm.m_texture / countf, avg.m_arm.m_fp16ArithmeticPercentage / countf);
		printf("    AMD: VGPR %f SGPR %f ISA size %f\n", avg.m_amd.m_vgprCount / countf, avg.m_amd.m_sgprCount / countf,
			   avg.m_amd.m_isaSize / countf);

		const Stats& maxs = stage.m_maxStats;
		printf("  Max:\n");
		printf("    Arm: Regs %f FMA %f CVT %f SFU %f LS %f VAR %f TEX %f FP16 %f%%\n", maxs.m_arm.m_workRegisters,
			   maxs.m_arm.m_fma, maxs.m_arm.m_cvt, maxs.m_arm.m_sfu, maxs.m_arm.m_loadStore, maxs.m_arm.m_varying,
			   maxs.m_arm.m_texture, maxs.m_arm.m_fp16ArithmeticPercentage);
		printf("    AMD: VGPR %f SGPR %f ISA size %f\n", maxs.m_amd.m_vgprCount, maxs.m_amd.m_sgprCount,
			   maxs.m_amd.m_isaSize);
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
	if(parseCommandLineArgs(WeakArray<char*>(argv, argc), dumpStats, filename))
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
