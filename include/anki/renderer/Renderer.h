// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/Common.h>
#include <anki/renderer/Drawer.h>
#include <anki/renderer/Clusterer.h>
#include <anki/Math.h>
#include <anki/Gr.h>
#include <anki/scene/Forward.h>
#include <anki/resource/Forward.h>
#include <anki/resource/ShaderResource.h>
#include <anki/core/Timestamp.h>

namespace anki
{

// Forward
class ConfigSet;
class ResourceManager;

/// @addtogroup renderer
/// @{

/// Offscreen renderer. It is a class and not a namespace because we may need
/// external renderers for security cameras for example
class Renderer
{
public:
	Renderer();

	~Renderer();

	Bool getIrEnabled() const
	{
		return m_ir.isCreated();
	}

	Ir& getIr()
	{
		return *m_ir;
	}

	Ms& getMs()
	{
		return *m_ms;
	}

	Is& getIs()
	{
		return *m_is;
	}

	Fs& getFs()
	{
		return *m_fs;
	}

	Tm& getTm()
	{
		return *m_tm;
	}

	Bool getSsaoEnabled() const
	{
		return m_ssao.isCreated();
	}

	Ssao& getSsao()
	{
		return *m_ssao;
	}

	Bool getBloomEnabled() const
	{
		return m_bloom.isCreated();
	}

	Bloom& getBloom()
	{
		return *m_bloom;
	}

	Bool getSslfEnabled() const
	{
		return m_sslf.isCreated();
	}

	Sslf& getSslf()
	{
		return *m_sslf;
	}

	Bool getPpsEnabled() const
	{
		return m_pps.isCreated();
	}

	Pps& getPps()
	{
		return *m_pps;
	}

	Dbg& getDbg()
	{
		return *m_dbg;
	}

	U32 getWidth() const
	{
		return m_width;
	}

	U32 getHeight() const
	{
		return m_height;
	}

	F32 getAspectRatio() const
	{
		return F32(m_width) / m_height;
	}

	/// Init the renderer.
	ANKI_USE_RESULT Error init(ThreadPool* threadpool,
		ResourceManager* resources,
		GrManager* gr,
		HeapAllocator<U8> alloc,
		StackAllocator<U8> frameAlloc,
		const ConfigSet& config,
		Timestamp* globTimestamp);

	/// Set the output of the renderer before calling #render.
	void setOutputFramebuffer(FramebufferPtr outputFb, U32 width, U32 height)
	{
		m_outputFb = outputFb;
		m_outputFbSize = UVec2(width, height);
	}

	/// This function does all the rendering stages and produces a final result.
	ANKI_USE_RESULT Error render(
		SceneNode& frustumableNode, U frustumIdx, CommandBufferPtr cmdb);

anki_internal:
	/// WARNING: If you change the tile size you need to change some shaders
	static const U TILE_SIZE = 64;

	void getOutputFramebuffer(FramebufferPtr& outputFb, U32& width, U32& height)
	{
		if(m_outputFb.isCreated())
		{
			outputFb = m_outputFb;
			width = m_outputFbSize.x();
			height = m_outputFbSize.y();
		}
	}

	U64 getFrameCount() const
	{
		return m_frameCount;
	}

	const FrustumComponent& getActiveFrustumComponent() const
	{
		ANKI_ASSERT(m_frc);
		return *m_frc;
	}

	FrustumComponent& getActiveFrustumComponent()
	{
		ANKI_ASSERT(m_frc);
		return *m_frc;
	}

	const RenderableDrawer& getSceneDrawer() const
	{
		return m_sceneDrawer;
	}

	RenderableDrawer& getSceneDrawer()
	{
		return m_sceneDrawer;
	}

	U getSamples() const
	{
		return m_samples;
	}

	Bool getTessellationEnabled() const
	{
		return m_tessellation;
	}

	U getTileCount() const
	{
		return m_tileCount;
	}

	const UVec2& getTileCountXY() const
	{
		return m_tileCountXY;
	}

	U getClusterCount() const
	{
		return m_clusterer.getClusterCount();
	}

	const Clusterer& getClusterer() const
	{
		return m_clusterer;
	}

	const ShaderPtr& getDrawQuadVertexShader() const
	{
		return m_drawQuadVert->getGrShader();
	}

	/// My version of gluUnproject
	/// @param windowCoords Window screen coords
	/// @param modelViewMat The modelview matrix
	/// @param projectionMat The projection matrix
	/// @param view The view vector
	/// @return The unprojected coords
	static Vec3 unproject(const Vec3& windowCoords,
		const Mat4& modelViewMat,
		const Mat4& projectionMat,
		const int view[4]);

	/// Draws a quad. Actually it draws 2 triangles because OpenGL will no
	/// longer support quads
	void drawQuad(CommandBufferPtr& cmdb)
	{
		drawQuadInstanced(cmdb, 1);
	}

	void drawQuadConditional(OcclusionQueryPtr& q, CommandBufferPtr& cmdb)
	{
		cmdb->drawArraysConditional(q, 3, 1);
	}

	void drawQuadInstanced(CommandBufferPtr& cmdb, U32 primitiveCount)
	{
		cmdb->drawArrays(3, primitiveCount);
	}

	/// Get the LOD given the distance of an object from the camera
	F32 calculateLod(F32 distance) const
	{
		return distance / m_lodDistance;
	}

	/// Create a pipeline object that has as a vertex shader the m_drawQuadVert
	/// and the given fragment progam
	void createDrawQuadPipeline(
		ShaderPtr frag, const ColorStateInfo& colorState, PipelinePtr& ppline);

	/// Create a framebuffer attachment texture
	void createRenderTarget(U32 w,
		U32 h,
		const PixelFormat& format,
		U32 samples,
		SamplingFilter filter,
		U mipsCount,
		TexturePtr& rt);

	Bool doGpuVisibilityTest(const CollisionShape& cs, const Aabb& aabb) const;

	void prepareForVisibilityTests(const SceneNode& cam);

	GrManager& getGrManager()
	{
		return *m_gr;
	}

	HeapAllocator<U8> getAllocator() const
	{
		return m_alloc;
	}

	StackAllocator<U8> getFrameAllocator() const
	{
		return m_frameAlloc;
	}

	ResourceManager& getResourceManager()
	{
		return *m_resources;
	}

	ThreadPool& getThreadPool()
	{
		return *m_threadpool;
	}

	const DynamicBufferToken& getCommonUniformsDynamicBufferToken() const
	{
		return m_commonUniformsToken;
	}

	Timestamp getGlobalTimestamp() const
	{
		return *m_globTimestamp;
	}

	Timestamp* getGlobalTimestampPtr()
	{
		return m_globTimestamp;
	}

private:
	ThreadPool* m_threadpool;
	ResourceManager* m_resources;
	GrManager* m_gr;
	Timestamp* m_globTimestamp;
	HeapAllocator<U8> m_alloc;
	StackAllocator<U8> m_frameAlloc;

	Clusterer m_clusterer;

	DynamicBufferToken m_commonUniformsToken;

	/// @name Rendering stages
	/// @{
	UniquePtr<Ir> m_ir;
	UniquePtr<Ms> m_ms; ///< Material rendering stage
	UniquePtr<Is> m_is; ///< Illumination rendering stage
	UniquePtr<Fs> m_fs; ///< Forward shading.
	UniquePtr<Lf> m_lf; ///< Forward shading lens flares.
	UniquePtr<Upsample> m_upsample;
	UniquePtr<Tiler> m_tiler;
	UniquePtr<DownscaleBlur> m_downscale;
	UniquePtr<Tm> m_tm;
	UniquePtr<Ssao> m_ssao;
	UniquePtr<Bloom> m_bloom;
	UniquePtr<Sslf> m_sslf;
	UniquePtr<Pps> m_pps; ///< Postprocessing rendering stage
	UniquePtr<Dbg> m_dbg; ///< Debug stage.
	/// @}

	U32 m_width;
	U32 m_height;

	F32 m_lodDistance; ///< Distance that used to calculate the LOD
	U8 m_samples; ///< Number of sample in multisampling
	Bool8 m_tessellation;
	U32 m_tileCount;
	UVec2 m_tileCountXY;

	ShaderResourcePtr m_drawQuadVert;

	FrustumComponent* m_frc = nullptr; ///< Cache current frustum component.
	RenderableDrawer m_sceneDrawer;

	U64 m_frameCount; ///< Frame number

	FramebufferPtr m_outputFb;
	UVec2 m_outputFbSize;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);
};
/// @}

} // end namespace anki
