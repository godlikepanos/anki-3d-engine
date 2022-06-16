// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/DLSSCtxImpl.h>
#include <AnKi/Gr/Vulkan/GrManagerImpl.h>
#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Gr/Vulkan/CommandBufferImpl.h>

// Ngx specific
#include <ThirdParty/nvngx_dlss_sdk/sdk/include/nvsdk_ngx.h>
#include <ThirdParty/nvngx_dlss_sdk/sdk/include/nvsdk_ngx_helpers.h>
#include <ThirdParty/nvngx_dlss_sdk/sdk/include/nvsdk_ngx_vk.h>
#include <ThirdParty/nvngx_dlss_sdk/sdk/include/nvsdk_ngx_helpers_vk.h>

namespace anki {

// Use random ID
#define NGX_APP_ID 231313132
// TODO: Improve this
#define RW_LOGS_DIR L"./"

// See enum DLSSQualityMode
static NVSDK_NGX_PerfQuality_Value s_NVQUALITYMODES[] =
{
    NVSDK_NGX_PerfQuality_Value_MaxPerf, // DISABLED
    NVSDK_NGX_PerfQuality_Value_MaxPerf,
    NVSDK_NGX_PerfQuality_Value_Balanced,
    NVSDK_NGX_PerfQuality_Value_MaxQuality
};

static inline NVSDK_NGX_PerfQuality_Value dlssQualityModeToNVQualityMode(DLSSQualityMode mode) 
{
    return s_NVQUALITYMODES[static_cast<U32>(mode)];
}

DLSSCtxImpl::~DLSSCtxImpl()
{
    shutdown();
}

Error DLSSCtxImpl::init(const DLSSCtxInitInfo& init)
{
    NVSDK_NGX_Result result = NVSDK_NGX_Result_Fail;

    VkDevice vkdevice = getGrManagerImpl().getDevice();
    VkPhysicalDevice vkphysicaldevice = getGrManagerImpl().getPhysicalDevice();
    VkInstance vkinstance = getGrManagerImpl().getInstance();
    result = NVSDK_NGX_VULKAN_Init(NGX_APP_ID, RW_LOGS_DIR, vkinstance, vkphysicaldevice, vkdevice);

    m_ngxInitialized = !NVSDK_NGX_FAILED(result);
    if (!isNgxInitialized())
    {
        if (result == NVSDK_NGX_Result_FAIL_FeatureNotSupported || result == NVSDK_NGX_Result_FAIL_PlatformError) 
        {
            ANKI_VK_LOGE("NVIDIA NGX not available on this hardware/platform., code = 0x%08x, info: %ls", result, GetNGXResultAsString(result));
        }
        else 
        {
            ANKI_VK_LOGE("Failed to initialize NGX, error code = 0x%08x, info: %ls", result, GetNGXResultAsString(result));
        }

        return Error::FUNCTION_FAILED;
    }

    result = NVSDK_NGX_VULKAN_GetCapabilityParameters(&m_ngxParameters);

    if (NVSDK_NGX_FAILED(result))
    {
        ANKI_VK_LOGE("NVSDK_NGX_GetCapabilityParameters failed, code = 0x%08x, info: %ls", result, GetNGXResultAsString(result));
        shutdown();
        return Error::FUNCTION_FAILED;
    }

    const char** instanceExt(nullptr);
	const char** deviceExt(nullptr);
	U32 instanceExtCount(0);
	U32 deviceExtCount(0);
	NVSDK_NGX_Result queryExtResult = NVSDK_NGX_VULKAN_RequiredExtensions(&instanceExtCount, &instanceExt, &deviceExtCount, &deviceExt);

    // Currently, the SDK and this sample are not in sync.  The sample is a bit forward looking,
    // in this case.  This will likely be resolved very shortly, and therefore, the code below
    // should be thought of as needed for a smooth user experience.
#if defined(NVSDK_NGX_Parameter_SuperSampling_NeedsUpdatedDriver)        \
    && defined (NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMajor) \
    && defined (NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMinor)

    // If NGX Successfully initialized then it should set those flags in return
    int needsUpdatedDriver = 0;
    unsigned int minDriverVersionMajor = 0;
    unsigned int minDriverVersionMinor = 0;
    NVSDK_NGX_Result ResultUpdatedDriver = m_ngxParameters->Get(NVSDK_NGX_Parameter_SuperSampling_NeedsUpdatedDriver, &needsUpdatedDriver);
    NVSDK_NGX_Result ResultMinDriverVersionMajor = m_ngxParameters->Get(NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMajor, &minDriverVersionMajor);
    NVSDK_NGX_Result ResultMinDriverVersionMinor = m_ngxParameters->Get(NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMinor, &minDriverVersionMinor);
    if (ResultUpdatedDriver == NVSDK_NGX_Result_Success &&
        ResultMinDriverVersionMajor == NVSDK_NGX_Result_Success &&
        ResultMinDriverVersionMinor == NVSDK_NGX_Result_Success)
    {
        if (needsUpdatedDriver)
        {
            ANKI_VK_LOGE("NVIDIA DLSS cannot be loaded due to outdated driver. Minimum Driver Version required : %u.%u", minDriverVersionMajor, minDriverVersionMinor);

            shutdown();
            return Error::FUNCTION_FAILED;
        }
        else
        {
            ANKI_VK_LOGI("NVIDIA DLSS Minimum driver version was reported as : %u.%u", minDriverVersionMajor, minDriverVersionMinor);
        }
    }
    else
    {
        ANKI_VK_LOGI("NVIDIA DLSS Minimum driver version was not reported.");
    }

#endif

    int dlssAvailable = 0;
    NVSDK_NGX_Result resultDLSS = m_ngxParameters->Get(NVSDK_NGX_Parameter_SuperSampling_Available, &dlssAvailable);
    if (resultDLSS != NVSDK_NGX_Result_Success || !dlssAvailable)
    {
        // More details about what failed (per feature init result)
        NVSDK_NGX_Result FeatureInitResult = NVSDK_NGX_Result_Fail;
        NVSDK_NGX_Parameter_GetI(m_ngxParameters, NVSDK_NGX_Parameter_SuperSampling_FeatureInitResult, (int*)&FeatureInitResult);
        ANKI_VK_LOGE("NVIDIA DLSS not available on this hardware/platform., FeatureInitResult = 0x%08x, info: %ls", FeatureInitResult, GetNGXResultAsString(FeatureInitResult));
        shutdown();
        return Error::FUNCTION_FAILED;
    }

    // Create the feature
    if (createDLSSFeature(init.m_srcRes, init.m_dstRes, init.m_mode) != Error::NONE)
    {
        ANKI_VK_LOGE("Failed to create DLSS feature");
        shutdown();
        return Error::FUNCTION_FAILED;
    }

	return Error::NONE;
}

static NVSDK_NGX_Resource_VK getNGXResourceFromAnkiTexture(const TextureViewImpl& tex, Bool isUAV)
{
	NVSDK_NGX_Resource_VK resourceVK = {};
	VkImageView imageView = tex.getHandle();
	VkFormat format = convertFormat(tex.getTextureImpl().getFormat());
	VkImage image = tex.getTextureImpl().m_imageHandle;
	VkImageSubresourceRange subresourceRange = tex.getVkImageSubresourceRange();

	return NVSDK_NGX_Create_ImageView_Resource_VK(imageView, image, subresourceRange, format,
												  tex.getTextureImpl().getWidth(), tex.getTextureImpl().getHeight(), isUAV);
}

void DLSSCtxImpl::upscale(CommandBufferPtr cmdb, const TextureViewPtr& srcRt, const TextureViewPtr& dstRt,
						  const TextureViewPtr& mvRt, const TextureViewPtr& depthRt, const TextureViewPtr& exposure,
						  const Bool resetAccumulation, const Vec2& jitterOffset, const Vec2& mVScale)
{
    if (!isNgxInitialized())
    {
        ANKI_VK_LOGE("Attempt to upscale an image without NGX being initialized.");
        return;
    }

    const TextureViewImpl& srcViewImpl = static_cast<const TextureViewImpl&>(*srcRt);
	const TextureViewImpl& dstViewImpl = static_cast<const TextureViewImpl&>(*dstRt);
	const TextureViewImpl& mvViewImpl = static_cast<const TextureViewImpl&>(*mvRt);
	const TextureViewImpl& depthViewImpl = static_cast<const TextureViewImpl&>(*depthRt);
	const TextureViewImpl& exposureViewImpl = static_cast<const TextureViewImpl&>(*exposure);

    NVSDK_NGX_Resource_VK srcResVk = getNGXResourceFromAnkiTexture(srcViewImpl, false);
	NVSDK_NGX_Resource_VK dstResVk = getNGXResourceFromAnkiTexture(dstViewImpl, true);
	NVSDK_NGX_Resource_VK mvResVk = getNGXResourceFromAnkiTexture(mvViewImpl, false);
	NVSDK_NGX_Resource_VK depthResVk = getNGXResourceFromAnkiTexture(depthViewImpl, false);
	NVSDK_NGX_Resource_VK exposureResVk = getNGXResourceFromAnkiTexture(exposureViewImpl, true);

    NVSDK_NGX_Coordinates renderingOffset = {0, 0};
	NVSDK_NGX_Dimensions renderingSize = {srcViewImpl.getTextureImpl().getWidth(), srcViewImpl.getTextureImpl().getHeight()}; 

    NVSDK_NGX_VK_DLSS_Eval_Params vkDlssEvalParams;
	memset(&vkDlssEvalParams, 0, sizeof(vkDlssEvalParams));
	vkDlssEvalParams.Feature.pInColor = &srcResVk;
	vkDlssEvalParams.Feature.pInOutput = &dstResVk;
	vkDlssEvalParams.pInDepth = &depthResVk;
	vkDlssEvalParams.pInMotionVectors = &mvResVk;
	vkDlssEvalParams.pInExposureTexture = &exposureResVk;
	vkDlssEvalParams.InJitterOffsetX = jitterOffset.x();
	vkDlssEvalParams.InJitterOffsetY = jitterOffset.y();
	vkDlssEvalParams.Feature.InSharpness = m_recommendedSettings.m_recommendedSharpness;
	vkDlssEvalParams.InReset = resetAccumulation;
	vkDlssEvalParams.InMVScaleX = mVScale.x();
	vkDlssEvalParams.InMVScaleY = mVScale.y();
	vkDlssEvalParams.InColorSubrectBase = renderingOffset;
	vkDlssEvalParams.InDepthSubrectBase = renderingOffset;
	vkDlssEvalParams.InTranslucencySubrectBase = renderingOffset;
	vkDlssEvalParams.InMVSubrectBase = renderingOffset;
	vkDlssEvalParams.InRenderSubrectDimensions = renderingSize;

    CommandBufferImpl& cmdbImpl = static_cast<CommandBufferImpl&>(*cmdb);

    getGrManagerImpl().beginMarker(cmdbImpl.getHandle(), "DLSS");
	NVSDK_NGX_Result result = NGX_VULKAN_EVALUATE_DLSS_EXT(cmdbImpl.getHandle(), m_dlssFeature, m_ngxParameters, &vkDlssEvalParams);
	getGrManagerImpl().endMarker(cmdbImpl.getHandle());

    if(NVSDK_NGX_FAILED(result))
	{
		ANKI_LOGE("Failed to NVSDK_NGX_VULKAN_EvaluateFeature for DLSS, code = 0x%08x, info: %ls", result, GetNGXResultAsString(result));
	}
}

void DLSSCtxImpl::shutdown() 
{
    if (isNgxInitialized())
    {
        // WaitForIdle
        getManager().finish();

        if (m_dlssFeature != nullptr)
        {
            releaseDLSSFeature();
        }
        NVSDK_NGX_VULKAN_DestroyParameters(m_ngxParameters);
        NVSDK_NGX_VULKAN_Shutdown();
        m_ngxInitialized = false;
    }
}

Error DLSSCtxImpl::createDLSSFeature(const UVec2& srcRes, const UVec2& dstRes, const DLSSQualityMode mode)
{
    if (!isNgxInitialized())
    {
        ANKI_VK_LOGE("Attempt to create feature without NGX being initialized.");
        return Error::FUNCTION_FAILED;
    }

    U32 creationNodeMask = 1;
    U32 visibilityNodeMask = 1;
    I32 dlssCreateFeatureFlags = NVSDK_NGX_DLSS_Feature_Flags_None;
    NVSDK_NGX_Result resultDLSS = NVSDK_NGX_Result_Fail;

    // Next create features	(See NVSDK_NGX_DLSS_Feature_Flags in nvsdk_ngx_defs.h)
    dlssCreateFeatureFlags |= NVSDK_NGX_DLSS_Feature_Flags_MVLowRes;
    dlssCreateFeatureFlags |= NVSDK_NGX_DLSS_Feature_Flags_IsHDR;

    Error querySettingsResult = queryOptimalSettings(dstRes, mode, &m_recommendedSettings);
    if (querySettingsResult != Error::NONE || 
        (srcRes < m_recommendedSettings.m_dynamicMinimumRenderSize || srcRes > m_recommendedSettings.m_dynamicMaximumRenderSize))
    {
        ANKI_VK_LOGE("Selected DLSS mode or render resolution is not supported");
        return Error::FUNCTION_FAILED;
    }

    NVSDK_NGX_DLSS_Create_Params dlssCreateParams;
    memset(&dlssCreateParams, 0, sizeof(dlssCreateParams));
    dlssCreateParams.Feature.InWidth = srcRes.x();
    dlssCreateParams.Feature.InHeight = srcRes.y();
    dlssCreateParams.Feature.InTargetWidth = dstRes.x();
    dlssCreateParams.Feature.InTargetHeight = dstRes.y();
    dlssCreateParams.Feature.InPerfQualityValue = dlssQualityModeToNVQualityMode(m_recommendedSettings.m_qualityMode);
    dlssCreateParams.InFeatureCreateFlags = dlssCreateFeatureFlags;

    // Create the feature with a tmp CmdBuffer
    CommandBufferInitInfo cmdbinit;
    cmdbinit.m_flags = CommandBufferFlag::GENERAL_WORK | CommandBufferFlag::SMALL_BATCH;
    CommandBufferPtr cmdb = getManager().newCommandBuffer(cmdbinit);
    CommandBufferImpl& cmdbImpl = static_cast<CommandBufferImpl&>(*cmdb);
    
    cmdbImpl.beginRecordingExt();
    resultDLSS = NGX_VULKAN_CREATE_DLSS_EXT(cmdbImpl.getHandle(), creationNodeMask, visibilityNodeMask, &m_dlssFeature, m_ngxParameters, &dlssCreateParams);
    if (NVSDK_NGX_FAILED(resultDLSS))
    {
        ANKI_VK_LOGE("Failed to create DLSS Features = 0x%08x, info: %ls", resultDLSS, GetNGXResultAsString(resultDLSS));
        return Error::FUNCTION_FAILED;
    }
    cmdbImpl.endRecording();
    getGrManagerImpl().flushCommandBuffer(cmdbImpl.getMicroCommandBuffer(), false, {}, nullptr);

    return Error::NONE;
}

void DLSSCtxImpl::releaseDLSSFeature()
{
    if (!isNgxInitialized())
    {
        ANKI_VK_LOGE("Attempt to ReleaseDLSSFeatures without NGX being initialized.");
        return;
    }

    getManager().finish();
    
    NVSDK_NGX_Result resultDLSS = (m_dlssFeature != nullptr) ? NVSDK_NGX_VULKAN_ReleaseFeature(m_dlssFeature) : NVSDK_NGX_Result_Success;
    if (NVSDK_NGX_FAILED(resultDLSS))
    {
        ANKI_VK_LOGE("Failed to NVSDK_NGX_D3D12_ReleaseFeature, code = 0x%08x, info: %ls", resultDLSS, GetNGXResultAsString(resultDLSS));
        
    }

    m_dlssFeature = nullptr;
}

static Error queryNgxQualitySettings(const UVec2& displayRes,
    const NVSDK_NGX_PerfQuality_Value quality,
    NVSDK_NGX_Parameter* parameters,
    DLSSRecommendedSettings* outSettings)
{
    NVSDK_NGX_Result result = NGX_DLSS_GET_OPTIMAL_SETTINGS(parameters,
        displayRes.x(), displayRes.y(), quality,
        &outSettings->m_recommendedOptimalRenderSize[0], &outSettings->m_recommendedOptimalRenderSize[1],
        &outSettings->m_dynamicMaximumRenderSize[0], &outSettings->m_dynamicMaximumRenderSize[1],
        &outSettings->m_dynamicMinimumRenderSize[0], &outSettings->m_dynamicMinimumRenderSize[1],
        &outSettings->m_recommendedSharpness);

    if (NVSDK_NGX_FAILED(result))
    {
        outSettings->m_recommendedOptimalRenderSize = displayRes;
        outSettings->m_dynamicMaximumRenderSize = displayRes;
        outSettings->m_dynamicMinimumRenderSize = displayRes;
        outSettings->m_recommendedSharpness = 0.0f;

        ANKI_VK_LOGW("Querying Settings failed! code = 0x%08x, info: %ls", result, GetNGXResultAsString(result));
        return Error::FUNCTION_FAILED;
    }
    return Error::NONE;
}

Error DLSSCtxImpl::queryOptimalSettings(const UVec2& displayRes, const DLSSQualityMode mode, DLSSRecommendedSettings* outRecommendedSettings)
{
    ANKI_ASSERT(mode < DLSSQualityMode::COUNT);

    outRecommendedSettings->m_qualityMode = mode;
    Error err(Error::FUNCTION_FAILED);
    switch (mode) 
    {
    case DLSSQualityMode::PERFORMANCE:
        err = queryNgxQualitySettings(displayRes, NVSDK_NGX_PerfQuality_Value_MaxPerf, m_ngxParameters, outRecommendedSettings);
        break;
    case DLSSQualityMode::BALANCED:
        err = queryNgxQualitySettings(displayRes, NVSDK_NGX_PerfQuality_Value_Balanced, m_ngxParameters, outRecommendedSettings);
        break;
    case DLSSQualityMode::QUALITY:
        err = queryNgxQualitySettings(displayRes, NVSDK_NGX_PerfQuality_Value_MaxQuality, m_ngxParameters, outRecommendedSettings);
        break;
    default:
        break;
    }
    return err;
}

} // end namespace anki
