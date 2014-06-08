// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/core/Config.h"

namespace anki {

//==============================================================================
Config::Config()
{
	//
	// Renderer
	// 

	// Ms
	newOption("ms.ez.enabled", false);
	newOption("ms.ez.maxObjectsToDraw", 10);

	// Is
	newOption("is.sm.enabled", true);
	newOption("is.sm.poissonEnabled", true);
	newOption("is.sm.bilinearEnabled", true);
	newOption("is.sm.resolution", 512);
	newOption("is.sm.maxLights", 4);

	newOption("is.groundLightEnabled", true);
	newOption("is.maxPointLights", 512 - 16);
	newOption("is.maxSpotLights", 8);
	newOption("is.maxSpotTexLights", 4);
	newOption("is.maxPointLightsPerTile", 48);
	newOption("is.maxSpotLightsPerTile", 4);
	newOption("is.maxSpotTexLightsPerTile", 4);

	// Pps
	newOption("pps.hdr.enabled", true);
	newOption("pps.hdr.renderingQuality", 0.5);
	newOption("pps.hdr.blurringDist", 1.0);
	newOption("pps.hdr.samples", 5);
	newOption("pps.hdr.blurringIterationsCount", 1);
	newOption("pps.hdr.exposure", 4.0);

	newOption("pps.ssao.enabled", true);
	newOption("pps.ssao.renderingQuality", 0.3);
	newOption("pps.ssao.blurringIterationsCount", 1);

	newOption("pps.sslr.enabled", true);
	newOption("pps.sslr.renderingQuality", 0.2);
	newOption("pps.sslr.blurringIterationsCount", 1);

	newOption("pps.bl.enabled", true);
	newOption("pps.bl.blurringIterationsCount", 1);
	newOption("pps.bl.sideBlurFactor", 1.0);

	newOption("pps.lf.enabled", true);
	newOption("pps.lf.maxFlaresPerLight", 8);
	newOption("pps.lf.maxLightsWithFlares", 4);

	newOption("pps.enabled", true);
	newOption("pps.sharpen", true);
	newOption("pps.gammaCorrection", true);

	// Dbg
	newOption("dbg.enabled", false);

	// Globals
	newOption("width", 0);
	newOption("height", 0);
	newOption("renderingQuality", 1.0); // Applies only to MainRenderer
	newOption("lodDistance", 10.0); // Distance that used to calculate the LOD
	newOption("samples", 1);
	newOption("tilesXCount", 16);
	newOption("tilesYCount", 16);
	newOption("tessellation", true);

	newOption("offscreen", false);

	//
	// Resource
	//

	newOption("maxTextureSize", 1024 * 1024);
	newOption("textureAnisotropy", 8);
}

//==============================================================================
Config::~Config()
{}

} // end namespace anki

