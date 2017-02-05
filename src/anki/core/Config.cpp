// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/core/Config.h>

namespace anki
{

Config::Config()
{
	//
	// Renderer
	//

	newOption("sm.enabled", true);
	newOption("sm.poissonEnabled", true);
	newOption("sm.bilinearEnabled", true);
	newOption("sm.resolution", 512);
	newOption("sm.maxLights", 4);

	newOption("is.maxPointLights", 384);
	newOption("is.maxSpotLights", 16);
	newOption("is.maxSpotTexLights", 8);
	newOption("is.maxLightsPerCluster", 8);

	newOption("lf.maxSpritesPerFlare", 8);
	newOption("lf.maxFlares", 16);

	newOption("bloom.enabled", true);
	newOption("bloom.renderingQuality", 0.5);
	newOption("bloom.blurringDist", 1.0);
	newOption("bloom.samples", 5);
	newOption("bloom.blurringIterationsCount", 1);
	newOption("bloom.threshold", 1.0);
	newOption("bloom.scale", 2.0);

	newOption("ssao.enabled", true);
	newOption("ssao.renderingQuality", 0.3);
	newOption("ssao.blurringIterationsCount", 1);

	newOption("sslf.enabled", true);

	newOption("pps.enabled", true);
	newOption("pps.sharpen", false);
	newOption("pps.gammaCorrection", true);

	newOption("ir.enabled", true);
	newOption("ir.rendererSize", 128);
	newOption("ir.cubemapTextureArraySize", 32);
	newOption("sslr.enabled", true);
	newOption("sslr.startRoughnes", 0.2);

	newOption("dbg.enabled", false);
	newOption("tm.enabled", true);

	// Globals
	newOption("width", 1280);
	newOption("height", 768);
	newOption("renderingQuality", 1.0); // Applies only to MainRenderer
	newOption("lodDistance", 10.0); // Distance that used to calculate the LOD
	newOption("tessellation", true);
	newOption("clusterSizeX", 32);
	newOption("clusterSizeY", 26);
	newOption("clusterSizeZ", 32);
	newOption("imageReflectionMaxDistance", 30.0);

	// Resource
	newOption("maxTextureSize", 1024 * 1024);
	newOption("textureAnisotropy", 8);
	newOption("dataPaths", ".");

	// Window
	newOption("glmajor", 4);
	newOption("glminor", 5);
	newOption("fullscreenDesktopResolution", false);
	newOption("debugContext", false);
	newOption("vsync", false);

	// Core
	newOption("core.uniformPerFrameMemorySize", 1024 * 1024 * 16);
	newOption("core.storagePerFrameMemorySize", 1024 * 1024 * 16);
	newOption("core.vertexPerFrameMemorySize", 1024 * 1024 * 10);
	newOption("core.transferPerFrameMemorySize", 1024 * 1024 * 128);
	newOption("core.textureBufferPerFrameMemorySize", 1024 * 1024 * 1);
}

Config::~Config()
{
}

} // end namespace anki
