
// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Core/App.h>
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Util/File.h>
#include <AnKi/Util/Filesystem.h>
#include <AnKi/Util/System.h>
#include <AnKi/Util/ThreadHive.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Util/HighRezTimer.h>
#include <AnKi/Core/CoreTracer.h>
#include <AnKi/Core/DeveloperConsole.h>
#include <AnKi/Core/StatsUi.h>
#include <AnKi/Core/NativeWindow.h>
#include <AnKi/Core/MaliHwCounters.h>
#include <AnKi/Input/Input.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Physics/PhysicsWorld.h>
#include <AnKi/Renderer/MainRenderer.h>
#include <AnKi/Script/ScriptManager.h>
#include <AnKi/Resource/ResourceFilesystem.h>
#include <AnKi/Resource/AsyncLoader.h>
#include <AnKi/Core/GpuMemoryPools.h>
#include <AnKi/Ui/UiManager.h>
#include <AnKi/Ui/Canvas.h>
#include <csignal>

#if ANKI_OS_ANDROID
#	include <android_native_app_glue.h>
#endif

namespace anki {

#if ANKI_OS_ANDROID
/// The one and only android hack
android_app* g_androidApp = nullptr;
#endif

void* App::MemStats::allocCallback(void* userData, void* ptr, PtrSize size, [[maybe_unused]] PtrSize alignment)
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
		MemStats* self = static_cast<MemStats*>(userData);
		Header* allocation = static_cast<Header*>(
			self->m_originalAllocCallback(self->m_originalUserData, nullptr, newSize, newAlignment));
		allocation->m_allocatedSize = size;
		++allocation;
		out = static_cast<void*>(allocation);

		// Update stats
		self->m_allocatedMem.fetchAdd(size);
		self->m_allocCount.fetchAdd(1);
	}
	else
	{
		// Need to free

		MemStats* self = static_cast<MemStats*>(userData);

		Header* allocation = static_cast<Header*>(ptr);
		--allocation;
		ANKI_ASSERT(allocation->m_allocatedSize > 0);

		// Update stats
		self->m_freeCount.fetchAdd(1);
		self->m_allocatedMem.fetchSub(allocation->m_allocatedSize);

		// Free
		self->m_originalAllocCallback(self->m_originalUserData, allocation, 0, 0);
	}

	return out;
}

App::App()
{
}

App::~App()
{
	ANKI_CORE_LOGI("Destroying application");
	cleanup();
}

void App::cleanup()
{
	m_statsUi.reset(nullptr);
	m_console.reset(nullptr);

	deleteInstance(m_mainPool, m_scene);
	m_scene = nullptr;
	deleteInstance(m_mainPool, m_script);
	m_script = nullptr;
	deleteInstance(m_mainPool, m_renderer);
	m_renderer = nullptr;
	deleteInstance(m_mainPool, m_ui);
	m_ui = nullptr;
	deleteInstance(m_mainPool, m_resources);
	m_resources = nullptr;
	deleteInstance(m_mainPool, m_resourceFs);
	m_resourceFs = nullptr;
	deleteInstance(m_mainPool, m_physics);
	m_physics = nullptr;
	deleteInstance(m_mainPool, m_stagingMem);
	m_stagingMem = nullptr;
	deleteInstance(m_mainPool, m_unifiedGometryMemPool);
	m_unifiedGometryMemPool = nullptr;
	deleteInstance(m_mainPool, m_threadHive);
	m_threadHive = nullptr;
	deleteInstance(m_mainPool, m_maliHwCounters);
	m_maliHwCounters = nullptr;
	GrManager::deleteInstance(m_gr);
	m_gr = nullptr;
	Input::deleteInstance(m_input);
	m_input = nullptr;
	NativeWindow::deleteInstance(m_window);
	m_window = nullptr;

#if ANKI_ENABLE_TRACE
	m_heapAlloc.deleteInstance(m_coreTracer);
	m_coreTracer = nullptr;
#endif

	m_settingsDir.destroy(m_mainPool);
	m_cacheDir.destroy(m_mainPool);
}

Error App::init(ConfigSet* config, AllocAlignedCallback allocCb, void* allocCbUserData)
{
	ANKI_ASSERT(config);
	m_config = config;
	const Error err = initInternal(allocCb, allocCbUserData);
	if(err)
	{
		ANKI_CORE_LOGE("App initialization failed. Shutting down");
		cleanup();
	}

	return err;
}

Error App::initInternal(AllocAlignedCallback allocCb, void* allocCbUserData)
{
	LoggerSingleton::get().enableVerbosity(m_config->getCoreVerboseLog());

	setSignalHandlers();

	initMemoryCallbacks(allocCb, allocCbUserData);
	m_mainPool.init(allocCb, allocCbUserData, "Core");

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
#if ANKI_ENABLE_TRACE
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
		ANKI_CORE_LOGF(
			"AnKi is built with sse4.2 support but your CPU doesn't support it. Try bulding without SSE support");
	}
#endif

	ANKI_CORE_LOGI("Number of job threads: %u", m_config->getCoreJobThreadCount());

	//
	// Core tracer
	//
#if ANKI_ENABLE_TRACE
	m_coreTracer = m_heapAlloc.newInstance<CoreTracer>();
	ANKI_CHECK(m_coreTracer->init(m_heapAlloc, m_settingsDir));
#endif

	//
	// Window
	//
	NativeWindowInitInfo nwinit;
	nwinit.m_allocCallback = m_mainPool.getAllocationCallback();
	nwinit.m_allocCallbackUserData = m_mainPool.getAllocationCallbackUserData();
	nwinit.m_width = m_config->getWidth();
	nwinit.m_height = m_config->getHeight();
	nwinit.m_depthBits = 0;
	nwinit.m_stencilBits = 0;
	nwinit.m_fullscreenDesktopRez = m_config->getWindowFullscreen() > 0;
	nwinit.m_exclusiveFullscreen = m_config->getWindowFullscreen() == 2;
	ANKI_CHECK(NativeWindow::newInstance(nwinit, m_window));

	//
	// Input
	//
	ANKI_CHECK(Input::newInstance(m_mainPool.getAllocationCallback(), m_mainPool.getAllocationCallbackUserData(),
								  m_window, m_input));

	//
	// ThreadPool
	//
	m_threadHive = newInstance<ThreadHive>(m_mainPool, m_config->getCoreJobThreadCount(), &m_mainPool, true);

	//
	// Graphics API
	//
	GrManagerInitInfo grInit;
	grInit.m_allocCallback = m_mainPool.getAllocationCallback();
	grInit.m_allocCallbackUserData = m_mainPool.getAllocationCallbackUserData();
	grInit.m_cacheDirectory = m_cacheDir.toCString();
	grInit.m_config = m_config;
	grInit.m_window = m_window;

	ANKI_CHECK(GrManager::newInstance(grInit, m_gr));

	//
	// Mali HW counters
	//
	if(m_gr->getDeviceCapabilities().m_gpuVendor == GpuVendor::kArm && m_config->getCoreMaliHwCounters())
	{
		m_maliHwCounters = newInstance<MaliHwCounters>(m_mainPool, &m_mainPool);
	}

	//
	// GPU mem
	//
	m_unifiedGometryMemPool = newInstance<UnifiedGeometryMemoryPool>(m_mainPool);
	ANKI_CHECK(m_unifiedGometryMemPool->init(&m_mainPool, m_gr, *m_config));

	m_stagingMem = newInstance<StagingGpuMemoryPool>(m_mainPool);
	ANKI_CHECK(m_stagingMem->init(m_gr, *m_config));

	//
	// Physics
	//
	m_physics = newInstance<PhysicsWorld>(m_mainPool);
	ANKI_CHECK(m_physics->init(m_mainPool.getAllocationCallback(), m_mainPool.getAllocationCallbackUserData()));

	//
	// Resource FS
	//
#if !ANKI_OS_ANDROID
	// Add the location of the executable where the shaders are supposed to be
	StringRaii executableFname(&m_mainPool);
	ANKI_CHECK(getApplicationPath(executableFname));
	ANKI_CORE_LOGI("Executable path is: %s", executableFname.cstr());
	StringRaii shadersPath(&m_mainPool);
	getParentFilepath(executableFname, shadersPath);
	shadersPath.append(":");
	shadersPath.append(m_config->getRsrcDataPaths());
	m_config->setRsrcDataPaths(shadersPath);
#endif

	m_resourceFs = newInstance<ResourceFilesystem>(m_mainPool);
	ANKI_CHECK(
		m_resourceFs->init(*m_config, m_mainPool.getAllocationCallback(), m_mainPool.getAllocationCallbackUserData()));

	//
	// Resources
	//
	ResourceManagerInitInfo rinit;
	rinit.m_gr = m_gr;
	rinit.m_physics = m_physics;
	rinit.m_resourceFs = m_resourceFs;
	rinit.m_unifiedGometryMemoryPool = m_unifiedGometryMemPool;
	rinit.m_config = m_config;
	rinit.m_allocCallback = m_mainPool.getAllocationCallback();
	rinit.m_allocCallbackData = m_mainPool.getAllocationCallbackUserData();
	m_resources = newInstance<ResourceManager>(m_mainPool);

	ANKI_CHECK(m_resources->init(rinit));

	//
	// UI
	//
	m_ui = newInstance<UiManager>(m_mainPool);
	ANKI_CHECK(m_ui->init(m_mainPool.getAllocationCallback(), m_mainPool.getAllocationCallbackUserData(), m_resources,
						  m_gr, m_stagingMem, m_input));

	//
	// Renderer
	//
	MainRendererInitInfo renderInit;
	renderInit.m_swapchainSize = UVec2(m_window->getWidth(), m_window->getHeight());
	renderInit.m_allocCallback = m_mainPool.getAllocationCallback();
	renderInit.m_allocCallbackUserData = m_mainPool.getAllocationCallbackUserData();
	renderInit.m_threadHive = m_threadHive;
	renderInit.m_resourceManager = m_resources;
	renderInit.m_gr = m_gr;
	renderInit.m_stagingMemory = m_stagingMem;
	renderInit.m_ui = m_ui;
	renderInit.m_config = m_config;
	renderInit.m_globTimestamp = &m_globalTimestamp;
	m_renderer = newInstance<MainRenderer>(m_mainPool);
	ANKI_CHECK(m_renderer->init(renderInit));

	//
	// Script
	//
	m_script = newInstance<ScriptManager>(m_mainPool);
	ANKI_CHECK(m_script->init(m_mainPool.getAllocationCallback(), m_mainPool.getAllocationCallbackUserData()));

	//
	// Scene
	//
	m_scene = newInstance<SceneGraph>(m_mainPool);

	ANKI_CHECK(m_scene->init(m_mainPool.getAllocationCallback(), m_mainPool.getAllocationCallbackUserData(),
							 m_threadHive, m_resources, m_input, m_script, m_ui, m_config, &m_globalTimestamp));

	// Inform the script engine about some subsystems
	m_script->setRenderer(m_renderer);
	m_script->setSceneGraph(m_scene);

	//
	// Misc
	//
	ANKI_CHECK(m_ui->newInstance<StatsUi>(m_statsUi));
	ANKI_CHECK(m_ui->newInstance<DeveloperConsole>(m_console, m_script));

	ANKI_CORE_LOGI("Application initialized");

	return Error::kNone;
}

Error App::initDirs()
{
	// Settings path
#if !ANKI_OS_ANDROID
	StringRaii home(&m_mainPool);
	ANKI_CHECK(getHomeDirectory(home));

	m_settingsDir.sprintf(m_mainPool, "%s/.anki", &home[0]);
#else
	m_settingsDir.sprintf(m_mainPool, "%s/.anki", g_androidApp->activity->internalDataPath);
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
	m_cacheDir.sprintf(m_mainPool, "%s/cache", &m_settingsDir[0]);

	const Bool cacheDirExists = directoryExists(m_cacheDir.toCString());
	if(m_config->getCoreClearCaches() && cacheDirExists)
	{
		ANKI_CORE_LOGI("Will delete the cache dir and start fresh: %s", &m_cacheDir[0]);
		ANKI_CHECK(removeDirectory(m_cacheDir.toCString(), m_mainPool));
		ANKI_CHECK(createDirectory(m_cacheDir.toCString()));
	}
	else if(!cacheDirExists)
	{
		ANKI_CORE_LOGI("Will create cache dir: %s", &m_cacheDir[0]);
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

	while(!quit)
	{
		{
			ANKI_TRACE_SCOPED_EVENT(FRAME);
			const Second startTime = HighRezTimer::getCurrentTime();

			prevUpdateTime = crntTime;
			crntTime = HighRezTimer::getCurrentTime();

			// Update
			ANKI_CHECK(m_input->handleEvents());

			// User update
			ANKI_CHECK(userMainLoop(quit, crntTime - prevUpdateTime));

			ANKI_CHECK(m_scene->update(prevUpdateTime, crntTime));

			RenderQueue rqueue;
			m_scene->doVisibilityTests(rqueue);

			// Inject stats UI
			DynamicArrayRaii<UiQueueElement> newUiElementArr(&m_mainPool);
			injectUiElements(newUiElementArr, rqueue);

			// Render
			TexturePtr presentableTex = m_gr->acquireNextPresentableTexture();
			m_renderer->setStatsEnabled(m_config->getCoreDisplayStats() > 0
#if ANKI_ENABLE_TRACE
										|| TracerSingleton::get().getEnabled()
#endif
			);
			ANKI_CHECK(m_renderer->render(rqueue, presentableTex));

			// Pause and sync async loader. That will force all tasks before the pause to finish in this frame.
			m_resources->getAsyncLoader().pause();

			m_gr->swapBuffers();
			m_stagingMem->endFrame();

			// Update the trace info with some async loader stats
			U64 asyncTaskCount = m_resources->getAsyncLoader().getCompletedTaskCount();
			ANKI_TRACE_INC_COUNTER(RESOURCE_ASYNC_TASKS, asyncTaskCount - m_resourceCompletedAsyncTaskCount);
			m_resourceCompletedAsyncTaskCount = asyncTaskCount;

			// Now resume the loader
			m_resources->getAsyncLoader().resume();

			// Sleep
			const Second endTime = HighRezTimer::getCurrentTime();
			const Second frameTime = endTime - startTime;
			const Second timerTick = 1.0 / Second(m_config->getCoreTargetFps());
			if(frameTime < timerTick)
			{
				ANKI_TRACE_SCOPED_EVENT(TIMER_TICK_SLEEP);
				HighRezTimer::sleep(timerTick - frameTime);
			}

			// Stats
			if(m_config->getCoreDisplayStats() > 0)
			{
				StatsUiInput in;
				in.m_cpuFrameTime = frameTime;
				in.m_rendererTime = m_renderer->getStats().m_renderingCpuTime;
				in.m_sceneUpdateTime = m_scene->getStats().m_updateTime;
				in.m_visibilityTestsTime = m_scene->getStats().m_visibilityTestsTime;
				in.m_physicsTime = m_scene->getStats().m_physicsUpdate;

				in.m_gpuFrameTime = m_renderer->getStats().m_renderingGpuTime;

				if(m_maliHwCounters)
				{
					MaliHwCountersOut out;
					m_maliHwCounters->sample(out);
					in.m_gpuActiveCycles = out.m_gpuActive;
					in.m_gpuReadBandwidth = out.m_readBandwidth;
					in.m_gpuWriteBandwidth = out.m_writeBandwidth;
				}

				in.m_cpuAllocatedMemory = m_memStats.m_allocatedMem.load();
				in.m_cpuAllocationCount = m_memStats.m_allocCount.load();
				in.m_cpuFreeCount = m_memStats.m_freeCount.load();

				const GrManagerStats grStats = m_gr->getStats();
				BuddyAllocatorBuilderStats vertMemStats;
				m_unifiedGometryMemPool->getMemoryStats(vertMemStats);

				in.m_gpuDeviceMemoryAllocated = grStats.m_deviceMemoryAllocated;
				in.m_gpuDeviceMemoryInUse = grStats.m_deviceMemoryInUse;
				in.m_globalVertexAllocated = vertMemStats.m_realAllocatedSize;
				in.m_globalVertexUsed = vertMemStats.m_userAllocatedSize;
				in.m_globalVertexExternalFragmentation = vertMemStats.m_externalFragmentation;

				in.m_drawableCount = rqueue.countAllRenderables();
				in.m_vkCommandBufferCount = grStats.m_commandBufferCount;

				StatsUi& statsUi = *static_cast<StatsUi*>(m_statsUi.get());
				const StatsUiDetail detail =
					(m_config->getCoreDisplayStats() == 1) ? StatsUiDetail::kFpsOnly : StatsUiDetail::kDetailed;
				statsUi.setStats(in, detail);
			}

#if ANKI_ENABLE_TRACE
			if(m_renderer->getStats().m_renderingGpuTime >= 0.0)
			{
				ANKI_TRACE_CUSTOM_EVENT(GPU_TIME, m_renderer->getStats().m_renderingGpuSubmitTimestamp,
										m_renderer->getStats().m_renderingGpuTime);
			}
#endif

			++m_globalTimestamp;
		}

#if ANKI_ENABLE_TRACE
		static U64 frame = 1;
		m_coreTracer->flushFrame(frame++);
#endif
	}

	return Error::kNone;
}

void App::injectUiElements(DynamicArrayRaii<UiQueueElement>& newUiElementArr, RenderQueue& rqueue)
{
	const U32 originalCount = rqueue.m_uis.getSize();
	if(m_config->getCoreDisplayStats() > 0 || m_consoleEnabled)
	{
		const U32 extraElements = (m_config->getCoreDisplayStats() > 0) + (m_consoleEnabled != 0);
		newUiElementArr.create(originalCount + extraElements);

		if(originalCount > 0)
		{
			memcpy(&newUiElementArr[0], &rqueue.m_uis[0], rqueue.m_uis.getSizeInBytes());
		}

		rqueue.m_uis = WeakArray<UiQueueElement>(newUiElementArr);
	}

	U32 count = originalCount;
	if(m_config->getCoreDisplayStats() > 0)
	{
		newUiElementArr[count].m_userData = m_statsUi.get();
		newUiElementArr[count].m_drawCallback = [](CanvasPtr& canvas, void* userData) -> void {
			static_cast<StatsUi*>(userData)->build(canvas);
		};
		++count;
	}

	if(m_consoleEnabled)
	{
		newUiElementArr[count].m_userData = m_console.get();
		newUiElementArr[count].m_drawCallback = [](CanvasPtr& canvas, void* userData) -> void {
			static_cast<DeveloperConsole*>(userData)->build(canvas);
		};
		++count;
	}
}

void App::initMemoryCallbacks(AllocAlignedCallback& allocCb, void*& allocCbUserData)
{
	if(m_config->getCoreDisplayStats() > 1)
	{
		m_memStats.m_originalAllocCallback = allocCb;
		m_memStats.m_originalUserData = allocCbUserData;

		allocCb = MemStats::allocCallback;
		allocCbUserData = &m_memStats;
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
		HeapMemoryPool pool(allocAligned, nullptr);
		backtrace(pool, [&count](CString symbol) {
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

} // end namespace anki
