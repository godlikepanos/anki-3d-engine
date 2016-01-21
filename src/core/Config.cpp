// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/core/Config.h>

namespace anki
{

//==============================================================================
Config::Config()
{
	//
	// Renderer
	//

	// Is
	newOption("is.sm.enabled", true);
	newOption("is.sm.poissonEnabled", true);
	newOption("is.sm.bilinearEnabled", true);
	newOption("is.sm.resolution", 512);
	newOption("is.sm.maxLights", 4);

	newOption("is.groundLightEnabled", true);
	newOption("is.maxPointLights", 384);
	newOption("is.maxSpotLights", 16);
	newOption("is.maxSpotTexLights", 8);
	newOption("is.maxLightsPerCluster", 8);

	newOption("lf.maxSpritesPerFlare", 8);
	newOption("lf.maxFlares", 16);

	// Pps
	newOption("pps.bloom.enabled", true);
	newOption("pps.bloom.renderingQuality", 0.5);
	newOption("pps.bloom.blurringDist", 1.0);
	newOption("pps.bloom.samples", 5);
	newOption("pps.bloom.blurringIterationsCount", 1);
	newOption("pps.bloom.threshold", 1.0);
	newOption("pps.bloom.scale", 2.0);

	newOption("pps.ssao.enabled", true);
	newOption("pps.ssao.renderingQuality", 0.3);
	newOption("pps.ssao.blurringIterationsCount", 1);

	newOption("pps.bl.enabled", true);
	newOption("pps.bl.blurringIterationsCount", 1);
	newOption("pps.bl.sideBlurFactor", 1.0);

	newOption("pps.sslf.enabled", true);

	newOption("pps.enabled", true);
	newOption("pps.sharpen", true);
	newOption("pps.gammaCorrection", true);

	// Reflections
	newOption("ir.enabled", true);
	newOption("ir.rendererSize", 128);
	newOption("ir.cubemapTextureArraySize", 16);
	newOption("ir.clusterSizeZ", 16);
	newOption("sslr.enabled", true);
	newOption("sslr.startRoughnes", 0.2);

	// Dbg
	newOption("dbg.enabled", false);

	// Globals
	newOption("width", 0);
	newOption("height", 0);
	newOption("renderingQuality", 1.0); // Applies only to MainRenderer
	newOption("lodDistance", 10.0); // Distance that used to calculate the LOD
	newOption("samples", 1);
	newOption("tessellation", true);
	newOption("sceneFrameAllocatorSize", 1024 * 1024);
	newOption("clusterSizeZ", 32);
	newOption("imageReflectionMaxDistance", 30.0);

	//
	// GR
	//
	newOption("gr.frameUniformsSize", 1024 * 1024 * 16);
	newOption("gr.frameStorageSize", 1024 * 1024 * 16);
	newOption("gr.frameVertexSize", 1024 * 1024 * 2);
	newOption("gr.frameTransferSize", 1024 * 1024 * 32);

	//
	// Resource
	//
	newOption("maxTextureSize", 1024 * 1024);
	newOption("textureAnisotropy", 8);
	newOption("dataPaths", ".");

	//
	// Window
	//
	newOption("glminor", 5);
	newOption("glmajor", 4);
	newOption("fullscreenDesktopResolution", false);
	newOption("debugContext",
#if ANKI_DEBUG == 1
		true
#else
		false
#endif
		);
}

//==============================================================================
Config::~Config()
{
}

} // end namespace anki
