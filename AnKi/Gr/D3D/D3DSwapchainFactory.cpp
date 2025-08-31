// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DSwapchainFactory.h>
#include <AnKi/Gr/D3D/D3DDescriptor.h>
#include <AnKi/Gr/D3D/D3DTexture.h>
#include <AnKi/Gr/D3D/D3DGrManager.h>
#include <AnKi/Window/NativeWindowSdl.h>

#if ANKI_WINDOWING_SYSTEM_SDL
#	include <AnKi/Window/NativeWindowSdl.h>
#	include <SDL3/SDL.h>
#endif

namespace anki {

MicroSwapchain::MicroSwapchain()
{
	if(initInternal())
	{
		ANKI_D3D_LOGF("Error creating the swapchain. Will not try to recover");
	}
}

MicroSwapchain::~MicroSwapchain()
{
	// First release the textures
	m_textures = {};

	for(ID3D12Resource* rsrc : m_rtvResources)
	{
		safeRelease(rsrc);
	}

	safeRelease(m_swapchain);
}

Error MicroSwapchain::initInternal()
{
	const NativeWindowSdl& window = static_cast<NativeWindowSdl&>(NativeWindow::getSingleton());

	const HWND hwnd =
		static_cast<HWND>(SDL_GetPointerProperty(SDL_GetWindowProperties(window.m_sdlWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL));

	ComPtr<IDXGIFactory2> factory2;
	ANKI_D3D_CHECK(CreateDXGIFactory2((g_cvarGrValidation) ? DXGI_CREATE_FACTORY_DEBUG : 0, IID_PPV_ARGS(&factory2)));

	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = kMaxFramesInFlight;
	swapChainDesc.Width = window.getWidth();
	swapChainDesc.Height = window.getHeight();
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Flags |= (!g_cvarGrVsync) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	ComPtr<IDXGISwapChain1> swapChain;
	// Swap chain needs the queue so that it can force a flush on it.
	ANKI_D3D_CHECK(factory2->CreateSwapChainForHwnd(&getGrManagerImpl().getCommandQueue(GpuQueueType::kGeneral), hwnd, &swapChainDesc, nullptr,
													nullptr, &swapChain));

	swapChain->QueryInterface(IID_PPV_ARGS(&m_swapchain));

	// Does not support fullscreen transitions.
	factory2->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

	m_backbufferIdx = m_swapchain->GetCurrentBackBufferIndex();

	// Store swapchain RTVs
	for(U32 i = 0; i < kMaxFramesInFlight; ++i)
	{
		ANKI_D3D_CHECK(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_rtvResources[i])));

		TextureInitInfo init("SwapchainImg");
		init.m_width = window.getWidth();
		init.m_height = window.getHeight();
		init.m_format = Format::kR8G8B8A8_Unorm;

		init.m_usage = TextureUsageBit::kRtvDsvRead | TextureUsageBit::kRtvDsvWrite | TextureUsageBit::kPresent;
		if(m_rtvResources[i]->GetDesc().Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
		{
			init.m_usage |= TextureUsageBit::kAllUav;
		}

		init.m_type = TextureType::k2D;

		TextureImpl* tex = newInstance<TextureImpl>(GrMemoryPool::getSingleton(), init.getName());
		m_textures[i].reset(tex);

		ANKI_CHECK(tex->initExternal(m_rtvResources[i], init));
	}

	return Error::kNone;
}

void MicroSwapchain::releaseInternal()
{
	SwapchainFactory::getSingleton().m_recycler.recycle(this);
}

MicroSwapchainPtr SwapchainFactory::newInstance()
{
	// Delete stale swapchains (they are stale because they are probably out of data) and always create a new one
	m_recycler.trimCache();

	// This is useless but call it to avoid assertions
	[[maybe_unused]] MicroSwapchain* dummy = m_recycler.findToReuse();
	ANKI_ASSERT(dummy == nullptr);

	return MicroSwapchainPtr(anki::newInstance<MicroSwapchain>(GrMemoryPool::getSingleton()));
}

} // end namespace anki
