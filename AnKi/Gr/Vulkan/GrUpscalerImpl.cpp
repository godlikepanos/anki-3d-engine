// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/GrUpscalerImpl.h>
#include <AnKi/Gr/Vulkan/GrManagerImpl.h>
#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Gr/Fence.h>
#include <AnKi/Gr/Vulkan/CommandBufferImpl.h>

// Ngx specific
#if ANKI_DLSS
#	include <ThirdParty/DlssSdk/sdk/include/nvsdk_ngx.h>
#	include <ThirdParty/DlssSdk/sdk/include/nvsdk_ngx_helpers.h>
#	include <ThirdParty/DlssSdk/sdk/include/nvsdk_ngx_vk.h>
#	include <ThirdParty/DlssSdk/sdk/include/nvsdk_ngx_helpers_vk.h>
#endif

namespace anki {

GrUpscalerImpl::~GrUpscalerImpl()
{
#if ANKI_DLSS
	if(m_upscalerType == GrUpscalerType::DLSS_2)
	{
		destroyDlss();
	}
#endif
}

Error GrUpscalerImpl::initInternal(const GrUpscalerInitInfo& initInfo)
{
	m_upscalerType = initInfo.m_upscalerType;

#if ANKI_DLSS
	ANKI_ASSERT(initInfo.m_upscalerType == GrUpscalerType::DLSS_2);
	ANKI_CHECK(initDlss(initInfo));
#else
	ANKI_ASSERT(!"Not supported");
#endif

	return Error::NONE;
}

// ==== DLSS ====
#if ANKI_DLSS

#	define ANKI_NGX_CHECK(x_) \
		do \
		{ \
			const NVSDK_NGX_Result result = x_; \
			if(NVSDK_NGX_FAILED(result)) \
			{ \
				ANKI_VK_LOGE("DLSS failed to initialize %ls", GetNGXResultAsString(result)); \
				return Error::FUNCTION_FAILED; \
			} \
		} while(0)

static NVSDK_NGX_PerfQuality_Value getDlssQualityModeToNVQualityMode(GrUpscalerQualityMode mode)
{
	static Array<NVSDK_NGX_PerfQuality_Value, U32(GrUpscalerQualityMode::COUNT)> nvQualityModes = {
		NVSDK_NGX_PerfQuality_Value_MaxPerf, NVSDK_NGX_PerfQuality_Value_Balanced,
		NVSDK_NGX_PerfQuality_Value_MaxQuality};

	return nvQualityModes[mode];
}

Error GrUpscalerImpl::initDlss(const GrUpscalerInitInfo& initInfo)
{
	const VkDevice vkdevice = getGrManagerImpl().getDevice();
	const VkPhysicalDevice vkphysicaldevice = getGrManagerImpl().getPhysicalDevice();
	const VkInstance vkinstance = getGrManagerImpl().getInstance();
	const U32 ngxAppId = 231313132; // Something random
	const WChar* appLogs = L"./";
	ANKI_NGX_CHECK(NVSDK_NGX_VULKAN_Init(ngxAppId, appLogs, vkinstance, vkphysicaldevice, vkdevice));
	m_ngxInitialized = true;

	ANKI_NGX_CHECK(NVSDK_NGX_VULKAN_GetCapabilityParameters(&m_ngxParameters));
	ANKI_ASSERT(m_ngxParameters);

	// Currently, the SDK and this sample are not in sync.  The sample is a bit forward looking,
	// in this case.  This will likely be resolved very shortly, and therefore, the code below
	// should be thought of as needed for a smooth user experience.
#	if defined(NVSDK_NGX_Parameter_SuperSampling_NeedsUpdatedDriver) \
		&& defined(NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMajor) \
		&& defined(NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMinor)

	// If NGX Successfully initialized then it should set those flags in return
	I32 needsUpdatedDriver = 0;
	ANKI_NGX_CHECK(m_ngxParameters->Get(NVSDK_NGX_Parameter_SuperSampling_NeedsUpdatedDriver, &needsUpdatedDriver));

	if(needsUpdatedDriver)
	{
		ANKI_VK_LOGE("DLSS cannot be loaded due to outdated driver");
		return Error::FUNCTION_FAILED;
	}
#	endif

	I32 dlssAvailable = 0;
	ANKI_NGX_CHECK(m_ngxParameters->Get(NVSDK_NGX_Parameter_SuperSampling_Available, &dlssAvailable));
	if(!dlssAvailable)
	{
		ANKI_VK_LOGE("NVIDIA DLSS not available on this hardware/platform");
		return Error::FUNCTION_FAILED;
	}

	// Create the feature
	ANKI_CHECK(createDlssFeature(initInfo.m_sourceTextureResolution, initInfo.m_targetTextureResolution,
								 initInfo.m_qualityMode));

	return Error::NONE;
}

Error GrUpscalerImpl::createDlssFeature(const UVec2& srcRes, const UVec2& dstRes, const GrUpscalerQualityMode quality)
{
	NVSDK_NGX_PerfQuality_Value nvQuality = getDlssQualityModeToNVQualityMode(quality);
	ANKI_NGX_CHECK(NGX_DLSS_GET_OPTIMAL_SETTINGS(
		m_ngxParameters, dstRes.x(), dstRes.y(), nvQuality, &m_recommendedSettings.m_optimalRenderSize.x(),
		&m_recommendedSettings.m_optimalRenderSize.y(), &m_recommendedSettings.m_dynamicMaximumRenderSize.x(),
		&m_recommendedSettings.m_dynamicMaximumRenderSize.y(), &m_recommendedSettings.m_dynamicMinimumRenderSize.x(),
		&m_recommendedSettings.m_dynamicMinimumRenderSize.y(), &m_recommendedSettings.m_sharpness));

	// Next create features	(See NVSDK_NGX_DLSS_Feature_Flags in nvsdk_ngx_defs.h)
	const I32 dlssCreateFeatureFlags =
		NVSDK_NGX_DLSS_Feature_Flags_MVLowRes | NVSDK_NGX_DLSS_Feature_Flags_IsHDR; // TODO

	NVSDK_NGX_DLSS_Create_Params dlssCreateParams;
	memset(&dlssCreateParams, 0, sizeof(dlssCreateParams));
	dlssCreateParams.Feature.InWidth = srcRes.x();
	dlssCreateParams.Feature.InHeight = srcRes.y();
	dlssCreateParams.Feature.InTargetWidth = dstRes.x();
	dlssCreateParams.Feature.InTargetHeight = dstRes.y();
	dlssCreateParams.Feature.InPerfQualityValue = nvQuality;
	dlssCreateParams.InFeatureCreateFlags = dlssCreateFeatureFlags;

	// Create the feature with a tmp CmdBuffer
	CommandBufferInitInfo cmdbinit;
	cmdbinit.m_flags = CommandBufferFlag::GENERAL_WORK | CommandBufferFlag::SMALL_BATCH;
	CommandBufferPtr cmdb = getManager().newCommandBuffer(cmdbinit);
	CommandBufferImpl& cmdbImpl = static_cast<CommandBufferImpl&>(*cmdb);

	cmdbImpl.beginRecordingExt();
	const U32 creationNodeMask = 1;
	const U32 visibilityNodeMask = 1;
	ANKI_NGX_CHECK(NGX_VULKAN_CREATE_DLSS_EXT(cmdbImpl.getHandle(), creationNodeMask, visibilityNodeMask,
											  &m_dlssFeature, m_ngxParameters, &dlssCreateParams));
	FencePtr fence;
	cmdb->flush({}, &fence);
	fence->clientWait(60.0_sec);

	return Error::NONE;
}

void GrUpscalerImpl::destroyDlss()
{
	if(m_dlssFeature)
	{
		NVSDK_NGX_VULKAN_ReleaseFeature(m_dlssFeature);
		m_dlssFeature = nullptr;
	}

	if(m_ngxParameters)
	{
		NVSDK_NGX_VULKAN_DestroyParameters(m_ngxParameters);
		m_ngxParameters = nullptr;
	}

	if(m_ngxInitialized)
	{
		NVSDK_NGX_VULKAN_Shutdown();
		m_ngxInitialized = false;
	}
}
#endif // ANKI_DLSS

} // end namespace anki
