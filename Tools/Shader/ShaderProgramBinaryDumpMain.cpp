// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
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
-stats     : Print performance statistics for all shaders. By default it doesn't
-no-binary : Don't print the binary
-no-glsl   : Don't print GLSL
-spirv     : Print SPIR-V
)";

static Error parseCommandLineArgs(WeakArray<char*> argv, Bool& dumpStats, Bool& dumpBinary, Bool& glsl, Bool& spirv,
								  StringRaii& filename)
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
			dumpStats = true;
		}
		else if(CString(argv[i]) == "-no-binary")
		{
			dumpBinary = false;
			dumpStats = true;
		}
		else if(CString(argv[i]) == "-no-glsl")
		{
			glsl = false;
		}
		else if(CString(argv[i]) == "-spirv")
		{
			spirv = true;
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
			F64 m_spillingCount;
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
				m_arm.m_workRegisters = m_arm.m_fp16ArithmeticPercentage = m_arm.m_spillingCount = v;

			m_amd.m_vgprCount = m_amd.m_sgprCount = m_amd.m_isaSize = v;
		}

		Stats()
			: Stats(0.0)
		{
		}

		void op(const Stats& b, void (*func)(F64& a, F64 b))
		{
			func(m_arm.m_fma, b.m_arm.m_fma);
			func(m_arm.m_cvt, b.m_arm.m_cvt);
			func(m_arm.m_sfu, b.m_arm.m_sfu);
			func(m_arm.m_loadStore, b.m_arm.m_loadStore);
			func(m_arm.m_varying, b.m_arm.m_varying);
			func(m_arm.m_texture, b.m_arm.m_texture);
			func(m_arm.m_workRegisters, b.m_arm.m_workRegisters);
			func(m_arm.m_fp16ArithmeticPercentage, b.m_arm.m_fp16ArithmeticPercentage);
			func(m_arm.m_spillingCount, b.m_arm.m_spillingCount);

			func(m_amd.m_vgprCount, b.m_amd.m_vgprCount);
			func(m_amd.m_sgprCount, b.m_amd.m_sgprCount);
			func(m_amd.m_isaSize, b.m_amd.m_isaSize);
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
		HeapMemoryPool* m_pool = nullptr;
		DynamicArrayRaii<Stats> m_spirvStats{m_pool};
		DynamicArrayRaii<Atomic<U32>> m_spirvVisited{m_pool};
		Atomic<U32> m_variantCount = {0};
		const ShaderProgramBinary* m_bin = nullptr;
		Atomic<I32> m_error = {0};

		Ctx(HeapMemoryPool* pool)
			: m_pool(pool)
		{
		}
	};

	Ctx ctx(&pool);
	ctx.m_bin = &bin;
	ctx.m_spirvStats.create(bin.m_codeBlocks.getSize());
	ctx.m_spirvVisited.create(bin.m_codeBlocks.getSize());
	memset(ctx.m_spirvVisited.getBegin(), 0, ctx.m_spirvVisited.getSizeInBytes());

	ThreadHive hive(getCpuCoresCount(), &pool);

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
				const U32 codeblockIdx = variant.m_codeBlockIndices[shaderType];

				if(codeblockIdx == kMaxU32)
				{
					continue;
				}

				const Bool visited = ctx.m_spirvVisited[codeblockIdx].fetchAdd(1) != 0;
				if(visited)
				{
					continue;
				}

				const ShaderProgramBinaryCodeBlock& codeBlock = ctx.m_bin->m_codeBlocks[codeblockIdx];

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
				RgaOutput rgaOut = {};
#if 1
				err = runRadeonGpuAnalyzer(
#	if ANKI_OS_LINUX
					ANKI_SOURCE_DIRECTORY "/ThirdParty/Bin/Linux64/RadeonGpuAnalyzer/rga",
#	elif ANKI_OS_WINDOWS
					ANKI_SOURCE_DIRECTORY "/ThirdParty/Bin/Windows64/RadeonGpuAnalyzer/rga.exe",
#	else
#		error "Not supported"
#	endif
					codeBlock.m_binary, shaderType, *ctx.m_pool, rgaOut);

				if(err)
				{
					ANKI_LOGE("Radeon GPU Analyzer compiler failed");
					ctx.m_error.store(1);
					break;
				}
#endif

				// Write stats
				Stats& stats = ctx.m_spirvStats[codeblockIdx];

				stats.m_arm.m_fma = maliocOut.m_fma;
				stats.m_arm.m_cvt = maliocOut.m_cvt;
				stats.m_arm.m_sfu = maliocOut.m_sfu;
				stats.m_arm.m_loadStore = maliocOut.m_loadStore;
				stats.m_arm.m_varying = maliocOut.m_varying;
				stats.m_arm.m_texture = maliocOut.m_texture;
				stats.m_arm.m_workRegisters = maliocOut.m_workRegisters;
				stats.m_arm.m_fp16ArithmeticPercentage = maliocOut.m_fp16ArithmeticPercentage;
				stats.m_arm.m_spillingCount = (maliocOut.m_spilling) ? 1.0 : 0.0;

				stats.m_amd.m_vgprCount = F64(rgaOut.m_vgprCount);
				stats.m_amd.m_sgprCount = F64(rgaOut.m_sgprCount);
				stats.m_amd.m_isaSize = F64(rgaOut.m_isaSize);
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

	// Cather the results
	Array<StageStats, U32(ShaderType::kCount)> allStageStats;
	for(const ShaderProgramBinaryVariant& variant : bin.m_variants)
	{
		for(ShaderType stage : EnumIterable<ShaderType>())
		{
			if(variant.m_codeBlockIndices[stage] == kMaxU32)
			{
				continue;
			}

			const Stats& stats = ctx.m_spirvStats[variant.m_codeBlockIndices[stage]];
			StageStats& allStats = allStageStats[stage];

			++allStats.m_count;

			allStats.m_avgStats.op(stats, [](F64& a, F64 b) {
				a += b;
			});

			allStats.m_minStats.op(stats, [](F64& a, F64 b) {
				a = min(a, b);
			});

			allStats.m_maxStats.op(stats, [](F64& a, F64 b) {
				a = max(a, b);
			});
		}
	}

	// Print
	for(ShaderType shaderType : EnumIterable<ShaderType>())
	{
		const StageStats& stage = allStageStats[shaderType];
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

Error dump(CString fname, Bool bDumpStats, Bool dumpBinary, Bool glsl, Bool spirv)
{
	HeapMemoryPool pool(allocAligned, nullptr);

	ShaderProgramBinaryWrapper binw(&pool);
	ANKI_CHECK(binw.deserializeFromFile(fname));

	if(dumpBinary)
	{
		ShaderDumpOptions options;
		options.m_writeGlsl = glsl;
		options.m_writeSpirv = spirv;

		StringRaii txt(&pool);
		dumpShaderProgramBinary(options, binw.getBinary(), txt);

		printf("%s\n", txt.cstr());
	}

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
