// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/Common.h>
#include <AnKi/Renderer/LightShading.h>
#include <AnKi/Renderer/VolumetricFog.h>
#include <AnKi/Renderer/ForwardShading.h>
#include <AnKi/Renderer/FinalComposite.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/ShadowMapping.h>
#include <AnKi/Renderer/MainRenderer.h>
#include <AnKi/Renderer/DownscaleBlur.h>
#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Renderer/LensFlare.h>
#include <AnKi/Renderer/TemporalAA.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Renderer/Ssr.h>
#include <AnKi/Renderer/ProbeReflections.h>
#include <AnKi/Renderer/Dbg.h>
#include <AnKi/Renderer/Drawer.h>
#include <AnKi/Renderer/UiStage.h>
#include <AnKi/Renderer/Tonemapping.h>
#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/Bloom.h>
#include <AnKi/Renderer/VolumetricLightingAccumulation.h>
#include <AnKi/Renderer/GlobalIllumination.h>
#include <AnKi/Renderer/GenericCompute.h>
#include <AnKi/Renderer/ShadowmapsResolve.h>
#include <AnKi/Renderer/IndirectDiffuse.h>

/// @defgroup renderer Renderering system
