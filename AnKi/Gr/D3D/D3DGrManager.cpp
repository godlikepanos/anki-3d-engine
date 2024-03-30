// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DGrManager.h>
#include <AnKi/Gr/D3D/D3DAccelerationStructure.h>
#include <AnKi/Gr/D3D/D3DTexture.h>
#include <AnKi/Gr/D3D/D3DCommandBuffer.h>
#include <AnKi/Gr/D3D/D3DShader.h>
#include <AnKi/Gr/D3D/D3DShaderProgram.h>
#include <AnKi/Gr/D3D/D3DTexture.h>
#include <AnKi/Gr/D3D/D3DTextureView.h>
#include <AnKi/Gr/D3D/D3DPipelineQuery.h>
#include <AnKi/Gr/D3D/D3DSampler.h>
#include <AnKi/Gr/D3D/D3DFramebuffer.h>
#include <AnKi/Gr/RenderGraph.h>
#include <AnKi/Gr/D3D/D3DGrUpscaler.h>
#include <AnKi/Gr/D3D/D3DTimestampQuery.h>

#include <AnKi/ShaderCompiler/Common.h>

#if ANKI_WINDOWING_SYSTEM_SDL
#	include <AnKi/Window/NativeWindowSdl.h>
#	include <SDL_syswm.h>
#endif

namespace anki {

BoolCVar g_validationCVar(CVarSubsystem::kGr, "Validation", false, "Enable or not validation");
static BoolCVar g_gpuValidationCVar(CVarSubsystem::kGr, "GpuValidation", false, "Enable or not GPU validation");
BoolCVar g_vsyncCVar(CVarSubsystem::kGr, "Vsync", false, "Enable or not vsync");
BoolCVar g_debugMarkersCVar(CVarSubsystem::kGr, "DebugMarkers", false, "Enable or not debug markers");
BoolCVar g_meshShadersCVar(CVarSubsystem::kGr, "MeshShaders", false, "Enable or not mesh shaders");
static NumericCVar<U8> g_deviceCVar(CVarSubsystem::kGr, "Device", 0, 0, 16, "Choose an available device. Devices are sorted by performance");

template<>
template<>
GrManager& MakeSingletonPtr<GrManager>::allocateSingleton<>()
{
	ANKI_ASSERT(m_global == nullptr);
	m_global = new GrManagerImpl;

#if ANKI_ASSERTIONS_ENABLED
	++g_singletonsAllocated;
#endif

	return *m_global;
}

template<>
void MakeSingletonPtr<GrManager>::freeSingleton()
{
	if(m_global)
	{
		delete static_cast<GrManagerImpl*>(m_global);
		m_global = nullptr;
#if ANKI_ASSERTIONS_ENABLED
		--g_singletonsAllocated;
#endif
	}
}

GrManager::GrManager()
{
}

GrManager::~GrManager()
{
}

Error GrManager::init(GrManagerInitInfo& inf)
{
	ANKI_D3D_SELF(GrManagerImpl);
	return self.initInternal(inf);
}

TexturePtr GrManager::acquireNextPresentableTexture()
{
	ANKI_ASSERT(!"TODO");
	return TexturePtr(nullptr);
}

void GrManager::swapBuffers()
{
	ANKI_ASSERT(!"TODO");
}

void GrManager::finish()
{
	ANKI_ASSERT(!"TODO");
}

#define ANKI_NEW_GR_OBJECT(type) \
	type##Ptr GrManager::new##type(const type##InitInfo& init) \
	{ \
		type##Ptr ptr(nullptr); \
		if(!ptr.isCreated()) [[unlikely]] \
		{ \
			ANKI_D3D_LOGF("Failed to create a " ANKI_STRINGIZE(type) " object"); \
		} \
		return ptr; \
	}

#define ANKI_NEW_GR_OBJECT_NO_INIT_INFO(type) \
	type##Ptr GrManager::new##type() \
	{ \
		type##Ptr ptr(nullptr); \
		if(!ptr.isCreated()) [[unlikely]] \
		{ \
			ANKI_D3D_LOGF("Failed to create a " ANKI_STRINGIZE(type) " object"); \
		} \
		return ptr; \
	}

ANKI_NEW_GR_OBJECT(Buffer)
ANKI_NEW_GR_OBJECT(Texture)
ANKI_NEW_GR_OBJECT(TextureView)
ANKI_NEW_GR_OBJECT(Sampler)
ANKI_NEW_GR_OBJECT(Shader)
ANKI_NEW_GR_OBJECT(ShaderProgram)
ANKI_NEW_GR_OBJECT(CommandBuffer)
ANKI_NEW_GR_OBJECT(Framebuffer)
// ANKI_NEW_GR_OBJECT_NO_INIT_INFO(OcclusionQuery)
ANKI_NEW_GR_OBJECT_NO_INIT_INFO(TimestampQuery)
ANKI_NEW_GR_OBJECT(PipelineQuery)
ANKI_NEW_GR_OBJECT_NO_INIT_INFO(RenderGraph)
ANKI_NEW_GR_OBJECT(AccelerationStructure)
ANKI_NEW_GR_OBJECT(GrUpscaler)

#undef ANKI_NEW_GR_OBJECT
#undef ANKI_NEW_GR_OBJECT_NO_INIT_INFO

GrManagerImpl::~GrManagerImpl()
{
	destroy();
}

void GrManager::submit(WeakArray<CommandBuffer*> cmdbs, WeakArray<Fence*> waitFences, FencePtr* signalFence)
{
	ANKI_ASSERT(!"TODO");
}

Error GrManagerImpl::initInternal(const GrManagerInitInfo& init)
{
	ANKI_D3D_LOGI("Initializing D3D backend");

	GrMemoryPool::allocateSingleton(init.m_allocCallback, init.m_allocCallbackUserData);

	// Validation
	UINT dxgiFactoryFlags = 0;
	if(g_validationCVar.get())
	{
		ComPtr<ID3D12Debug> debugInterface;
		ANKI_D3D_CHECK(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));

		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

		if(g_gpuValidationCVar.get())
		{
			ComPtr<ID3D12Debug1> debugInterface1;
			ANKI_D3D_CHECK(debugInterface->QueryInterface(IID_PPV_ARGS(&debugInterface1)));

			debugInterface1->SetEnableGPUBasedValidation(true);
		}
	}

	ComPtr<IDXGIFactory4> factory2;
	ANKI_D3D_CHECK(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory2)));
	ComPtr<IDXGIFactory6> factory6;
	ANKI_D3D_CHECK(factory2->QueryInterface(IID_PPV_ARGS(&factory6)));

	// Get adapters
	struct Adapter
	{
		ComPtr<IDXGIAdapter1> m_adapter;
		DXGI_ADAPTER_DESC1 m_descr;
	};

	GrDynamicArray<Adapter> adapters;
	ComPtr<IDXGIAdapter1> pAdapter;
	UINT adapterIdx = 0;
	while(factory6->EnumAdapterByGpuPreference(adapterIdx, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&pAdapter)) != DXGI_ERROR_NOT_FOUND)
	{
		Adapter& a = *adapters.emplaceBack();
		a.m_adapter = pAdapter;
		pAdapter->GetDesc1(&a.m_descr);

		++adapterIdx;
	}

	const U32 chosenPhysDevIdx = min<U32>(g_deviceCVar.get(), adapters.getSize() - 1);

	ANKI_D3D_LOGI("Physical devices:");
	for(U32 i = 0; i < adapters.getSize(); ++i)
	{
		ANKI_D3D_LOGI((i == chosenPhysDevIdx) ? "\t(Selected) %s" : "\t%s", ws2s(&adapters[i].m_descr.Description[0]).c_str());
	}

	// Find vendor
	switch(adapters[chosenPhysDevIdx].m_descr.VendorId)
	{
	case 0x13B5:
		m_capabilities.m_gpuVendor = GpuVendor::kArm;
		m_capabilities.m_minSubgroupSize = 16;
		m_capabilities.m_maxSubgroupSize = 16;
		break;
	case 0x10DE:
		m_capabilities.m_gpuVendor = GpuVendor::kNvidia;
		m_capabilities.m_minSubgroupSize = 32;
		m_capabilities.m_maxSubgroupSize = 32;
		break;
	case 0x1002:
	case 0x1022:
		m_capabilities.m_gpuVendor = GpuVendor::kAMD;
		m_capabilities.m_minSubgroupSize = 32;
		m_capabilities.m_maxSubgroupSize = 64;
		break;
	case 0x8086:
		m_capabilities.m_gpuVendor = GpuVendor::kIntel;
		m_capabilities.m_minSubgroupSize = 8;
		m_capabilities.m_maxSubgroupSize = 32;
		break;
	case 0x5143:
		m_capabilities.m_gpuVendor = GpuVendor::kQualcomm;
		m_capabilities.m_minSubgroupSize = 64;
		m_capabilities.m_maxSubgroupSize = 128;
		break;
	default:
		m_capabilities.m_gpuVendor = GpuVendor::kUnknown;
		// Choose something really low
		m_capabilities.m_minSubgroupSize = 8;
		m_capabilities.m_maxSubgroupSize = 8;
	}
	ANKI_D3D_LOGI("Vendor identified as %s", &kGPUVendorStrings[m_capabilities.m_gpuVendor][0]);

	// Create device
	ANKI_D3D_CHECK(D3D12CreateDevice(adapters[chosenPhysDevIdx].m_adapter.Get(), D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&m_device)));

	// Create queues
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		ANKI_D3D_CHECK(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_queues[GpuQueueType::kGeneral])));
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		ANKI_D3D_CHECK(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_queues[GpuQueueType::kCompute])));
	}

	// Create swapchain
	{
		const NativeWindowSdl& window = static_cast<NativeWindowSdl&>(NativeWindow::getSingleton());

		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		SDL_GetWindowWMInfo(window.m_sdlWindow, &wmInfo);
		const HWND hwnd = wmInfo.info.win.window;

		// Describe and create the swap chain.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = kMaxFramesInFlight;
		swapChainDesc.Width = window.getWidth();
		swapChainDesc.Height = window.getHeight();
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		ComPtr<IDXGISwapChain1> swapChain;
		// Swap chain needs the queue so that it can force a flush on it.
		ANKI_D3D_CHECK(factory2->CreateSwapChainForHwnd(m_queues[GpuQueueType::kGeneral], hwnd, &swapChainDesc, nullptr, nullptr, &swapChain));

		swapChain->QueryInterface(IID_PPV_ARGS(&m_swapchain));

		// Does not support fullscreen transitions.
		factory2->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

		m_backbufferIdx = m_swapchain->GetCurrentBackBufferIndex();
	}

	return Error::kNone;
}

void GrManagerImpl::destroy()
{
	ANKI_D3D_LOGI("Destroying D3D backend");

	safeRelease(m_swapchain);
	safeRelease(m_queues[GpuQueueType::kGeneral]);
	safeRelease(m_queues[GpuQueueType::kCompute]);
	safeRelease(m_device);

	GrMemoryPool::freeSingleton();
}

} // end namespace anki
