// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

ANKI_CONFIG_VAR_GROUP(R)

ANKI_CONFIG_VAR_U8(RTextureAnisotropy, (ANKI_PLATFORM_MOBILE) ? 1 : 8, 1, 16, "Texture anisotropy for the main passes")
ANKI_CONFIG_VAR_U32(RTileSize, 64, 8, 256, "Tile lighting tile size")
ANKI_CONFIG_VAR_U32(RZSplitCount, 64, 8, 1024, "Clusterer number of Z splits")
ANKI_CONFIG_VAR_BOOL(RPreferCompute, !ANKI_PLATFORM_MOBILE, "Prefer compute shaders")
ANKI_CONFIG_VAR_BOOL(RVrs, true, "Enable VRS in multiple passes")
ANKI_CONFIG_VAR_F32(RVrsThreshold, 0.05f, 0.0f, 1.0f, "Threshold under which a lower shading rate will be applied")
ANKI_CONFIG_VAR_BOOL(RHighQualityHdr, !ANKI_PLATFORM_MOBILE,
					 "If true use R16G16B16 for HDR images. Alternatively use B10G11R11")

ANKI_CONFIG_VAR_F32(RInternalRenderScaling, 1.0f, 0.5f, 1.0f,
					"A factor over the requested swapchain resolution. Applies to all passes up to TAA")
ANKI_CONFIG_VAR_F32(RRenderScaling, 1.0f, 0.5f, 8.0f,
					"A factor over the requested swapchain resolution. Applies to post-processing and UI")

ANKI_CONFIG_VAR_F32(RVolumetricLightingAccumulationQualityXY, 4.0f, 1.0f, 16.0f,
					"Quality of XY dimensions of volumetric lights")
ANKI_CONFIG_VAR_F32(RVolumetricLightingAccumulationQualityZ, 4.0f, 1.0f, 16.0f,
					"Quality of Z dimension of volumetric lights")
ANKI_CONFIG_VAR_U32(RVolumetricLightingAccumulationFinalZSplit, 26, 1, 256,
					"Final cluster split that will recieve volumetric lights")

// SSR
ANKI_CONFIG_VAR_U32(RSsrFirstStepPixels, 32, 1, 256, "The 1st step in ray marching")
ANKI_CONFIG_VAR_U32(RSsrDepthLod, 2, 0, 1000, "Texture LOD of the depth texture that will be raymarched")
ANKI_CONFIG_VAR_U32(RSsrMaxSteps, 64, 1, 256, "Max SSR raymarching steps")
ANKI_CONFIG_VAR_BOOL(RSsrStochastic, false, "Stochastic reflections")

// GI probes
ANKI_CONFIG_VAR_U32(RIndirectDiffuseProbeTileResolution, (ANKI_PLATFORM_MOBILE) ? 16 : 32, 8, 32, "GI tile resolution")
ANKI_CONFIG_VAR_U32(RIndirectDiffuseProbeShadowMapResolution, 128, 4, 2048, "GI shadowmap resolution")
ANKI_CONFIG_VAR_U32(RIndirectDiffuseProbeMaxCachedProbes, 16, 4, 2048, "Max cached probes")
ANKI_CONFIG_VAR_U32(RIndirectDiffuseProbeMaxVisibleProbes, 8, 1, 256, "Max visible GI probes")

// GI
ANKI_CONFIG_VAR_U32(RIndirectDiffuseSsgiSampleCount, 8, 1, 1024, "SSGI sample count")
ANKI_CONFIG_VAR_F32(RIndirectDiffuseSsgiRadius, 2.0f, 0.1f, 100.0f, "SSGI radius in meters")
ANKI_CONFIG_VAR_U32(RIndirectDiffuseDenoiseSampleCount, 4, 1, 128, "Indirect diffuse denoise sample count")
ANKI_CONFIG_VAR_F32(RIndirectDiffuseSsaoStrength, 2.5f, 0.1f, 10.0f, "SSAO strength")
ANKI_CONFIG_VAR_F32(RIndirectDiffuseSsaoBias, -0.1f, -10.0f, 10.0f, "SSAO bias")
ANKI_CONFIG_VAR_F32(RIndirectDiffuseVrsDistanceThreshold, 0.01f, 0.00001f, 10.0f,
					"The meters that control the VRS SRI generation")

// Shadows
ANKI_CONFIG_VAR_U32(RShadowMappingTileResolution, (ANKI_PLATFORM_MOBILE) ? 128 : 512, 16, 2048,
					"Shadowmapping tile resolution")
ANKI_CONFIG_VAR_U32(RShadowMappingTileCountPerRowOrColumn, 16, 1, 256,
					"Shadowmapping atlas will have this number squared number of tiles")
ANKI_CONFIG_VAR_U32(RShadowMappingScratchTileCountX, 4 * (MAX_SHADOW_CASCADES2 + 2), 1, 256,
					"Number of tiles of the scratch buffer in X")
ANKI_CONFIG_VAR_U32(RShadowMappingScratchTileCountY, 4, 1, 256, "Number of tiles of the scratch buffer in Y")

// Probe reflections
ANKI_CONFIG_VAR_U32(RProbeReflectionResolution, 128, 4, 2048, "Reflection probe face resolution")
ANKI_CONFIG_VAR_U32(RProbeReflectionIrradianceResolution, 16, 4, 2048, "Reflection probe irradiance resolution")
ANKI_CONFIG_VAR_U32(RProbeRefectionMaxCachedProbes, 32, 4, 256, "Max cached number of reflection probes")
ANKI_CONFIG_VAR_U32(RProbeReflectionShadowMapResolution, 64, 4, 2048, "Reflection probe shadow resolution")

// Lens flare
ANKI_CONFIG_VAR_U8(RLensFlareMaxSpritesPerFlare, 8, 4, 255, "Max sprites per lens flare")
ANKI_CONFIG_VAR_U8(RLensFlareMaxFlares, 16, 8, 255, "Max flare count")

ANKI_CONFIG_VAR_U32(RMotionBlurSamples, 32, 1, 2048, "Max motion blur samples")

ANKI_CONFIG_VAR_BOOL(RDbgEnabled, false, "Enable or not debugging")

ANKI_CONFIG_VAR_F32(RBloomThreshold, 2.5f, 0.0f, 256.0f, "Bloom threshold")
ANKI_CONFIG_VAR_F32(RBloomScale, 2.5f, 0.0f, 256.0f, "Bloom scale")

ANKI_CONFIG_VAR_BOOL(RSmResolveQuarterRez, ANKI_PLATFORM_MOBILE, "Shadowmapping resolve quality")

ANKI_CONFIG_VAR_BOOL(RRtShadowsSvgf, false, "Enable or not RT shadows SVGF")
ANKI_CONFIG_VAR_U8(RRtShadowsSvgfAtrousPassCount, 3, 1, 20, "Number of atrous passes of SVGF")
ANKI_CONFIG_VAR_U32(RRtShadowsRaysPerPixel, 1, 1, 8, "Number of shadow rays per pixel")

ANKI_CONFIG_VAR_U8(RFsr, 1, 0, 2, "0: Use bilinear, 1: FSR low quality, 2: FSR high quality")
ANKI_CONFIG_VAR_BOOL(RSharpen, true, "Sharpen the image")
