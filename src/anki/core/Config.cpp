// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/core/Config.h>
#include <anki/util/System.h>
#include <anki/Math.h>
#include <shaders/glsl_cpp_common/ClusteredShading.h>

namespace anki
{

Config::Config()
{
	// Renderer
	newOption("r.renderingQuality", 1.0, "Rendering quality factor");
	newOption("r.lodDistance0", 20.0, "Distance that will be used to calculate the LOD 0");
	newOption("r.lodDistance1", 40.0, "Distance that will be used to calculate the LOD 1");
	newOption("r.clusterSizeX", 32);
	newOption("r.clusterSizeY", 26);
	newOption("r.clusterSizeZ", 32);
	newOption("r.avgObjectsPerCluster", 16);
	newOption("r.textureAnisotropy", 8);

	newOption("r.volumetricLightingAccumulation.clusterFractionXY", 4);
	newOption("r.volumetricLightingAccumulation.clusterFractionZ", 4);
	newOption("r.volumetricLightingAccumulation.finalClusterInZ", 26);

	newOption("r.shadowMapping.enabled", true);
	newOption("r.shadowMapping.tileResolution", 128);
	newOption("r.shadowMapping.tileCountPerRowOrColumn", 16);
	newOption("r.shadowMapping.scratchTileCountX", 4 * (MAX_SHADOW_CASCADES + 2));
	newOption("r.shadowMapping.scratchTileCountY", 4);
	newOption("r.shadowMapping.lightLodDistance0", 10.0);
	newOption("r.shadowMapping.lightLodDistance1", 20.0);

	newOption("r.lensFlare.maxSpritesPerFlare", 8);
	newOption("r.lensFlare.maxFlares", 16);

	newOption("r.bloom.threshold", 2.5);
	newOption("r.bloom.scale", 2.5);

	newOption("r.indirect.reflectionResolution", 128);
	newOption("r.indirect.irradianceResolution", 16);
	newOption("r.indirect.maxSimultaneousProbeCount", 32);
	newOption("r.indirect.shadowMapResolution", 64);

	newOption("r.gi.tileResolution", 32);
	newOption("r.gi.shadowMapResolution", 128);
	newOption("r.gi.maxCachedProbes", 16);
	newOption("r.gi.maxVisibleProbes", 4);
	newOption("r.gi.firstClipmapLevelCellSize", 1.0);
	newOption("r.gi.secondClipmapLevelCellSize", 8.0);
	newOption("r.gi.firstClipmapMaxDistance", 20.0);

	newOption("r.motionBlur.maxSamples", 32);

	newOption("r.ssr.maxSteps", 64);
	newOption("r.ssr.historyBlendFactor", 0.3);

	newOption("r.dbg.enabled", false);

	newOption("r.final.motionBlurSamples", 32);

	// Scene
	newOption("scene.earlyZDistance", 10.0, "Objects with distance lower than that will be used in early Z");
	newOption("scene.reflectionProbeEffectiveDistance", 256.0, "How far reflection probes can look");
	newOption("scene.reflectionProbeShadowEffectiveDistance", 32.0, "How far to render shadows for reflection probes");

	// Globals
	newOption("width", 1280);
	newOption("height", 768);

	// Resource
	newOption("rsrc.maxTextureSize", 1024 * 1024);
	newOption("rsrc.dataPaths", ".", "The engine loads assets only in from these paths. Separate them with :");
	newOption("rsrc.transferScratchMemorySize", 256_MB);

	// Window
	newOption("window.fullscreen", false);
	newOption("window.debugContext", false);
	newOption("window.vsync", false);
	newOption("window.debugMarkers", false);

	// GR
	newOption("gr.diskShaderCacheMaxSize", 10_MB);
	newOption("gr.vkminor", 1);
	newOption("gr.vkmajor", 1);
	newOption("gr.glmajor", 4);
	newOption("gr.glminor", 5);

	// Core
	newOption("core.uniformPerFrameMemorySize", 16_MB);
	newOption("core.storagePerFrameMemorySize", 16_MB);
	newOption("core.vertexPerFrameMemorySize", 10_MB);
	newOption("core.textureBufferPerFrameMemorySize", 1_MB);
	newOption("core.mainThreadCount", max(2u, getCpuCoresCount() / 2u - 1u));
	newOption("core.displayStats", false);
	newOption("core.clearCaches", false);
}

Config::~Config()
{
}

} // end namespace anki
