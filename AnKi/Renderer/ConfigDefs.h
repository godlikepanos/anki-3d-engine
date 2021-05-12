// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

ANKI_CONFIG_OPTION(r_clusterSizeX, 32, 1, 256)
ANKI_CONFIG_OPTION(r_clusterSizeY, 26, 1, 256)
ANKI_CONFIG_OPTION(r_clusterSizeZ, 32, 1, 256)
ANKI_CONFIG_OPTION(r_textureAnisotropy, 8, 1, 16)
ANKI_CONFIG_OPTION(r_tileSize, 64, 8, 256, "Tile lighting tile size")
ANKI_CONFIG_OPTION(r_zSplitCount, 64, 8, 1024, "Clusterer number of Z splits")

ANKI_CONFIG_OPTION(r_renderingQuality, 1.0, 0.5, 1.0, "A factor over the requested renderingresolution")

ANKI_CONFIG_OPTION(r_volumetricLightingAccumulationQualityXY, 4.0, 1.0, 16.0)
ANKI_CONFIG_OPTION(r_volumetricLightingAccumulationQualityZ, 4.0, 1.0, 16.0)
ANKI_CONFIG_OPTION(r_volumetricLightingAccumulationFinalZSplit, 26, 1, 256)

ANKI_CONFIG_OPTION(r_ssrMaxSteps, 64, 1, 2048)
ANKI_CONFIG_OPTION(r_ssrDepthLod, 2, 0, 1000)

ANKI_CONFIG_OPTION(r_ssgiMaxSteps, 32, 1, 2048)
ANKI_CONFIG_OPTION(r_ssgiDepthLod, 2, 0, 1000)

ANKI_CONFIG_OPTION(r_shadowMappingTileResolution, 128, 16, 2048)
ANKI_CONFIG_OPTION(r_shadowMappingTileCountPerRowOrColumn, 16, 1, 256)
ANKI_CONFIG_OPTION(r_shadowMappingScratchTileCountX, 4 * (MAX_SHADOW_CASCADES + 2), 1u, 256u,
				   "Number of tiles of the scratch buffer in X")
ANKI_CONFIG_OPTION(r_shadowMappingScratchTileCountY, 4, 1, 256, "Number of tiles of the scratch buffer in Y")

ANKI_CONFIG_OPTION(r_probeReflectionResolution, 128, 4, 2048)
ANKI_CONFIG_OPTION(r_probeReflectionIrradianceResolution, 16, 4, 2048)
ANKI_CONFIG_OPTION(r_probeRefectionlMaxSimultaneousProbeCount, 32, 4, 256)
ANKI_CONFIG_OPTION(r_probeReflectionShadowMapResolution, 64, 4, 2048)

ANKI_CONFIG_OPTION(r_lensFlareMaxSpritesPerFlare, 8, 4, 256)
ANKI_CONFIG_OPTION(r_lensFlareMaxFlares, 16, 8, 256)

ANKI_CONFIG_OPTION(r_giTileResolution, 32, 4, 2048)
ANKI_CONFIG_OPTION(r_giShadowMapResolution, 128, 4, 2048)
ANKI_CONFIG_OPTION(r_giMaxCachedProbes, 16, 4, 2048)
ANKI_CONFIG_OPTION(r_giMaxVisibleProbes, 8, 1, 256)

ANKI_CONFIG_OPTION(r_motionBlurSamples, 32, 1, 2048)

ANKI_CONFIG_OPTION(r_dbgEnabled, 0, 0, 1)

ANKI_CONFIG_OPTION(r_avgObjectsPerCluster, 16, 16, 256)

ANKI_CONFIG_OPTION(r_bloomThreshold, 2.5, 0.0, 256.0)
ANKI_CONFIG_OPTION(r_bloomScale, 2.5, 0.0, 256.0)

ANKI_CONFIG_OPTION(r_smResolveFactor, 0.5, 0.25, 1.0)

ANKI_CONFIG_OPTION(r_rtShadowsSvgf, 0, 0, 1)
ANKI_CONFIG_OPTION(r_rtShadowsSvgfAtrousPassCount, 1, 1, 20)
ANKI_CONFIG_OPTION(r_rtShadowsRaysPerPixel, 1, 1, 8)
