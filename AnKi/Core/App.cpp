
// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Core/App.h>
#include <AnKi/Util/CVarSet.h>
#include <AnKi/GpuMemory/UnifiedGeometryBuffer.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Util/File.h>
#include <AnKi/Util/Filesystem.h>
#include <AnKi/Util/System.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Util/HighRezTimer.h>
#include <AnKi/Core/CoreTracer.h>
#include <AnKi/GpuMemory/RebarTransientMemoryPool.h>
#include <AnKi/GpuMemory/GpuVisibleTransientMemoryPool.h>
#include <AnKi/GpuMemory/GpuReadbackMemoryPool.h>
#include <AnKi/Core/StatsSet.h>
#include <AnKi/Window/NativeWindow.h>
#include <AnKi/Core/MaliHwCounters.h>
#include <AnKi/Window/Input.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Physics/PhysicsWorld.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Script/ScriptManager.h>
#include <AnKi/Resource/ResourceFilesystem.h>
#include <AnKi/Resource/AsyncLoader.h>
#include <AnKi/Ui/UiManager.h>
#include <AnKi/Ui/UiCanvas.h>
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

ANKI_SVAR(CpuAllocatedMem, StatCategory::kCpuMem, "Total", StatFlag::kBytes)
ANKI_SVAR(CpuAllocationCount, StatCategory::kCpuMem, "Allocations/frame", StatFlag::kBytes | StatFlag::kZeroEveryFrame)
ANKI_SVAR(CpuFreesCount, StatCategory::kCpuMem, "Frees/frame", StatFlag::kBytes | StatFlag::kZeroEveryFrame)

#if ANKI_PLATFORM_MOBILE
ANKI_SVAR(MaliGpuActive, StatCategory::kGpuMisc, "Mali active cycles", StatFlag::kMainThreadUpdates)
ANKI_SVAR(MaliGpuReadBandwidth, StatCategory::kGpuMisc, "Mali read bandwidth", StatFlag::kMainThreadUpdates)
ANKI_SVAR(MaliGpuWriteBandwidth, StatCategory::kGpuMisc, "Mali write bandwidth", StatFlag::kMainThreadUpdates)
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
		g_svarCpuAllocatedMem.increment(size);
		g_svarCpuAllocationCount.increment(1);
	}
	else
	{
		// Need to free

		App* self = static_cast<App*>(userData);

		Header* allocation = static_cast<Header*>(ptr);
		--allocation;
		ANKI_ASSERT(allocation->m_allocatedSize > 0);

		// Update stats
		g_svarCpuAllocatedMem.decrement(allocation->m_allocatedSize);
		g_svarCpuFreesCount.increment(1);

		// Free
		self->m_originalAllocCallback(self->m_originalAllocUserData, allocation, 0, 0);
	}

	return out;
}

App::App(CString appName, AllocAlignedCallback allocCb, void* allocCbUserData)
{
	m_originalAllocCallback = allocCb;
	m_originalAllocUserData = allocCbUserData;

	if(ANKI_STATS_ENABLED && g_cvarCoreDisplayStats > 1)
	{
		m_allocCallback = statsAllocCallback;
		m_allocUserData = this;
	}
	else
	{
		m_allocCallback = m_originalAllocCallback;
		m_allocUserData = m_originalAllocUserData = allocCbUserData;
	}

	DefaultMemoryPool::allocateSingleton(m_allocCallback, m_allocUserData);
	CoreMemoryPool::allocateSingleton(m_allocCallback, m_allocUserData);

	if(appName.isEmpty())
	{
		appName = "UnnamedApp";
	}
	m_appName = appName;
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
	Renderer::freeSingleton();
	UiManager::freeSingleton();
	GpuSceneMicroPatcher::freeSingleton();
	ResourceManager::freeSingleton();
	PhysicsWorld::freeSingleton();
	RebarTransientMemoryPool::freeSingleton();
	GpuVisibleTransientMemoryPool::freeSingleton();
	UnifiedGeometryBuffer::freeSingleton();
	GpuSceneBuffer::freeSingleton();
	GpuReadbackMemoryPool::freeSingleton();
	CoreThreadJobManager::freeSingleton();
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
	m_appName.destroy();

	CoreMemoryPool::freeSingleton();
	DefaultMemoryPool::freeSingleton();

	ANKI_CORE_LOGI("Application finished shutting down");
}

Error App::init()
{
	StatsSet::getSingleton().initFromMainThread();
	Logger::getSingleton().enableVerbosity(g_cvarCoreVerboseLog);

	ANKI_CHECK(initDirs());

	ANKI_CORE_LOGI("Initializing application. Build config: %s", kAnKiBuildConfigString);

// Check SIMD support
#if ANKI_SIMD_SSE && ANKI_COMPILER_GCC_COMPATIBLE
	if(!__builtin_cpu_supports("sse4.2"))
	{
		ANKI_CORE_LOGF("AnKi is built with sse4.2 support but your CPU doesn't support it. Try bulding without SSE support");
	}
#endif

	ANKI_CORE_LOGI("Number of job threads: %u", U32(g_cvarCoreJobThreadCount));

	if(g_cvarCoreBenchmarkMode && g_cvarGrVsync)
	{
		ANKI_CORE_LOGW("Vsync is enabled and benchmark mode as well. Will turn vsync off");
		g_cvarGrVsync = false;
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
	NativeWindow::allocateSingleton();
	ANKI_CHECK(NativeWindow::getSingleton().init(g_cvarCoreTargetFps, "AnKi"));

	//
	// Input
	//
	Input::allocateSingleton();
	ANKI_CHECK(Input::getSingleton().init());

	//
	// ThreadPool
	//
	const Bool pinThreads = !ANKI_OS_ANDROID;
	CoreThreadJobManager::allocateSingleton(U32(g_cvarCoreJobThreadCount), pinThreads);

	//
	// Graphics API
	//
	GrManagerInitInfo grInit;
	grInit.m_allocCallback = m_allocCallback;
	grInit.m_allocCallbackUserData = m_allocUserData;
	grInit.m_cacheDirectory = m_cacheDir.toCString();
	ANKI_CHECK(GrManager::allocateSingleton().init(grInit));

	//
	// Mali HW counters
	//
#if ANKI_PLATFORM_MOBILE
	if(ANKI_STATS_ENABLED && GrManager::getSingleton().getDeviceCapabilities().m_gpuVendor == GpuVendor::kArm && g_cvarCoreMaliHwCounters)
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
	ANKI_CHECK(PhysicsWorld::getSingleton().init(m_allocCallback, m_allocUserData));

	//
	// Resources
	//
#if !ANKI_OS_ANDROID
	// Add the location of the executable where the shaders are supposed to be
	String executableFname;
	ANKI_CHECK(getApplicationPath(executableFname));
	ANKI_CORE_LOGI("Executable path is: %s", executableFname.cstr());
	String extraPaths;
	getParentFilepath(executableFname, extraPaths);
	extraPaths += "|ankiprogbin"; // Shaders
	extraPaths += ":" ANKI_SOURCE_DIRECTORY "|EngineAssets,!AndroidProject"; // EngineAssets
	extraPaths += ":";
	extraPaths += g_cvarRsrcDataPaths;
	g_cvarRsrcDataPaths = extraPaths;
#endif

	ANKI_CHECK(ResourceManager::allocateSingleton().init(m_allocCallback, m_allocUserData));

	//
	// UI
	//
	ANKI_CHECK(UiManager::allocateSingleton().init(m_allocCallback, m_allocUserData));

	//
	// GPU scene
	//
	ANKI_CHECK(GpuSceneMicroPatcher::allocateSingleton().init());

	//
	// Renderer
	//
	RendererInitInfo renderInit;
	renderInit.m_swapchainSize = UVec2(NativeWindow::getSingleton().getWidth(), NativeWindow::getSingleton().getHeight());
	renderInit.m_allocCallback = m_allocCallback;
	renderInit.m_allocCallbackUserData = m_allocUserData;
	ANKI_CHECK(Renderer::allocateSingleton().init(renderInit));

	//
	// Script
	//
	ScriptManager::allocateSingleton(m_allocCallback, m_allocUserData);

	//
	// Scene
	//
	ANKI_CHECK(SceneGraph::allocateSingleton().init(m_allocCallback, m_allocUserData));

	GrManager::getSingleton().finish();
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
	if(g_cvarCoreClearCaches && cacheDirExists)
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
	// Initialize the application
	Error err = Error::kNone;
	if((err = userPreInit()))
	{
		ANKI_CORE_LOGE("User initialization failed. Shutting down");
		cleanup();
		return err;
	}

	if((err = init()))
	{
		ANKI_CORE_LOGE("App initialization failed. Shutting down");
		cleanup();
		return err;
	}

	if((err = userPostInit()))
	{
		ANKI_CORE_LOGE("User initialization failed. Shutting down");
		cleanup();
		return err;
	}

	// Continue with the main loop
	ANKI_CORE_LOGI("Entering main loop");
	Bool quit = false;

	Second prevUpdateTime = HighRezTimer::getCurrentTime();
	Second crntTime = prevUpdateTime;

	// Benchmark mode stuff:
	const Bool benchmarkMode = g_cvarCoreBenchmarkMode;
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

			ANKI_CHECK(Input::getSingleton().handleEvents());
			GrManager::getSingleton().beginFrame();

			ANKI_CHECK(userMainLoop(quit, crntTime - prevUpdateTime));
			SceneGraph::getSingleton().update(prevUpdateTime, crntTime);

			ANKI_CHECK(Renderer::getSingleton().render());

			// If we get stats exclude the time of GR because it forces some GPU-CPU serialization. We don't want to count that
			Second grTime = 0.0;
			if(benchmarkMode || g_cvarCoreDisplayStats > 0) [[unlikely]]
			{
				grTime = HighRezTimer::getCurrentTime();
			}

			GrManager::getSingleton().endFrame();

			if(benchmarkMode || g_cvarCoreDisplayStats > 0) [[unlikely]]
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
			g_svarCpuTotalTime.set((frameTime - grTime) * 1000.0);
			if(!benchmarkMode) [[likely]]
			{
				const Second timerTick = 1.0_sec / Second(g_cvarCoreTargetFps);
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
				g_svarMaliGpuActive.set(out.m_gpuActive);
				g_svarMaliGpuReadBandwidth.set(out.m_readBandwidth);
				g_svarMaliGpuWriteBandwidth.set(out.m_writeBandwidth);
			}
#endif

			StatsSet::getSingleton().endFrame();

			++GlobalFrameIndex::getSingleton().m_value;

			if(benchmarkMode) [[unlikely]]
			{
				if(GlobalFrameIndex::getSingleton().m_value >= g_cvarCoreBenchmarkModeFrameCount)
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

Bool App::toggleDeveloperConsole()
{
	SceneNode& node = SceneGraph::getSingleton().findSceneNode("_DevConsole");
	static_cast<DeveloperConsoleUiNode&>(node).toggleConsole();
	m_consoleEnabled = static_cast<DeveloperConsoleUiNode&>(node).isConsoleEnabled();
	return m_consoleEnabled;
}

} // end namespace anki
