// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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
__declspec(dllexport) extern const UINT D3D12SDKVersion = 614; // Number taken from the download page
__declspec(dllexport) extern const char* D3D12SDKPath = ".\\"; // The D3D12Core.dll should be in the same dir as the .exe
}

namespace anki {

static void NTAPI d3dDebugMessageCallback([[maybe_unused]] D3D12_MESSAGE_CATEGORY category, D3D12_MESSAGE_SEVERITY severity,
										  [[maybe_unused]] D3D12_MESSAGE_ID id, LPCSTR pDescription, [[maybe_unused]] void* pContext)
{
	if(id == D3D12_MESSAGE_ID_INVALID_BARRIER_ACCESS)
	{
		// Skip these for now
		return;
	}

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

	self.m_crntSwapchain->m_swapchain->Present((g_vsyncCVar) ? 1 : 0, (g_vsyncCVar) ? 0 : DXGI_PRESENT_ALLOW_TEARING);

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
	DescriptorFactory::getSingleton().endFrame();
}

void GrManager::finish()
{
	if(FenceFactory::isAllocated())
	{
		for(GpuQueueType qType : EnumIterable<GpuQueueType>())
		{
			MicroFencePtr fence = FenceFactory::getSingleton().newInstance();
			fence->gpuSignal(qType);
			fence->clientWait(kMaxSecond);
		}
	}
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
		impl.postSubmitWork(fence.get());
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
	if(g_validationCVar || g_gpuValidationCVar)
	{
		ComPtr<ID3D12Debug> debugInterface;
		ANKI_D3D_CHECK(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));

		debugInterface->EnableDebugLayer();

		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

		if(g_gpuValidationCVar)
		{
			ComPtr<ID3D12Debug1> debugInterface1;
			ANKI_D3D_CHECK(debugInterface->QueryInterface(IID_PPV_ARGS(&debugInterface1)));

			debugInterface1->SetEnableGPUBasedValidation(true);
		}

		ANKI_D3D_LOGI("Validation is enabled (GPU validation %s)", (g_gpuValidationCVar) ? "as well" : "no");
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

	const U32 chosenPhysDevIdx = min<U32>(g_deviceCVar, adapters.getSize() - 1);

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
		m_capabilities.m_minWaveSize = 16;
		m_capabilities.m_maxWaveSize = 16;
		break;
	case 0x10DE:
		m_capabilities.m_gpuVendor = GpuVendor::kNvidia;
		m_capabilities.m_minWaveSize = 32;
		m_capabilities.m_maxWaveSize = 32;
		break;
	case 0x1002:
	case 0x1022:
		m_capabilities.m_gpuVendor = GpuVendor::kAMD;
		m_capabilities.m_minWaveSize = 32;
		m_capabilities.m_maxWaveSize = 64;
		break;
	case 0x8086:
		m_capabilities.m_gpuVendor = GpuVendor::kIntel;
		m_capabilities.m_minWaveSize = 8;
		m_capabilities.m_maxWaveSize = 32;
		break;
	case 0x5143:
		m_capabilities.m_gpuVendor = GpuVendor::kQualcomm;
		m_capabilities.m_minWaveSize = 64;
		m_capabilities.m_maxWaveSize = 128;
		break;
	default:
		m_capabilities.m_gpuVendor = GpuVendor::kUnknown;
		// Choose something really low
		m_capabilities.m_minWaveSize = 8;
		m_capabilities.m_maxWaveSize = 8;
	}
	ANKI_D3D_LOGI("Vendor identified as %s", &kGPUVendorStrings[m_capabilities.m_gpuVendor][0]);

	// Create device
	ComPtr<ID3D12Device> dev;
	ANKI_D3D_CHECK(D3D12CreateDevice(adapters[chosenPhysDevIdx].m_adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&dev)));
	ANKI_D3D_CHECK(dev->QueryInterface(IID_PPV_ARGS(&m_device)));

	if(g_validationCVar)
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

			// At least break when debugging
			if(IsDebuggerPresent())
			{
				ComPtr<ID3D12InfoQueue> infoq;
				const HRESULT res = m_device->QueryInterface(IID_PPV_ARGS(&infoq));

				if(res == S_OK)
				{
					infoq->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
					// infoq->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
				}
			}
		}
	}

	if(g_dredCVar)
	{
		ComPtr<ID3D12DeviceRemovedExtendedDataSettings> dredSettings;
		ANKI_D3D_CHECK(D3D12GetDebugInterface(IID_PPV_ARGS(&dredSettings)));

		// Turn on AutoBreadcrumbs and Page Fault reporting
		dredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
		dredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);

		ANKI_D3D_LOGI("DRED is enabled");
		m_canInvokeDred = true;
	}

	// Create queues
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		ANKI_D3D_CHECK(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_queues[GpuQueueType::kGeneral])));
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		ANKI_D3D_CHECK(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_queues[GpuQueueType::kCompute])));

		ANKI_D3D_CHECK(m_queues[GpuQueueType::kGeneral]->GetTimestampFrequency(&m_timestampFrequency));
	}

	// Set device capabilities (taken from mesa's dozen driver)
	{
		D3D12_FEATURE_DATA_D3D12_OPTIONS16 options16;
		ANKI_D3D_CHECK(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS16, &options16, sizeof(options16)));
		D3D12_FEATURE_DATA_ARCHITECTURE architecture = {.NodeIndex = 0};
		ANKI_D3D_CHECK(m_device->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE, &architecture, sizeof(architecture)));
		D3D12_FEATURE_DATA_D3D12_OPTIONS21 options21;
		ANKI_D3D_CHECK(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS21, &options21, sizeof(options21)));

		if(g_workGraphcsCVar && options21.WorkGraphsTier == D3D12_WORK_GRAPHS_TIER_NOT_SUPPORTED)
		{
			ANKI_D3D_LOGW("WorkGraphs can't be enabled. They not supported");
		}
		else if(g_workGraphcsCVar && options21.WorkGraphsTier != D3D12_WORK_GRAPHS_TIER_NOT_SUPPORTED)
		{
			ANKI_D3D_LOGV("WorkGraphs supported");
			m_capabilities.m_workGraphs = true;
		}

		m_d3dCapabilities.m_rebar = options16.GPUUploadHeapSupported;
		if(!m_d3dCapabilities.m_rebar)
		{
			ANKI_D3D_LOGW("ReBAR not supported");
		}

		m_capabilities.m_constantBufferBindOffsetAlignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
		m_capabilities.m_structuredBufferBindOffsetAlignment = 0; // Not for DX
		m_capabilities.m_structuredBufferNaturalAlignment = true;
		m_capabilities.m_texelBufferBindOffsetAlignment = 32;
		m_capabilities.m_fastConstantsSize = kMaxFastConstantsSize;
		m_capabilities.m_computeSharedMemorySize = D3D12_CS_TGSM_REGISTER_COUNT * sizeof(F32);
		m_capabilities.m_accelerationStructureBuildScratchOffsetAlignment = 32; // ?
		m_capabilities.m_sbtRecordAlignment = 32; // ?
		m_capabilities.m_maxDrawIndirectCount = kMaxU32;
		m_capabilities.m_discreteGpu = !architecture.UMA;
		m_capabilities.m_majorApiVersion = 12;
		m_capabilities.m_rayTracingEnabled = g_rayTracingCVar && false; // TODO: Support RT
		m_capabilities.m_vrs = g_vrsCVar;
		m_capabilities.m_unalignedBbpTextureFormats = false;
		m_capabilities.m_dlss = false;
		m_capabilities.m_meshShaders = g_meshShadersCVar;
		m_capabilities.m_pipelineQuery = true;
		m_capabilities.m_barycentrics = true;
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

	IndirectCommandSignatureFactory::allocateSingleton();
	ANKI_CHECK(IndirectCommandSignatureFactory::getSingleton().init());

	TimestampQueryFactory::allocateSingleton();
	PrimitivesPassedClippingFactory::allocateSingleton();

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
	PrimitivesPassedClippingFactory::freeSingleton();
	TimestampQueryFactory::freeSingleton();
	SwapchainFactory::freeSingleton();
	FrameGarbageCollector::freeSingleton();
	RootSignatureFactory::freeSingleton();
	DescriptorFactory::freeSingleton();
	IndirectCommandSignatureFactory::freeSingleton();
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
	if(FenceFactory::isAllocated())
	{
		for(GpuQueueType queueType : EnumIterable<GpuQueueType>())
		{
			MicroFencePtr fence = FenceFactory::getSingleton().newInstance();
			fence->gpuSignal(queueType);
			fence->clientWait(kMaxSecond);
		}
	}
}

void GrManagerImpl::invokeDred() const
{
	Bool error = false;

	do
	{
		if(m_canInvokeDred)
		{
			ComPtr<ID3D12DeviceRemovedExtendedData> pDred;
			if(!SUCCEEDED(m_device->QueryInterface(IID_PPV_ARGS(&pDred))))
			{
				error = true;
				break;
			}

			D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT dredAutoBreadcrumbsOutput;
			if(!SUCCEEDED(pDred->GetAutoBreadcrumbsOutput(&dredAutoBreadcrumbsOutput)))
			{
				error = true;
				break;
			}

			D3D12_DRED_PAGE_FAULT_OUTPUT dredPageFaultOutput;
			if(!SUCCEEDED(pDred->GetPageFaultAllocationOutput(&dredPageFaultOutput)))
			{
				error = true;
				break;
			}
		}
	} while(false);
}

} // end namespace anki
