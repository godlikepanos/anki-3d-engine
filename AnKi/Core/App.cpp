// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
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
#include <AnKi/Core/NativeWindow.h>
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

class App::StatsUi
{
public:
	template<typename T>
	class BufferedValue
	{
	public:
		void set(T x)
		{
			m_total += x;
			++m_count;
		}

		F64 get(Bool flush)
		{
			if(flush)
			{
				m_avg = F64(m_total) / m_count;
				m_count = 0;
				m_total = 0.0;
			}

			return m_avg;
		}

	private:
		T m_total = T(0);
		F64 m_avg = 0.0;
		U32 m_count = 0;
	};

	GenericMemoryPoolAllocator<U8> m_alloc;

	BufferedValue<Second> m_frameTime;
	BufferedValue<Second> m_renderTime;
	BufferedValue<Second> m_sceneUpdateTime;
	BufferedValue<Second> m_visTestsTime;
	BufferedValue<Second> m_physicsTime;
	BufferedValue<Second> m_gpuTime;

	PtrSize m_allocatedCpuMem = 0;
	U64 m_allocCount = 0;
	U64 m_freeCount = 0;

	U64 m_vkCpuMem = 0;
	U64 m_vkGpuMem = 0;
	U32 m_vkCmdbCount = 0;

	PtrSize m_drawableCount = 0;

	static const U32 BUFFERED_FRAMES = 16;
	U32 m_bufferedFrames = 0;

	StatsUi(const GenericMemoryPoolAllocator<U8>& alloc)
		: m_alloc(alloc)
	{
	}

	void build(CanvasPtr canvas)
	{
		// Misc
		++m_bufferedFrames;
		Bool flush = false;
		if(m_bufferedFrames == BUFFERED_FRAMES)
		{
			flush = true;
			m_bufferedFrames = 0;
		}

		// Start drawing the UI
		canvas->pushFont(canvas->getDefaultFont(), 16);

		const Vec4 oldWindowColor = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
		ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w = 0.3f;

		if(ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::SetWindowPos(Vec2(5.0f, 5.0f));
			ImGui::SetWindowSize(Vec2(230.0f, 450.0f));

			ImGui::Text("CPU Time:");
			labelTime(m_frameTime.get(flush), "Total frame");
			labelTime(m_renderTime.get(flush), "Renderer");
			labelTime(m_sceneUpdateTime.get(flush), "Scene update");
			labelTime(m_visTestsTime.get(flush), "Visibility");
			labelTime(m_physicsTime.get(flush), "Physics");

			ImGui::Text("----");
			ImGui::Text("GPU Time:");
			labelTime(m_gpuTime.get(flush), "Total frame");

			ImGui::Text("----");
			ImGui::Text("Memory:");
			labelBytes(m_allocatedCpuMem, "Total CPU");
			labelUint(m_allocCount, "Total allocations");
			labelUint(m_freeCount, "Total frees");
			labelBytes(m_vkCpuMem, "Vulkan CPU");
			labelBytes(m_vkGpuMem, "Vulkan GPU");

			ImGui::Text("----");
			ImGui::Text("Vulkan:");
			labelUint(m_vkCmdbCount, "Cmd buffers");

			ImGui::Text("----");
			ImGui::Text("Other:");
			labelUint(m_drawableCount, "Drawbles");
		}

		ImGui::End();
		ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = oldWindowColor;

		canvas->popFont();
	}

	void labelTime(Second val, CString name)
	{
		ImGui::Text("%s: %fms", name.cstr(), val * 1000.0);
	}

	void labelBytes(PtrSize val, CString name)
	{
		PtrSize gb, mb, kb, b;

		gb = val / 1_GB;
		val -= gb * 1_GB;

		mb = val / 1_MB;
		val -= mb * 1_MB;

		kb = val / 1_KB;
		val -= kb * 1_KB;

		b = val;

		StringAuto timestamp(m_alloc);
		if(gb)
		{
			timestamp.sprintf("%s: %4u,%04u,%04u,%04u", name.cstr(), gb, mb, kb, b);
		}
		else if(mb)
		{
			timestamp.sprintf("%s: %4u,%04u,%04u", name.cstr(), mb, kb, b);
		}
		else if(kb)
		{
			timestamp.sprintf("%s: %4u,%04u", name.cstr(), kb, b);
		}
		else
		{
			timestamp.sprintf("%s: %4u", name.cstr(), b);
		}
		ImGui::TextUnformatted(timestamp.cstr());
	}

	void labelUint(U64 val, CString name)
	{
		ImGui::Text("%s: %lu", name.cstr(), val);
	}
};

void* App::MemStats::allocCallback(void* userData, void* ptr, PtrSize size, PtrSize alignment)
{
	ANKI_ASSERT(userData);

	static const PtrSize MAX_ALIGNMENT = 64;

	struct alignas(MAX_ALIGNMENT) Header
	{
		PtrSize m_allocatedSize;
		Array<U8, MAX_ALIGNMENT - sizeof(PtrSize)> _m_padding;
	};
	static_assert(sizeof(Header) == MAX_ALIGNMENT, "See file");
	static_assert(alignof(Header) == MAX_ALIGNMENT, "See file");

	void* out = nullptr;

	if(ptr == nullptr)
	{
		// Need to allocate
		ANKI_ASSERT(size > 0);
		ANKI_ASSERT(alignment > 0 && alignment <= MAX_ALIGNMENT);

		const PtrSize newAlignment = MAX_ALIGNMENT;
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
	cleanup();
}

void App::cleanup()
{
	m_heapAlloc.deleteInstance(m_statsUi);
	m_console.reset(nullptr);

	m_heapAlloc.deleteInstance(m_scene);
	m_scene = nullptr;
	m_heapAlloc.deleteInstance(m_script);
	m_script = nullptr;
	m_heapAlloc.deleteInstance(m_renderer);
	m_renderer = nullptr;
	m_heapAlloc.deleteInstance(m_ui);
	m_ui = nullptr;
	m_heapAlloc.deleteInstance(m_resources);
	m_resources = nullptr;
	m_heapAlloc.deleteInstance(m_resourceFs);
	m_resourceFs = nullptr;
	m_heapAlloc.deleteInstance(m_physics);
	m_physics = nullptr;
	m_heapAlloc.deleteInstance(m_stagingMem);
	m_stagingMem = nullptr;
	m_heapAlloc.deleteInstance(m_vertexMem);
	m_vertexMem = nullptr;
	m_heapAlloc.deleteInstance(m_threadHive);
	m_threadHive = nullptr;
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

	m_settingsDir.destroy(m_heapAlloc);
	m_cacheDir.destroy(m_heapAlloc);
}

Error App::init(const ConfigSet& config, AllocAlignedCallback allocCb, void* allocCbUserData)
{
	const Error err = initInternal(config, allocCb, allocCbUserData);
	if(err)
	{
		ANKI_CORE_LOGE("App initialization failed. Shutting down");
		cleanup();
	}

	return err;
}

Error App::initInternal(const ConfigSet& config_, AllocAlignedCallback allocCb, void* allocCbUserData)
{
	setSignalHandlers();

	Thread::setNameOfCurrentThread("anki_main");

	ConfigSet config = config_;
	m_displayStats = config.getNumberU32("core_displayStats");

	initMemoryCallbacks(allocCb, allocCbUserData);
	m_heapAlloc = HeapAllocator<U8>(m_allocCb, m_allocCbData);

	ANKI_CHECK(initDirs(config));

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

	m_timerTick = 1.0 / F32(config.getNumberU32("core_targetFps")); // in sec. 1.0 / period

// Check SIMD support
#if ANKI_SIMD_SSE && ANKI_COMPILER_GCC_COMPATIBLE
	if(!__builtin_cpu_supports("sse4.2"))
	{
		ANKI_CORE_LOGF(
			"AnKi is built with sse4.2 support but your CPU doesn't support it. Try bulding without SSE support");
	}
#endif

	ANKI_CORE_LOGI("Number of main threads: %u", config.getNumberU32("core_mainThreadCount"));

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
	nwinit.m_allocCallback = m_allocCb;
	nwinit.m_allocCallbackUserData = m_allocCbData;
	nwinit.m_width = config.getNumberU32("width");
	nwinit.m_height = config.getNumberU32("height");
	nwinit.m_depthBits = 0;
	nwinit.m_stencilBits = 0;
	nwinit.m_fullscreenDesktopRez = config.getBool("window_fullscreen");
	ANKI_CHECK(NativeWindow::newInstance(nwinit, m_window));

	//
	// Input
	//
	ANKI_CHECK(Input::newInstance(m_allocCb, m_allocCbData, m_window, m_input));

	//
	// ThreadPool
	//
	m_threadHive = m_heapAlloc.newInstance<ThreadHive>(config.getNumberU32("core_mainThreadCount"), m_heapAlloc, true);

	//
	// Graphics API
	//
	GrManagerInitInfo grInit;
	grInit.m_allocCallback = m_allocCb;
	grInit.m_allocCallbackUserData = m_allocCbData;
	grInit.m_cacheDirectory = m_cacheDir.toCString();
	grInit.m_config = &config;
	grInit.m_window = m_window;

	ANKI_CHECK(GrManager::newInstance(grInit, m_gr));

	//
	// GPU mem
	//
	m_vertexMem = m_heapAlloc.newInstance<VertexGpuMemoryPool>();
	ANKI_CHECK(m_vertexMem->init(m_heapAlloc, m_gr, config));

	m_stagingMem = m_heapAlloc.newInstance<StagingGpuMemoryPool>();
	ANKI_CHECK(m_stagingMem->init(m_gr, config));

	//
	// Physics
	//
	m_physics = m_heapAlloc.newInstance<PhysicsWorld>();

	ANKI_CHECK(m_physics->init(m_allocCb, m_allocCbData));

	//
	// Resource FS
	//
	m_resourceFs = m_heapAlloc.newInstance<ResourceFilesystem>(m_heapAlloc);
	ANKI_CHECK(m_resourceFs->init(config, m_cacheDir.toCString()));

	//
	// Resources
	//
	ResourceManagerInitInfo rinit;
	rinit.m_gr = m_gr;
	rinit.m_physics = m_physics;
	rinit.m_resourceFs = m_resourceFs;
	rinit.m_vertexMemory = m_vertexMem;
	rinit.m_config = &config;
	rinit.m_cacheDir = m_cacheDir.toCString();
	rinit.m_allocCallback = m_allocCb;
	rinit.m_allocCallbackData = m_allocCbData;
	m_resources = m_heapAlloc.newInstance<ResourceManager>();

	ANKI_CHECK(m_resources->init(rinit));

	//
	// UI
	//
	m_ui = m_heapAlloc.newInstance<UiManager>();
	ANKI_CHECK(m_ui->init(m_allocCb, m_allocCbData, m_resources, m_gr, m_stagingMem, m_input));

	//
	// Renderer
	//
	if(nwinit.m_fullscreenDesktopRez)
	{
		config.set("width", m_window->getWidth());
		config.set("height", m_window->getHeight());
	}

	m_renderer = m_heapAlloc.newInstance<MainRenderer>();

	ANKI_CHECK(m_renderer->init(m_threadHive, m_resources, m_gr, m_stagingMem, m_ui, m_allocCb, m_allocCbData, config,
								&m_globalTimestamp));

	//
	// Script
	//
	m_script = m_heapAlloc.newInstance<ScriptManager>();
	ANKI_CHECK(m_script->init(m_allocCb, m_allocCbData));

	//
	// Scene
	//
	m_scene = m_heapAlloc.newInstance<SceneGraph>();

	ANKI_CHECK(m_scene->init(m_allocCb, m_allocCbData, m_threadHive, m_resources, m_input, m_script, m_ui,
							 &m_globalTimestamp, config));

	// Inform the script engine about some subsystems
	m_script->setRenderer(m_renderer);
	m_script->setSceneGraph(m_scene);

	//
	// Misc
	//
	m_statsUi = m_heapAlloc.newInstance<StatsUi>(m_heapAlloc);
	ANKI_CHECK(m_ui->newInstance<DeveloperConsole>(m_console, m_allocCb, m_allocCbData, m_script));

	ANKI_CORE_LOGI("Application initialized");

	return Error::NONE;
}

Error App::initDirs(const ConfigSet& cfg)
{
	// Settings path
#if !ANKI_OS_ANDROID
	StringAuto home(m_heapAlloc);
	ANKI_CHECK(getHomeDirectory(home));

	m_settingsDir.sprintf(m_heapAlloc, "%s/.anki", &home[0]);
#else
	m_settingsDir.sprintf(m_heapAlloc, "%s/.anki", g_androidApp->activity->internalDataPath);
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
	m_cacheDir.sprintf(m_heapAlloc, "%s/cache", &m_settingsDir[0]);

	const Bool cacheDirExists = directoryExists(m_cacheDir.toCString());
	if(cfg.getBool("core_clearCaches") && cacheDirExists)
	{
		ANKI_CORE_LOGI("Will delete the cache dir and start fresh: %s", &m_cacheDir[0]);
		ANKI_CHECK(removeDirectory(m_cacheDir.toCString(), m_heapAlloc));
		ANKI_CHECK(createDirectory(m_cacheDir.toCString()));
	}
	else if(!cacheDirExists)
	{
		ANKI_CORE_LOGI("Will create cache dir: %s", &m_cacheDir[0]);
		ANKI_CHECK(createDirectory(m_cacheDir.toCString()));
	}

	return Error::NONE;
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
			DynamicArrayAuto<UiQueueElement> newUiElementArr(m_heapAlloc);
			injectUiElements(newUiElementArr, rqueue);

			// Render
			TexturePtr presentableTex = m_gr->acquireNextPresentableTexture();
			m_renderer->setStatsEnabled(m_displayStats
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
			if(frameTime < m_timerTick)
			{
				ANKI_TRACE_SCOPED_EVENT(TIMER_TICK_SLEEP);
				HighRezTimer::sleep(m_timerTick - frameTime);
			}

			// Stats
			if(m_displayStats)
			{
				m_statsUi->m_frameTime.set(frameTime);
				m_statsUi->m_renderTime.set(m_renderer->getStats().m_renderingCpuTime);
				m_statsUi->m_sceneUpdateTime.set(m_scene->getStats().m_updateTime);
				m_statsUi->m_visTestsTime.set(m_scene->getStats().m_visibilityTestsTime);
				m_statsUi->m_physicsTime.set(m_scene->getStats().m_physicsUpdate);
				m_statsUi->m_gpuTime.set(m_renderer->getStats().m_renderingGpuTime);
				m_statsUi->m_allocatedCpuMem = m_memStats.m_allocatedMem.load();
				m_statsUi->m_allocCount = m_memStats.m_allocCount.load();
				m_statsUi->m_freeCount = m_memStats.m_freeCount.load();

				GrManagerStats grStats = m_gr->getStats();
				m_statsUi->m_vkCpuMem = grStats.m_cpuMemory;
				m_statsUi->m_vkGpuMem = grStats.m_gpuMemory;
				m_statsUi->m_vkCmdbCount = grStats.m_commandBufferCount;

				m_statsUi->m_drawableCount = rqueue.countAllRenderables();
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

	return Error::NONE;
}

void App::injectUiElements(DynamicArrayAuto<UiQueueElement>& newUiElementArr, RenderQueue& rqueue)
{
	const U32 originalCount = rqueue.m_uis.getSize();
	if(m_displayStats || m_consoleEnabled)
	{
		const U32 extraElements = (m_displayStats != 0) + (m_consoleEnabled != 0);
		newUiElementArr.create(originalCount + extraElements);

		if(originalCount > 0)
		{
			memcpy(&newUiElementArr[0], &rqueue.m_uis[0], rqueue.m_uis.getSizeInBytes());
		}

		rqueue.m_uis = WeakArray<UiQueueElement>(newUiElementArr);
	}

	U32 count = originalCount;
	if(m_displayStats)
	{
		newUiElementArr[count].m_userData = m_statsUi;
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

void App::initMemoryCallbacks(AllocAlignedCallback allocCb, void* allocCbUserData)
{
	if(m_displayStats)
	{
		m_memStats.m_originalAllocCallback = allocCb;
		m_memStats.m_originalUserData = allocCbUserData;

		m_allocCb = MemStats::allocCallback;
		m_allocCbData = &m_memStats;
	}
	else
	{
		m_allocCb = allocCb;
		m_allocCbData = allocCbUserData;
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
		backtrace(HeapAllocator<U8>(allocAligned, nullptr), [&count](CString symbol) {
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
