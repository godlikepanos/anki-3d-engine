
// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Core/App.h>
#include <AnKi/Core/CVarSet.h>
#include <AnKi/Core/GpuMemory/UnifiedGeometryBuffer.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Util/File.h>
#include <AnKi/Util/Filesystem.h>
#include <AnKi/Util/System.h>
#include <AnKi/Util/ThreadHive.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Util/HighRezTimer.h>
#include <AnKi/Core/CoreTracer.h>
#include <AnKi/Core/GpuMemory/RebarTransientMemoryPool.h>
#include <AnKi/Core/GpuMemory/GpuVisibleTransientMemoryPool.h>
#include <AnKi/Core/GpuMemory/GpuReadbackMemoryPool.h>
#include <AnKi/Core/StatsSet.h>
#include <AnKi/Window/NativeWindow.h>
#include <AnKi/Core/MaliHwCounters.h>
#include <AnKi/Window/Input.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Physics/PhysicsWorld.h>
#include <AnKi/Renderer/MainRenderer.h>
#include <AnKi/Script/ScriptManager.h>
#include <AnKi/Resource/ResourceFilesystem.h>
#include <AnKi/Resource/AsyncLoader.h>
#include <AnKi/Ui/UiManager.h>
#include <AnKi/Ui/Canvas.h>
#include <AnKi/Scene/DeveloperConsoleUiNode.h>
#include <csignal>

#if ANKI_OS_ANDROID
#	include <android_native_app_glue.h>
#endif

namespace anki {

#if ANKI_OS_ANDROID
/// The one and only android hack
android_app* g_androidApp = nullptr;
#endif

StatCounter g_cpuTotalTime(StatCategory::kTime, "CPU total", StatFlag::kMilisecond | StatFlag::kShowAverage);
static StatCounter g_cpuAllocatedMem(StatCategory::kCpuMem, "Total", StatFlag::kBytes | StatFlag::kThreadSafe);
static StatCounter g_cpuAllocationCount(StatCategory::kCpuMem, "Allocations/frame",
										StatFlag::kBytes | StatFlag::kZeroEveryFrame | StatFlag::kThreadSafe);
static StatCounter g_cpuFreesCount(StatCategory::kCpuMem, "Frees/frame", StatFlag::kBytes | StatFlag::kZeroEveryFrame | StatFlag::kThreadSafe);

NumericCVar<U32> g_windowWidthCVar(CVarSubsystem::kCore, "Width", 1920, 16, 16 * 1024, "Width");
NumericCVar<U32> g_windowHeightCVar(CVarSubsystem::kCore, "Height", 1080, 16, 16 * 1024, "Height");
NumericCVar<U32> g_windowFullscreenCVar(CVarSubsystem::kCore, "WindowFullscreen", 1, 0, 2,
										"0: windowed, 1: borderless fullscreen, 2: exclusive fullscreen");
NumericCVar<U32> g_targetFpsCVar(CVarSubsystem::kCore, "TargetFps", 60u, 1u, kMaxU32, "Target FPS");
static NumericCVar<U32> g_jobThreadCountCVar(CVarSubsystem::kCore, "JobThreadCount", max(2u, getCpuCoresCount() / 2u), 2u, 1024u,
											 "Number of job thread");
NumericCVar<U32> g_displayStatsCVar(CVarSubsystem::kCore, "DisplayStats", 0, 0, 2, "Display stats, 0: None, 1: Simple, 2: Detailed");
BoolCVar g_clearCachesCVar(CVarSubsystem::kCore, "ClearCaches", false, "Clear all caches");
BoolCVar g_verboseLogCVar(CVarSubsystem::kCore, "VerboseLog", false, "Verbose logging");
BoolCVar g_benchmarkModeCVar(CVarSubsystem::kCore, "BenchmarkMode", false, "Run in a benchmark mode. Fixed timestep, unlimited target FPS");
NumericCVar<U32> g_benchmarkModeFrameCountCVar(CVarSubsystem::kCore, "BenchmarkModeFrameCount", 60 * 60 * 2, 1, kMaxU32,
											   "How many frames the benchmark will run before it quits");

NumericCVar<F32> g_lod0MaxDistanceCVar(CVarSubsystem::kCore, "Lod0MaxDistance", 20.0f, 1.0f, kMaxF32,
									   "Distance that will be used to calculate the LOD 0");
NumericCVar<F32> g_lod1MaxDistanceCVar(CVarSubsystem::kCore, "Lod1MaxDistance", 40.0f, 2.0f, kMaxF32,
									   "Distance that will be used to calculate the LOD 1");

#if ANKI_PLATFORM_MOBILE
static StatCounter g_maliGpuActive(StatCategory::kGpuMisc, "Mali active cycles");
static StatCounter g_maliGpuReadBandwidth(StatCategory::kGpuMisc, "Mali read bandwidth");
static StatCounter g_maliGpuWriteBandwidth(StatCategory::kGpuMisc, "Mali write bandwidth");

static BoolCVar g_maliHwCountersCVar(CVarSubsystem::kCore, "MaliHwCounters", false, "Enable Mali counters");
#endif

void* App::statsAllocCallback(void* userData, void* ptr, PtrSize size, [[maybe_unused]] PtrSize alignment)
{
	ANKI_ASSERT(userData);

	constexpr PtrSize kMaxAlignment = 64;

	struct alignas(kMaxAlignment) Header
	{
		PtrSize m_allocatedSize;
		Array<U8, kMaxAlignment - sizeof(PtrSize)> _m_padding;
	};
	static_assert(sizeof(Header) == kMaxAlignment, "See file");
	static_assert(alignof(Header) == kMaxAlignment, "See file");

	void* out = nullptr;

	if(ptr == nullptr)
	{
		// Need to allocate
		ANKI_ASSERT(size > 0);
		ANKI_ASSERT(alignment > 0 && alignment <= kMaxAlignment);

		const PtrSize newAlignment = kMaxAlignment;
		const PtrSize newSize = sizeof(Header) + size;

		// Allocate
		App* self = static_cast<App*>(userData);
		Header* allocation = static_cast<Header*>(self->m_originalAllocCallback(self->m_originalAllocUserData, nullptr, newSize, newAlignment));
		allocation->m_allocatedSize = size;
		++allocation;
		out = static_cast<void*>(allocation);

		// Update stats
		g_cpuAllocatedMem.atomicIncrement(size);
		g_cpuAllocationCount.atomicIncrement(1);
	}
	else
	{
		// Need to free

		App* self = static_cast<App*>(userData);

		Header* allocation = static_cast<Header*>(ptr);
		--allocation;
		ANKI_ASSERT(allocation->m_allocatedSize > 0);

		// Update stats
		g_cpuAllocatedMem.atomicDecrement(allocation->m_allocatedSize);
		g_cpuFreesCount.atomicIncrement(1);

		// Free
		self->m_originalAllocCallback(self->m_originalAllocUserData, allocation, 0, 0);
	}

	return out;
}

App::App(AllocAlignedCallback allocCb, void* allocCbUserData)
{
	m_originalAllocCallback = allocCb;
	m_originalAllocUserData = allocCbUserData;
}

App::~App()
{
	ANKI_CORE_LOGI("Destroying application");
	cleanup();
}

void App::cleanup()
{
	SceneGraph::freeSingleton();
	ScriptManager::freeSingleton();
	MainRenderer::freeSingleton();
	UiManager::freeSingleton();
	GpuSceneMicroPatcher::freeSingleton();
	ResourceManager::freeSingleton();
	PhysicsWorld::freeSingleton();
	RebarTransientMemoryPool::freeSingleton();
	GpuVisibleTransientMemoryPool::freeSingleton();
	UnifiedGeometryBuffer::freeSingleton();
	GpuSceneBuffer::freeSingleton();
	GpuReadbackMemoryPool::freeSingleton();
	CoreThreadHive::freeSingleton();
	MaliHwCounters::freeSingleton();
	GrManager::freeSingleton();
	Input::freeSingleton();
	NativeWindow::freeSingleton();

#if ANKI_TRACING_ENABLED
	CoreTracer::freeSingleton();
#endif

	GlobalFrameIndex::freeSingleton();

	m_settingsDir.destroy();
	m_cacheDir.destroy();

	CoreMemoryPool::freeSingleton();
	DefaultMemoryPool::freeSingleton();
}

Error App::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_CORE_LOGE("App initialization failed. Shutting down");
		cleanup();
	}

	return err;
}

Error App::initInternal()
{
	Logger::getSingleton().enableVerbosity(g_verboseLogCVar.get());

	setSignalHandlers();

	AllocAlignedCallback allocCb = m_originalAllocCallback;
	void* allocCbUserData = m_originalAllocUserData;
	initMemoryCallbacks(allocCb, allocCbUserData);

	DefaultMemoryPool::allocateSingleton(allocCb, allocCbUserData);
	CoreMemoryPool::allocateSingleton(allocCb, allocCbUserData);

	ANKI_CHECK(initDirs());

	// Print a message
	const char* buildType =
#if ANKI_OPTIMIZE
		"optimized, "
#else
		"NOT optimized, "
#endif
#if ANKI_DEBUG_SYMBOLS
		"dbg symbols, "
#else
		"NO dbg symbols, "
#endif
#if ANKI_EXTRA_CHECKS
		"extra checks, "
#else
		"NO extra checks, "
#endif
#if ANKI_TRACING_ENABLED
		"built with tracing";
#else
		"NOT built with tracing";
#endif

	ANKI_CORE_LOGI("Initializing application ("
				   "version %u.%u, "
				   "%s, "
				   "compiler %s, "
				   "build date %s, "
				   "commit %s)",
				   ANKI_VERSION_MAJOR, ANKI_VERSION_MINOR, buildType, ANKI_COMPILER_STR, __DATE__, ANKI_REVISION);

// Check SIMD support
#if ANKI_SIMD_SSE && ANKI_COMPILER_GCC_COMPATIBLE
	if(!__builtin_cpu_supports("sse4.2"))
	{
		ANKI_CORE_LOGF("AnKi is built with sse4.2 support but your CPU doesn't support it. Try bulding without SSE support");
	}
#endif

	ANKI_CORE_LOGI("Number of job threads: %u", g_jobThreadCountCVar.get());

	if(g_benchmarkModeCVar.get() && g_vsyncCVar.get())
	{
		ANKI_CORE_LOGW("Vsync is enabled and benchmark mode as well. Will turn vsync off");
		g_vsyncCVar.set(false);
	}

	GlobalFrameIndex::allocateSingleton();

	//
	// Core tracer
	//
#if ANKI_TRACING_ENABLED
	ANKI_CHECK(CoreTracer::allocateSingleton().init(m_settingsDir));
#endif

	//
	// Window
	//
	NativeWindowInitInfo nwinit;
	nwinit.m_width = g_windowWidthCVar.get();
	nwinit.m_height = g_windowHeightCVar.get();
	nwinit.m_depthBits = 0;
	nwinit.m_stencilBits = 0;
	nwinit.m_fullscreenDesktopRez = g_windowFullscreenCVar.get() > 0;
	nwinit.m_exclusiveFullscreen = g_windowFullscreenCVar.get() == 2;
	nwinit.m_targetFps = g_targetFpsCVar.get();
	NativeWindow::allocateSingleton();
	ANKI_CHECK(NativeWindow::getSingleton().init(nwinit));

	//
	// Input
	//
	Input::allocateSingleton();
	ANKI_CHECK(Input::getSingleton().init());

	//
	// ThreadPool
	//
	const Bool pinThreads = !ANKI_OS_ANDROID;
	CoreThreadHive::allocateSingleton(g_jobThreadCountCVar.get(), pinThreads);

	//
	// Graphics API
	//
	GrManagerInitInfo grInit;
	grInit.m_allocCallback = allocCb;
	grInit.m_allocCallbackUserData = allocCbUserData;
	grInit.m_cacheDirectory = m_cacheDir.toCString();
	ANKI_CHECK(GrManager::allocateSingleton().init(grInit));

	//
	// Mali HW counters
	//
#if ANKI_PLATFORM_MOBILE
	if(ANKI_STATS_ENABLED && GrManager::getSingleton().getDeviceCapabilities().m_gpuVendor == GpuVendor::kArm && g_maliHwCountersCVar.get())
	{
		MaliHwCounters::allocateSingleton();
	}
#endif

	//
	// GPU mem
	//
	UnifiedGeometryBuffer::allocateSingleton().init();
	GpuSceneBuffer::allocateSingleton().init();
	RebarTransientMemoryPool::allocateSingleton().init();
	GpuVisibleTransientMemoryPool::allocateSingleton();
	GpuReadbackMemoryPool::allocateSingleton();

	//
	// Physics
	//
	PhysicsWorld::allocateSingleton();
	ANKI_CHECK(PhysicsWorld::getSingleton().init(allocCb, allocCbUserData));

	//
	// Resources
	//
#if !ANKI_OS_ANDROID
	// Add the location of the executable where the shaders are supposed to be
	String executableFname;
	ANKI_CHECK(getApplicationPath(executableFname));
	ANKI_CORE_LOGI("Executable path is: %s", executableFname.cstr());
	String shadersPath;
	getParentFilepath(executableFname, shadersPath);
	shadersPath += ":";
	shadersPath += g_dataPathsCVar.get();
	g_dataPathsCVar.set(shadersPath);
#endif

	ANKI_CHECK(ResourceManager::allocateSingleton().init(allocCb, allocCbUserData));

	//
	// UI
	//
	ANKI_CHECK(UiManager::allocateSingleton().init(allocCb, allocCbUserData));

	//
	// GPU scene
	//
	ANKI_CHECK(GpuSceneMicroPatcher::allocateSingleton().init());

	//
	// Renderer
	//
	MainRendererInitInfo renderInit;
	renderInit.m_swapchainSize = UVec2(NativeWindow::getSingleton().getWidth(), NativeWindow::getSingleton().getHeight());
	renderInit.m_allocCallback = allocCb;
	renderInit.m_allocCallbackUserData = allocCbUserData;
	ANKI_CHECK(MainRenderer::allocateSingleton().init(renderInit));

	//
	// Script
	//
	ScriptManager::allocateSingleton(allocCb, allocCbUserData);

	//
	// Scene
	//
	ANKI_CHECK(SceneGraph::allocateSingleton().init(allocCb, allocCbUserData));

	ANKI_CORE_LOGI("Application initialized");

	return Error::kNone;
}

Error App::initDirs()
{
	// Settings path
#if !ANKI_OS_ANDROID
	String home;
	ANKI_CHECK(getHomeDirectory(home));

	m_settingsDir.sprintf("%s/.anki", &home[0]);
#else
	m_settingsDir.sprintf("%s/.anki", g_androidApp->activity->internalDataPath);
#endif

	if(!directoryExists(m_settingsDir.toCString()))
	{
		ANKI_CORE_LOGI("Creating settings dir \"%s\"", &m_settingsDir[0]);
		ANKI_CHECK(createDirectory(m_settingsDir.toCString()));
	}
	else
	{
		ANKI_CORE_LOGI("Using settings dir \"%s\"", &m_settingsDir[0]);
	}

	// Cache
	m_cacheDir.sprintf("%s/cache", &m_settingsDir[0]);

	const Bool cacheDirExists = directoryExists(m_cacheDir.toCString());
	if(g_clearCachesCVar.get() && cacheDirExists)
	{
		ANKI_CORE_LOGI("Will delete the cache dir and start fresh: %s", m_cacheDir.cstr());
		ANKI_CHECK(removeDirectory(m_cacheDir.toCString()));
		ANKI_CHECK(createDirectory(m_cacheDir.toCString()));
	}
	else if(!cacheDirExists)
	{
		ANKI_CORE_LOGI("Will create cache dir: %s", m_cacheDir.cstr());
		ANKI_CHECK(createDirectory(m_cacheDir.toCString()));
	}

	return Error::kNone;
}

Error App::mainLoop()
{
	ANKI_CORE_LOGI("Entering main loop");
	Bool quit = false;

	Second prevUpdateTime = HighRezTimer::getCurrentTime();
	Second crntTime = prevUpdateTime;

	// Benchmark mode stuff:
	const Bool benchmarkMode = g_benchmarkModeCVar.get();
	Second aggregatedCpuTime = 0.0;
	Second aggregatedGpuTime = 0.0;
	constexpr U32 kBenchmarkFramesToGatherBeforeFlush = 60;
	U32 benchmarkFramesGathered = 0;
	File benchmarkCsvFile;
	CoreString benchmarkCsvFileFilename;
	if(benchmarkMode)
	{
		benchmarkCsvFileFilename.sprintf("%s/Benchmark.csv", m_settingsDir.cstr());
		ANKI_CHECK(benchmarkCsvFile.open(benchmarkCsvFileFilename, FileOpenFlag::kWrite));
		ANKI_CHECK(benchmarkCsvFile.writeText("CPU, GPU\n"));
	}

	while(!quit)
	{
		{
			ANKI_TRACE_SCOPED_EVENT(Frame);
			const Second startTime = HighRezTimer::getCurrentTime();

			prevUpdateTime = crntTime;
			crntTime = (!benchmarkMode) ? HighRezTimer::getCurrentTime() : (prevUpdateTime + 1.0_sec / 60.0_sec);

			// Update
			ANKI_CHECK(Input::getSingleton().handleEvents());

			// User update
			ANKI_CHECK(userMainLoop(quit, crntTime - prevUpdateTime));

			ANKI_CHECK(SceneGraph::getSingleton().update(prevUpdateTime, crntTime));

			RenderQueue rqueue;
			SceneGraph::getSingleton().doVisibilityTests(rqueue);

			// Render
			TexturePtr presentableTex = GrManager::getSingleton().acquireNextPresentableTexture();
			ANKI_CHECK(MainRenderer::getSingleton().render(rqueue, presentableTex.get()));

			// If we get stats exclude the time of GR because it forces some GPU-CPU serialization. We don't want to count that
			Second grTime = 0.0;
			if(benchmarkMode || g_displayStatsCVar.get() > 0) [[unlikely]]
			{
				grTime = HighRezTimer::getCurrentTime();
			}

			GrManager::getSingleton().swapBuffers();

			if(benchmarkMode || g_displayStatsCVar.get() > 0) [[unlikely]]
			{
				grTime = HighRezTimer::getCurrentTime() - grTime;
			}

			RebarTransientMemoryPool::getSingleton().endFrame();
			UnifiedGeometryBuffer::getSingleton().endFrame();
			GpuSceneBuffer::getSingleton().endFrame();
			GpuVisibleTransientMemoryPool::getSingleton().endFrame();
			GpuReadbackMemoryPool::getSingleton().endFrame();

			// Sleep
			const Second endTime = HighRezTimer::getCurrentTime();
			const Second frameTime = endTime - startTime;
			g_cpuTotalTime.set((frameTime - grTime) * 1000.0);
			if(!benchmarkMode) [[likely]]
			{
				const Second timerTick = 1.0_sec / Second(g_targetFpsCVar.get());
				if(frameTime < timerTick)
				{
					ANKI_TRACE_SCOPED_EVENT(TimerTickSleep);
					HighRezTimer::sleep(timerTick - frameTime);
				}
			}
			// Benchmark stats
			else
			{
				aggregatedCpuTime += frameTime - grTime;
				aggregatedGpuTime += 0; // TODO
				++benchmarkFramesGathered;
				if(benchmarkFramesGathered >= kBenchmarkFramesToGatherBeforeFlush)
				{
					aggregatedCpuTime = aggregatedCpuTime / Second(kBenchmarkFramesToGatherBeforeFlush) * 1000.0;
					aggregatedGpuTime = aggregatedGpuTime / Second(kBenchmarkFramesToGatherBeforeFlush) * 1000.0;
					ANKI_CHECK(benchmarkCsvFile.writeTextf("%f,%f\n", aggregatedCpuTime, aggregatedGpuTime));

					benchmarkFramesGathered = 0;
					aggregatedCpuTime = 0.0;
					aggregatedGpuTime = 0.0;
				}
			}

			// Stats
#if ANKI_PLATFORM_MOBILE
			if(MaliHwCounters::isAllocated())
			{
				MaliHwCountersOut out;
				MaliHwCounters::getSingleton().sample(out);
				g_maliGpuActive.set(out.m_gpuActive);
				g_maliGpuReadBandwidth.set(out.m_readBandwidth);
				g_maliGpuWriteBandwidth.set(out.m_writeBandwidth);
			}
#endif

			StatsSet::getSingleton().endFrame();

			++GlobalFrameIndex::getSingleton().m_value;

			if(benchmarkMode) [[unlikely]]
			{
				if(GlobalFrameIndex::getSingleton().m_value >= g_benchmarkModeFrameCountCVar.get())
				{
					quit = true;
				}
			}
		}

#if ANKI_TRACING_ENABLED
		static U64 frame = 1;
		CoreTracer::getSingleton().flushFrame(frame++);
#endif
	}

	if(benchmarkMode) [[unlikely]]
	{
		ANKI_CORE_LOGI("Benchmark file saved in: %s", benchmarkCsvFileFilename.cstr());
	}

	return Error::kNone;
}

void App::initMemoryCallbacks(AllocAlignedCallback& allocCb, void*& allocCbUserData)
{
	if(ANKI_STATS_ENABLED && g_displayStatsCVar.get() > 1)
	{
		allocCb = statsAllocCallback;
		allocCbUserData = this;
	}
	else
	{
		// Leave the default
	}
}

void App::setSignalHandlers()
{
	auto handler = [](int signum) -> void {
		const char* name = nullptr;
		switch(signum)
		{
		case SIGABRT:
			name = "SIGABRT";
			break;
		case SIGSEGV:
			name = "SIGSEGV";
			break;
#if ANKI_POSIX
		case SIGBUS:
			name = "SIGBUS";
			break;
#endif
		case SIGILL:
			name = "SIGILL";
			break;
		case SIGFPE:
			name = "SIGFPE";
			break;
		}

		if(name)
			printf("Caught signal %d (%s)\n", signum, name);
		else
			printf("Caught signal %d\n", signum);

		U32 count = 0;
		printf("Backtrace:\n");
		backtrace([&count](CString symbol) {
			printf("%.2u: %s\n", count++, symbol.cstr());
		});

		ANKI_DEBUG_BREAK();
	};

	signal(SIGSEGV, handler);
	signal(SIGILL, handler);
	signal(SIGFPE, handler);
#if ANKI_POSIX
	signal(SIGBUS, handler);
#endif
	// Ignore for now: signal(SIGABRT, handler);
}

Bool App::toggleDeveloperConsole()
{
	SceneNode& node = SceneGraph::getSingleton().findSceneNode("_DevConsole");
	static_cast<DeveloperConsoleUiNode&>(node).toggleConsole();
	m_consoleEnabled = static_cast<DeveloperConsoleUiNode&>(node).isConsoleEnabled();
	return m_consoleEnabled;
}

} // end namespace anki
