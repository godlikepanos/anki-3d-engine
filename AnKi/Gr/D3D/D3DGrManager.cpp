// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DGrManager.h>
#include <AnKi/Gr/D3D/D3DDescriptor.h>
#include <AnKi/Gr/D3D/D3DFrameGarbageCollector.h>

#include <AnKi/Gr/D3D/D3DAccelerationStructure.h>
#include <AnKi/Gr/D3D/D3DTexture.h>
#include <AnKi/Gr/D3D/D3DCommandBuffer.h>
#include <AnKi/Gr/D3D/D3DShader.h>
#include <AnKi/Gr/D3D/D3DShaderProgram.h>
#include <AnKi/Gr/D3D/D3DTexture.h>
#include <AnKi/Gr/D3D/D3DPipelineQuery.h>
#include <AnKi/Gr/D3D/D3DSampler.h>
#include <AnKi/Gr/RenderGraph.h>
#include <AnKi/Gr/D3D/D3DGrUpscaler.h>
#include <AnKi/Gr/D3D/D3DTimestampQuery.h>

#include <AnKi/ShaderCompiler/Common.h>
#include <AnKi/Util/Tracer.h>

#if ANKI_WINDOWING_SYSTEM_SDL
#	include <AnKi/Window/NativeWindowSdl.h>
#	include <SDL_syswm.h>
#endif

// Use the Agility SDK
extern "C" {
__declspec(dllexport) extern const UINT D3D12SDKVersion = 613; // Number taken from the download page
__declspec(dllexport) extern const char* D3D12SDKPath = ".\\"; // The D3D12Core.dll should be in the same dir as the .exe
}

namespace anki {

BoolCVar g_validationCVar(CVarSubsystem::kGr, "Validation", false, "Enable or not validation");
static BoolCVar g_gpuValidationCVar(CVarSubsystem::kGr, "GpuValidation", false, "Enable or not GPU validation");
BoolCVar g_vsyncCVar(CVarSubsystem::kGr, "Vsync", false, "Enable or not vsync");
BoolCVar g_debugMarkersCVar(CVarSubsystem::kGr, "DebugMarkers", false, "Enable or not debug markers");
BoolCVar g_meshShadersCVar(CVarSubsystem::kGr, "MeshShaders", false, "Enable or not mesh shaders");
static NumericCVar<U8> g_deviceCVar(CVarSubsystem::kGr, "Device", 0, 0, 16, "Choose an available device. Devices are sorted by performance");

static LONG NTAPI vexHandler(PEXCEPTION_POINTERS exceptionInfo)
{
	PEXCEPTION_RECORD exceptionRecord = exceptionInfo->ExceptionRecord;

	switch(exceptionRecord->ExceptionCode)
	{
	case DBG_PRINTEXCEPTION_WIDE_C:
	case DBG_PRINTEXCEPTION_C:

		if(exceptionRecord->NumberParameters >= 2)
		{
			ULONG len = exceptionRecord->ExceptionInformation[0];

			union
			{
				ULONG_PTR up;
				PCWSTR pwz;
				PCSTR psz;
			};

			up = exceptionRecord->ExceptionInformation[1];

			if(exceptionRecord->ExceptionCode == DBG_PRINTEXCEPTION_C)
			{
				const ULONG n = MultiByteToWideChar(CP_ACP, 0, psz, len, 0, 0);
				if(n)
				{
					WCHAR* wz = static_cast<WCHAR*>(_malloca(n * sizeof(WCHAR)));

					len = MultiByteToWideChar(CP_ACP, 0, psz, len, wz, n);
					if(len)
					{
						pwz = wz;
					}
				}
			}

			if(len)
			{
				const std::wstring wstring(pwz, len - 1);
				std::string str = ws2s(wstring);
				str.erase(std::remove(str.begin(), str.end(), '\n'), str.cend());
				str.erase(std::remove(str.begin(), str.end(), '\r'), str.cend());

				if(str.find("D3D12 INFO") == std::string::npos)
				{
					if(GrMemoryPool::isAllocated())
					{
						ANKI_D3D_LOGE("%s", str.c_str());
					}
					else
					{
						printf("D3D12 validation error: %s", str.c_str());
					}
				}
			}
		}
		return EXCEPTION_CONTINUE_EXECUTION;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

static void NTAPI d3dDebugMessageCallback([[maybe_unused]] D3D12_MESSAGE_CATEGORY category, D3D12_MESSAGE_SEVERITY severity,
										  [[maybe_unused]] D3D12_MESSAGE_ID id, LPCSTR pDescription, [[maybe_unused]] void* pContext)
{
	if(!Logger::isAllocated())
	{
		printf("d3dDebugMessageCallback : %s", pDescription);
		return;
	}
	else
	{
		switch(severity)
		{
		case D3D12_MESSAGE_SEVERITY_CORRUPTION:
		case D3D12_MESSAGE_SEVERITY_ERROR:
			ANKI_D3D_LOGE("%s", pDescription);
			break;
		case D3D12_MESSAGE_SEVERITY_WARNING:
			ANKI_D3D_LOGW("%s", pDescription);
			break;
		case D3D12_MESSAGE_SEVERITY_INFO:
		case D3D12_MESSAGE_SEVERITY_MESSAGE:
			ANKI_D3D_LOGI("%s", pDescription);
			break;
		}
	}
}

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
	ANKI_D3D_SELF(GrManagerImpl);

	self.m_crntSwapchain->m_backbufferIdx = self.m_crntSwapchain->m_swapchain->GetCurrentBackBufferIndex();
	return self.m_crntSwapchain->m_textures[self.m_crntSwapchain->m_backbufferIdx];
}

void GrManager::swapBuffers()
{
	ANKI_TRACE_SCOPED_EVENT(D3DSwapBuffers);
	ANKI_D3D_SELF(GrManagerImpl);

	self.m_crntSwapchain->m_swapchain->Present((g_vsyncCVar.get()) ? 1 : 0, (g_vsyncCVar.get()) ? 0 : DXGI_PRESENT_ALLOW_TEARING);

	MicroFencePtr presentFence = FenceFactory::getSingleton().newInstance();
	presentFence->gpuSignal(GpuQueueType::kGeneral);

	auto& crntFrame = self.m_frames[self.m_crntFrame];

	if(crntFrame.m_presentFence) [[likely]]
	{
		// Wait prev fence
		const Bool signaled = crntFrame.m_presentFence->clientWait(kMaxSecond);
		if(!signaled)
		{
			ANKI_D3D_LOGF("Frame fence didn't signal");
		}
	}

	crntFrame.m_presentFence = presentFence;

	self.m_crntFrame = (self.m_crntFrame + 1) % self.m_frames.getSize();

	FrameGarbageCollector::getSingleton().endFrame(presentFence.get());
}

void GrManager::finish()
{
	ANKI_ASSERT(!"TODO");
}

#define ANKI_NEW_GR_OBJECT(type) \
	type##Ptr GrManager::new##type(const type##InitInfo& init) \
	{ \
		type##Ptr ptr(type::newInstance(init)); \
		if(!ptr.isCreated()) [[unlikely]] \
		{ \
			ANKI_D3D_LOGF("Failed to create a " ANKI_STRINGIZE(type) " object"); \
		} \
		return ptr; \
	}

#define ANKI_NEW_GR_OBJECT_NO_INIT_INFO(type) \
	type##Ptr GrManager::new##type() \
	{ \
		type##Ptr ptr(type::newInstance()); \
		if(!ptr.isCreated()) [[unlikely]] \
		{ \
			ANKI_D3D_LOGF("Failed to create a " ANKI_STRINGIZE(type) " object"); \
		} \
		return ptr; \
	}

ANKI_NEW_GR_OBJECT(Buffer)
ANKI_NEW_GR_OBJECT(Texture)
ANKI_NEW_GR_OBJECT(Sampler)
ANKI_NEW_GR_OBJECT(Shader)
ANKI_NEW_GR_OBJECT(ShaderProgram)
ANKI_NEW_GR_OBJECT(CommandBuffer)
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
	ANKI_D3D_SELF(GrManagerImpl);

	// First thing, create a fence
	MicroFencePtr fence = FenceFactory::getSingleton().newInstance();

	// Gather command lists
	GrDynamicArray<ID3D12CommandList*> d3dCmdLists;
	d3dCmdLists.resizeStorage(cmdbs.getSize());
	GpuQueueType queueType = GpuQueueType::kCount;
	for(CommandBuffer* cmdb : cmdbs)
	{
		CommandBufferImpl& impl = static_cast<CommandBufferImpl&>(*cmdb);
		MicroCommandBuffer& mcmdb = impl.getMicroCommandBuffer();

		d3dCmdLists.emplaceBack(&mcmdb.getCmdList());
		if(queueType == GpuQueueType::kCount)
		{
			queueType = mcmdb.getQueueType();
		}
		else
		{
			ANKI_ASSERT(queueType == mcmdb.getQueueType());
		}

		mcmdb.setFence(fence.get());
	}

	// Wait for fences
	for(Fence* fence : waitFences)
	{
		FenceImpl& impl = static_cast<FenceImpl&>(*fence);
		impl.m_fence->gpuWait(queueType);
	}

	// Submit command lists
	self.m_queues[queueType]->ExecuteCommandLists(d3dCmdLists.getSize(), d3dCmdLists.getBegin());

	// Signal fence
	fence->gpuSignal(queueType);

	if(signalFence)
	{
		FenceImpl* fenceImpl = anki::newInstance<FenceImpl>(GrMemoryPool::getSingleton(), "SignalFence");
		fenceImpl->m_fence = fence;
		signalFence->reset(fenceImpl);
	}
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

		debugInterface->EnableDebugLayer();

		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

		if(g_gpuValidationCVar.get())
		{
			ComPtr<ID3D12Debug1> debugInterface1;
			ANKI_D3D_CHECK(debugInterface->QueryInterface(IID_PPV_ARGS(&debugInterface1)));

			debugInterface1->SetEnableGPUBasedValidation(true);
		}

		AddVectoredExceptionHandler(true, vexHandler);
	}

	ComPtr<IDXGIFactory2> factory2;
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
	ANKI_D3D_CHECK(D3D12CreateDevice(adapters[chosenPhysDevIdx].m_adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device)));

	if(g_validationCVar.get())
	{
		ComPtr<ID3D12InfoQueue1> infoq;
		const HRESULT res = m_device->QueryInterface(IID_PPV_ARGS(&infoq));

		if(res == S_OK)
		{
			ANKI_D3D_CHECK(
				infoq->RegisterMessageCallback(d3dDebugMessageCallback, D3D12_MESSAGE_CALLBACK_FLAG_NONE, nullptr, &m_debugMessageCallbackCookie));
		}
		else
		{
			ANKI_D3D_LOGW("ID3D12InfoQueue1 not supported");
		}
	}

	// Capabilities
	{
		D3D12_FEATURE_DATA_D3D12_OPTIONS16 options16;
		ANKI_D3D_CHECK(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS16, &options16, sizeof(options16)));
		m_d3dCapabilities.m_rebar = options16.GPUUploadHeapSupported;
		if(!m_d3dCapabilities.m_rebar)
		{
			ANKI_D3D_LOGW("ReBAR not supported");
		}
	}

	// Create queues
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		ANKI_D3D_CHECK(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_queues[GpuQueueType::kGeneral])));
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		ANKI_D3D_CHECK(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_queues[GpuQueueType::kCompute])));
	}

	// Other systems
	DescriptorFactory::allocateSingleton();
	ANKI_CHECK(DescriptorFactory::getSingleton().init());

	SwapchainFactory::allocateSingleton();
	m_crntSwapchain = SwapchainFactory::getSingleton().newInstance();

	RootSignatureFactory::allocateSingleton();
	FenceFactory::allocateSingleton();
	CommandBufferFactory::allocateSingleton();
	FrameGarbageCollector::allocateSingleton();

	return Error::kNone;
}

void GrManagerImpl::destroy()
{
	ANKI_D3D_LOGI("Destroying D3D backend");

	waitAllQueues();

	// Cleanup self
	m_crntSwapchain.reset(nullptr);
	m_frames = {};

	// Destroy systems
	CommandBufferFactory::freeSingleton();
	SwapchainFactory::freeSingleton();
	FrameGarbageCollector::freeSingleton();
	RootSignatureFactory::freeSingleton();
	DescriptorFactory::freeSingleton();
	FenceFactory::freeSingleton();

	safeRelease(m_queues[GpuQueueType::kGeneral]);
	safeRelease(m_queues[GpuQueueType::kCompute]);
	safeRelease(m_device);

	// Report objects that didn't cleaned up
	{
		ComPtr<IDXGIDebug1> dxgiDebug;
		if(SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
		{
			dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		}
	}

	GrMemoryPool::freeSingleton();
}

void GrManagerImpl::waitAllQueues()
{
	for(GpuQueueType queueType : EnumIterable<GpuQueueType>())
	{
		MicroFencePtr fence = FenceFactory::getSingleton().newInstance();
		fence->gpuSignal(queueType);
		fence->clientWait(kMaxSecond);
	}
}

} // end namespace anki
