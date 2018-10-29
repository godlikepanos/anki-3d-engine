// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/GBuffer.h>
#include <anki/renderer/Common.h>
#include <anki/renderer/LightShading.h>
#include <anki/renderer/VolumetricFog.h>
#include <anki/renderer/ForwardShading.h>
#include <anki/renderer/FinalComposite.h>
#include <anki/renderer/ClusterBin.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/ShadowMapping.h>
#include <anki/renderer/MainRenderer.h>
#include <anki/renderer/DownscaleBlur.h>
#include <anki/renderer/DepthDownscale.h>
#include <anki/renderer/LensFlare.h>
#include <anki/renderer/TemporalAA.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/renderer/Ssr.h>
#include <anki/renderer/Indirect.h>
#include <anki/renderer/Dbg.h>
#include <anki/renderer/Ssao.h>
#include <anki/renderer/Drawer.h>
#include <anki/renderer/UiStage.h>
#include <anki/renderer/Tonemapping.h>
#include <anki/renderer/RendererObject.h>
#include <anki/renderer/Bloom.h>
#include <anki/renderer/VolumetricLightingAccumulation.h>

/// @defgroup renderer Renderering system
