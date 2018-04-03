// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/core/App.h>
#include <anki/misc/ConfigSet.h>
#include <anki/util/Logger.h>
#include <anki/util/File.h>
#include <anki/util/Filesystem.h>
#include <anki/util/System.h>
#include <anki/util/ThreadPool.h>
#include <anki/util/ThreadHive.h>
#include <anki/core/Trace.h>

#include <anki/core/NativeWindow.h>
#include <anki/input/Input.h>
#include <anki/scene/SceneGraph.h>
#include <anki/resource/ResourceManager.h>
#include <anki/physics/PhysicsWorld.h>
#include <anki/renderer/MainRenderer.h>
#include <anki/script/ScriptManager.h>
#include <anki/resource/ResourceFilesystem.h>
#include <anki/resource/AsyncLoader.h>
#include <anki/core/StagingGpuMemoryManager.h>
#include <anki/ui/UiManager.h>

#if ANKI_OS == ANKI_OS_ANDROID
#	include <android_native_app_glue.h>
#endif

namespace anki
{

#if ANKI_OS == ANKI_OS_ANDROID
/// The one and only android hack
android_app* gAndroidApp = nullptr;
#endif

class App::StatsUi : public UiImmediateModeBuilder
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

	BufferedValue<Second> m_frameTime;
	BufferedValue<Second> m_renderTime;
	BufferedValue<Second> m_lightBinTime;
	BufferedValue<Second> m_sceneUpdateTime;
	BufferedValue<Second> m_visTestsTime;

	PtrSize m_allocatedCpuMem = 0;
	U64 m_allocCount = 0;
	U64 m_freeCount = 0;

	U64 m_vkCpuMem = 0;
	U64 m_vkGpuMem = 0;

	static const U32 BUFFERED_FRAMES = 16;
	U32 m_bufferedFrames = 0;

	StatsUi(UiManager* ui)
		: UiImmediateModeBuilder(ui)
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
		nk_context* ctx = &canvas->getNkContext();

		canvas->pushFont(canvas->getDefaultFont(), 16);

		nk_style_push_style_item(ctx, &ctx->style.window.fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 128)));

		if(nk_begin(ctx, "Stats", nk_rect(5, 5, 230, 290), 0))
		{
			nk_layout_row_dynamic(ctx, 17, 1);

			nk_label(ctx, "Time:", NK_TEXT_ALIGN_LEFT);
			labelTime(ctx, m_frameTime.get(flush), "Total frame");
			labelTime(ctx, m_renderTime.get(flush) - m_lightBinTime.get(flush), "Renderer");
			labelTime(ctx, m_lightBinTime.get(false), "Light bin");
			labelTime(ctx, m_sceneUpdateTime.get(flush), "Scene update");
			labelTime(ctx, m_visTestsTime.get(flush), "Visibility");

			nk_label(ctx, " ", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "Memory:", NK_TEXT_ALIGN_LEFT);
			labelBytes(ctx, m_allocatedCpuMem, "Total CPU");
			labelUint(ctx, m_allocCount, "Total allocations");
			labelUint(ctx, m_freeCount, "Total frees");
			labelBytes(ctx, m_vkCpuMem, "Vulkan CPU");
			labelBytes(ctx, m_vkGpuMem, "Vulkan GPU");
		}

		nk_style_pop_style_item(ctx);
		nk_end(ctx);
		canvas->popFont();
	}

	void labelTime(nk_context* ctx, Second val, CString name)
	{
		StringAuto timestamp(getAllocator());
		timestamp.sprintf("%s: %fms", name.cstr(), val * 1000.0);
		nk_label(ctx, timestamp.cstr(), NK_TEXT_ALIGN_LEFT);
	}

	void labelBytes(nk_context* ctx, PtrSize val, CString name)
	{
		U gb, mb, kb, b;

		gb = val / 1_GB;
		val -= gb * 1_GB;

		mb = val / 1_MB;
		val -= mb * 1_MB;

		kb = val / 1_KB;
		val -= kb * 1_KB;

		b = val;

		StringAuto timestamp(getAllocator());
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
		nk_label(ctx, timestamp.cstr(), NK_TEXT_ALIGN_LEFT);
	}

	void labelUint(nk_context* ctx, U64 val, CString name)
	{
		StringAuto timestamp(getAllocator());
		timestamp.sprintf("%s: %llu", name.cstr(), val);
		nk_label(ctx, timestamp.cstr(), NK_TEXT_ALIGN_LEFT);
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
	if(m_script)
	{
		m_heapAlloc.deleteInstance(m_script);
		m_script = nullptr;
	}

	if(m_scene)
	{
		m_heapAlloc.deleteInstance(m_scene);
		m_scene = nullptr;
	}

	if(m_renderer)
	{
		m_heapAlloc.deleteInstance(m_renderer);
		m_renderer = nullptr;
	}

	if(m_ui)
	{
		m_statsUi.reset(nullptr);

		m_heapAlloc.deleteInstance(m_ui);
		m_ui = nullptr;
	}

	if(m_resources)
	{
		m_heapAlloc.deleteInstance(m_resources);
		m_resources = nullptr;
	}

	if(m_resourceFs)
	{
		m_heapAlloc.deleteInstance(m_resourceFs);
		m_resourceFs = nullptr;
	}

	if(m_physics)
	{
		m_heapAlloc.deleteInstance(m_physics);
		m_physics = nullptr;
	}

	if(m_stagingMem)
	{
		m_heapAlloc.deleteInstance(m_stagingMem);
		m_stagingMem = nullptr;
	}

	if(m_gr)
	{
		GrManager::deleteInstance(m_gr);
		m_gr = nullptr;
	}

	if(m_threadpool)
	{
		m_heapAlloc.deleteInstance(m_threadpool);
		m_threadpool = nullptr;
	}

	if(m_threadHive)
	{
		m_heapAlloc.deleteInstance(m_threadHive);
		m_threadHive = nullptr;
	}

	if(m_input)
	{
		m_heapAlloc.deleteInstance(m_input);
		m_input = nullptr;
	}

	if(m_window)
	{
		m_heapAlloc.deleteInstance(m_window);
		m_window = nullptr;
	}

	m_settingsDir.destroy(m_heapAlloc);
	m_cacheDir.destroy(m_heapAlloc);

#if ANKI_ENABLE_TRACE
	TraceManagerSingleton::destroy();
#endif
}

Error App::init(const ConfigSet& config, AllocAlignedCallback allocCb, void* allocCbUserData)
{
	Error err = initInternal(config, allocCb, allocCbUserData);
	if(err)
	{
		cleanup();
		ANKI_CORE_LOGE("App initialization failed");
	}

	return err;
}

Error App::initInternal(const ConfigSet& config_, AllocAlignedCallback allocCb, void* allocCbUserData)
{
	ConfigSet config = config_;
	m_displayStats = config.getNumber("core.displayStats");

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
		"extra checks";
#else
		"NO extra checks";
#endif

	ANKI_CORE_LOGI("Initializing application ("
				   "version %u.%u, "
				   "%s, "
				   "compiler %s, "
				   "build date %s, "
				   "commit %s)",
		ANKI_VERSION_MAJOR,
		ANKI_VERSION_MINOR,
		buildType,
		ANKI_COMPILER_STR,
		__DATE__,
		ANKI_REVISION);

	m_timerTick = 1.0 / 60.0; // in sec. 1.0 / period

// Check SIMD support
#if ANKI_SIMD == ANKI_SIMD_SSE && ANKI_COMPILER != ANKI_COMPILER_MSVC
	if(!__builtin_cpu_supports("sse4.2"))
	{
		ANKI_CORE_LOGF(
			"AnKi is built with sse4.2 support but your CPU doesn't support it. Try bulding without SSE support");
	}
#endif

#if ANKI_ENABLE_TRACE
	ANKI_CHECK(TraceManagerSingleton::get().create(m_heapAlloc, m_settingsDir.toCString()));
#endif

	ANKI_CORE_LOGI("Number of main threads: %u", U(config.getNumber("core.mainThreadCount")));

	//
	// Window
	//
	NativeWindowInitInfo nwinit;
	nwinit.m_width = config.getNumber("width");
	nwinit.m_height = config.getNumber("height");
	nwinit.m_depthBits = 0;
	nwinit.m_stencilBits = 0;
	nwinit.m_fullscreenDesktopRez = config.getNumber("window.fullscreenDesktopResolution");
	m_window = m_heapAlloc.newInstance<NativeWindow>();

	ANKI_CHECK(m_window->init(nwinit, m_heapAlloc));

	//
	// Input
	//
	m_input = m_heapAlloc.newInstance<Input>();
	ANKI_CHECK(m_input->init(m_window));

	//
	// ThreadPool
	//
	m_threadpool = m_heapAlloc.newInstance<ThreadPool>(config.getNumber("core.mainThreadCount"), true);
	m_threadHive = m_heapAlloc.newInstance<ThreadHive>(config.getNumber("core.mainThreadCount"), m_heapAlloc, true);

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
	// Staging mem
	//
	m_stagingMem = m_heapAlloc.newInstance<StagingGpuMemoryManager>();
	ANKI_CHECK(m_stagingMem->init(m_gr, config));

	//
	// Physics
	//
	m_physics = m_heapAlloc.newInstance<PhysicsWorld>();

	ANKI_CHECK(m_physics->create(m_allocCb, m_allocCbData));

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

	ANKI_CHECK(m_ui->newInstance<StatsUi>(m_statsUi));

	//
	// Renderer
	//
	if(nwinit.m_fullscreenDesktopRez)
	{
		config.set("width", m_window->getWidth());
		config.set("height", m_window->getHeight());
	}

	m_renderer = m_heapAlloc.newInstance<MainRenderer>();

	ANKI_CHECK(m_renderer->init(
		m_threadpool, m_resources, m_gr, m_stagingMem, m_ui, m_allocCb, m_allocCbData, config, &m_globalTimestamp));

	//
	// Script
	//
	m_script = m_heapAlloc.newInstance<ScriptManager>();
	ANKI_CHECK(m_script->init(m_allocCb, m_allocCbData));

	//
	// Scene
	//
	m_scene = m_heapAlloc.newInstance<SceneGraph>();

	ANKI_CHECK(m_scene->init(m_allocCb,
		m_allocCbData,
		m_threadpool,
		m_threadHive,
		m_resources,
		m_input,
		m_script,
		&m_globalTimestamp,
		config));

	// Inform the script engine about some subsystems
	m_script->setRenderer(m_renderer);
	m_script->setSceneGraph(m_scene);

	ANKI_CORE_LOGI("Application initialized");
	return Error::NONE;
}

Error App::initDirs(const ConfigSet& cfg)
{
#if ANKI_OS != ANKI_OS_ANDROID
	// Settings path
	StringAuto home(m_heapAlloc);
	ANKI_CHECK(getHomeDirectory(m_heapAlloc, home));

	m_settingsDir.sprintf(m_heapAlloc, "%s/.anki", &home[0]);

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
	if(cfg.getNumber("clearCaches") && cacheDirExists)
	{
		ANKI_CORE_LOGI("Will delete the cache dir and start fresh: %s", &m_cacheDir[0]);
		ANKI_CHECK(removeDirectory(m_cacheDir.toCString()));
		ANKI_CHECK(createDirectory(m_cacheDir.toCString()));
	}
	else if(!cacheDirExists)
	{
		ANKI_CORE_LOGI("Will create cache dir: %s", &m_cacheDir[0]);
		ANKI_CHECK(createDirectory(m_cacheDir.toCString()));
	}

#else
	// ANKI_ASSERT(gAndroidApp);
	// ANativeActivity* activity = gAndroidApp->activity;

	// Settings path
	// settingsDir = String(activity->internalDataDir, alloc);
	settingsDir = String("/sdcard/.anki/");
	if(!directoryExists(settingsDir.c_str()))
	{
		createDirectory(settingsDir.c_str());
	}

	// Cache
	cacheDir = settingsDir + "/cache";
	if(directoryExists(cacheDir.c_str()))
	{
		removeDirectory(cacheDir.c_str());
	}

	createDirectory(cacheDir.c_str());
#endif

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
		ANKI_TRACE_START_FRAME();
		const Second startTime = HighRezTimer::getCurrentTime();

		prevUpdateTime = crntTime;
		crntTime = HighRezTimer::getCurrentTime();

		m_gr->beginFrame();

		// Update
		ANKI_CHECK(m_input->handleEvents());

		// User update
		ANKI_CHECK(userMainLoop(quit));

		ANKI_CHECK(m_scene->update(prevUpdateTime, crntTime));

		RenderQueue rqueue;
		m_scene->doVisibilityTests(rqueue);

		// Inject stats UI
		DynamicArrayAuto<UiQueueElement> newUiElementArr(m_heapAlloc);
		injectStatsUiElement(newUiElementArr, rqueue);

		// Render
		ANKI_CHECK(m_renderer->render(rqueue));

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
			StatsUi& statsUi = static_cast<StatsUi&>(*m_statsUi);
			statsUi.m_frameTime.set(frameTime);
			statsUi.m_renderTime.set(m_renderer->getStats().m_renderingTime);
			statsUi.m_lightBinTime.set(m_renderer->getStats().m_lightBinTime);
			statsUi.m_sceneUpdateTime.set(m_scene->getStats().m_updateTime);
			statsUi.m_visTestsTime.set(m_scene->getStats().m_visibilityTestsTime);
			statsUi.m_allocatedCpuMem = m_memStats.m_allocatedMem.load();
			statsUi.m_allocCount = m_memStats.m_allocCount.load();
			statsUi.m_freeCount = m_memStats.m_freeCount.load();

			GrManagerStats grStats = m_gr->getStats();
			statsUi.m_vkCpuMem = grStats.m_cpuMemory;
			statsUi.m_vkGpuMem = grStats.m_gpuMemory;
		}

		++m_globalTimestamp;

		ANKI_TRACE_STOP_FRAME();
	}

	return Error::NONE;
}

void App::injectStatsUiElement(DynamicArrayAuto<UiQueueElement>& newUiElementArr, RenderQueue& rqueue)
{
	if(m_displayStats)
	{
		U count = rqueue.m_uis.getSize();
		newUiElementArr.create(count + 1u);

		if(count)
		{
			memcpy(&newUiElementArr[0], &rqueue.m_uis[0], rqueue.m_uis.getSizeInBytes());
		}

		newUiElementArr[count].m_userData = m_statsUi.get();
		newUiElementArr[count].m_drawCallback = [](CanvasPtr& canvas, void* userData) -> void {
			static_cast<StatsUi*>(userData)->build(canvas);
		};

		rqueue.m_uis = newUiElementArr;
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

} // end namespace anki
