// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/core/Config.h>
#include <anki/util/System.h>

namespace anki
{

Config::Config()
{
	// Renderer
	newOption("r.renderingQuality", 1.0, "Rendering quality factor");
	newOption("r.lodDistance0", 10.0, "Distance that will be used to calculate the LOD 0");
	newOption("r.lodDistance1", 20.0, "Distance that will be used to calculate the LOD 1");
	newOption("r.clusterSizeX", 32);
	newOption("r.clusterSizeY", 26);
	newOption("r.clusterSizeZ", 32);
	newOption("r.maxLightsPerCluster", 8);

	newOption("r.shadowMapping.enabled", true);
	newOption("r.shadowMapping.resolution", 512);
	newOption("r.shadowMapping.tileCountPerRowOrColumn", 8);
	newOption("r.shadowMapping.scratchTileCount", 8);

	newOption("r.lensFlare.maxSpritesPerFlare", 8);
	newOption("r.lensFlare.maxFlares", 16);

	newOption("r.bloom.threshold", 2.5);
	newOption("r.bloom.scale", 2.5);

	newOption("r.finalComposite.sharpen", false);

	newOption("r.indirect.reflectionResolution", 128);
	newOption("r.indirect.maxSimultaneousProbeCount", 32);

	newOption("r.dbg.enabled", false);

	// Scene
	newOption("scene.imageReflectionMaxDistance", 30.0);
	newOption("scene.earlyZDistance", 10.0, "Objects with distance lower than that will be used in early Z");

	// Globals
	newOption("width", 1280);
	newOption("height", 768);
	newOption("tessellation", true);
	newOption("clearCaches", false);

	// Resource
	newOption("rsrc.maxTextureSize", 1024 * 1024);
	newOption("rsrc.textureAnisotropy", 8);
	newOption("rsrc.dataPaths", ".", "The engine loads assets only in from these paths. Separate them with :");
	newOption("rsrc.transferScratchMemorySize", 256_MB);

	// Window
	newOption("window.glmajor", 4);
	newOption("window.glminor", 5);
	newOption("window.fullscreenDesktopResolution", false);
	newOption("window.debugContext", false);
	newOption("window.vsync", false);
	newOption("window.debugMarkers", false);

	// GR
	newOption("gr.diskShaderCacheMaxSize", 10_MB);

	// Core
	newOption("core.uniformPerFrameMemorySize", 16_MB);
	newOption("core.storagePerFrameMemorySize", 16_MB);
	newOption("core.vertexPerFrameMemorySize", 10_MB);
	newOption("core.textureBufferPerFrameMemorySize", 1_MB);
	newOption("core.mainThreadCount", getCpuCoresCount() / 2);
	newOption("core.displayStats", false);
}

Config::~Config()
{
}

} // end namespace anki
