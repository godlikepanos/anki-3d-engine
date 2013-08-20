#ifndef ANKI_RENDERER_RENDERER_H
#define ANKI_RENDERER_RENDERER_H

#include "anki/Math.h"
#include "anki/resource/TextureResource.h"
#include "anki/resource/ShaderProgramResource.h"
#include "anki/resource/Resource.h"
#include "anki/gl/Gl.h"
#include "anki/util/HighRezTimer.h"

#include "anki/renderer/Common.h"
#include "anki/renderer/Ms.h"
#include "anki/renderer/Is.h"
#include "anki/renderer/Pps.h"
#include "anki/renderer/Bs.h"
#include "anki/renderer/Dbg.h"
#include "anki/renderer/Tiler.h"
#include "anki/renderer/Drawer.h"

namespace anki {

/// A struct to initialize the renderer. It contains a few extra params for
/// the MainRenderer
struct RendererInitializer
{
	// Ms
	struct Ms
	{
		struct Ez
		{
			Bool enabled = false;
			U maxObjectsToDraw = 10;
		} ez;
	} ms;

	// Is
	struct Is
	{
		// Sm
		struct Sm
		{
			Bool enabled = true;
			Bool pcfEnabled = true;
			Bool bilinearEnabled = true;
			U32 resolution = 512;
			U32 maxLights = 4;
		} sm;
		Bool groundLightEnabled = true;
		U32 maxPointLights = 512 - 16;
		U32 maxSpotLights = 8;
		U32 maxSpotTexLights = 4;
		U32 maxPointLightsPerTile = 48;
		U32 maxSpotLightsPerTile = 4;
		U32 maxSpotTexLightsPerTile = 4;
	} is;

	// Pps
	struct Pps
	{
		// Hdr
		struct Hdr
		{
			Bool enabled = true;
			F32 renderingQuality = 0.25;
			F32 blurringDist = 1.0;
			F32 blurringIterationsCount = 2;
			F32 exposure = 4.0;
		} hdr;

		// Ssao
		struct Ssao
		{
			Bool enabled = true;
			F32 mainPassRenderingQuality = 0.3;
			F32 blurringRenderingQuality = 0.3;
			U32 blurringIterationsNum = 2;
		} ssao;

		// Bl
		struct Bl
		{
			Bool enabled = true;
			U blurringIterationsNum = 2;
			F32 sideBlurFactor = 1.0;
		} bl;

		// Lf
		struct Lf
		{
			Bool enabled = true;
			U8 maxFlaresPerLight = 8;
			U8 maxLightsWithFlares = 4;
		} lf;

		Bool enabled = false;
		Bool sharpen = false;
	} pps;

	// Dbg
	struct Dbg
	{
		Bool enabled = false;
	} dbg;

	// the globals
	U32 width;
	U32 height;
	F32 renderingQuality = 1.0; ///< Applies only to MainRenderer
	F32 lodDistance = 10.0; ///< Distance that used to calculate the LOD
	U32 samples = 1; ///< Enables multisampling

	// funcs
	RendererInitializer()
	{}

	RendererInitializer(const RendererInitializer& initializer)
	{
		memcpy(this, &initializer, sizeof(RendererInitializer));
	}
};

class Camera;
struct RendererInitializer;
class ModelNode;

/// Offscreen renderer. It is a class and not a namespace because we may need
/// external renderers for security cameras for example
class Renderer
{
	friend class Ms;

public:
	typedef RendererInitializer Initializer;

	/// The types of rendering a ModelNode
	enum ModelNodeRenderType
	{
		MNRT_MS, ///< In material stage
		MNRT_DP, ///< In a depth pass
		MNRT_BS  ///< In blending stage
	};

	Renderer();
	~Renderer();

	/// @name Accessors
	/// @{
	const Ms& getMs() const
	{
		return ms;
	}
	Ms& getMs()
	{
		return ms;
	}

	const Is& getIs() const
	{
		return is;
	}
	Is& getIs()
	{
		return is;
	}

	const Tiler& getTiler() const
	{
		return tiler;
	}
	Tiler& getTiler()
	{
		return tiler;
	}

	const Pps& getPps() const
	{
		return pps;
	}
	Pps& getPps()
	{
		return pps;
	}

	U32 getWidth() const
	{
		return width;
	}

	U32 getHeight() const
	{
		return height;
	}

	F32 getAspectRatio() const
	{
		return F32(width) / height;
	}

	U32 getFramesCount() const
	{
		return framesNum;
	}

	const Mat4& getViewProjectionMatrix() const
	{
		return viewProjectionMat; // XXX remove that crap
	}

	const SceneGraph& getSceneGraph() const
	{
		return *scene;
	}
	SceneGraph& getSceneGraph()
	{
		return *scene;
	}

	const RenderableDrawer& getSceneDrawer() const
	{
		return sceneDrawer;
	}
	RenderableDrawer& getSceneDrawer()
	{
		return sceneDrawer;
	}

	const Vec2& getPlanes() const
	{
		return planes;
	}
	const Vec2& getLimitsOfNearPlane() const
	{
		return limitsOfNearPlane;
	}
	const Vec2& getLimitsOfNearPlane2() const
	{
		return limitsOfNearPlane2;
	}
	Timestamp getPlanesUpdateTimestamp() const
	{
		return planesUpdateTimestamp;
	}
	/// @}

	/// Init the renderer given an initialization class
	/// @param initializer The initializer class
	void init(const RendererInitializer& initializer);

	/// This function does all the rendering stages and produces a final FAI
	void render(SceneGraph& scene);

	/// My version of gluUnproject
	/// @param windowCoords Window screen coords
	/// @param modelViewMat The modelview matrix
	/// @param projectionMat The projection matrix
	/// @param view The view vector
	/// @return The unprojected coords
	static Vec3 unproject(const Vec3& windowCoords,
		const Mat4& modelViewMat, const Mat4& projectionMat,
		const int view[4]);

	/// Draws a quad. Actually it draws 2 triangles because OpenGL will no
	/// longer support quads
	void drawQuad();

	void drawQuadInstanced(U32 primitiveCount);

	/// Calculate the planes needed for the calculation of the fragment
	/// position z in view space. Having the fragment's depth, the camera's
	/// zNear and zFar the z of the fragment is being calculated inside the
	/// fragment shader from:
	/// @code z = (- zFar * zNear) / (zFar - depth * (zFar - zNear))
	/// @endcode
	/// The above can be optimized and this method actually pre-calculates a
	/// few things in order to lift a few calculations from the fragment
	/// shader. So the z is:
	/// @code z =  -planes.y / (planes.x + depth) @endcode
	/// @param[in] cameraRange The zNear, zFar
	/// @param[out] planes The planes
	static void calcPlanes(const Vec2& cameraRange, Vec2& planes);

	/// Calculates two values needed for the calculation of the fragment
	/// position in view space.
	static void calcLimitsOfNearPlane(const class PerspectiveCamera& cam,
		Vec2& limitsOfNearPlane);

	/// Get the LOD given the distance of an object from the camera
	U calculateLod(F32 distance) const
	{
		return distance / lodDistance;
	}

	/// On some GPUs its optimal to clean after binding to an FBO and there is
	/// no use for it's previous contents. For other GPUs the clear will be 
	/// skipped
	void clearAfterBindingFbo(const GLenum cap);

protected:
	/// @name Rendering stages
	/// @{
	Ms ms; ///< Material rendering stage
	Is is; ///< Illumination rendering stage
	Pps pps; ///< Postprocessing rendering stage
	Bs bs; ///< Blending stage
	/// @}

	Tiler tiler;

	/// Width of the rendering. Don't confuse with the window width
	U32 width;
	/// Height of the rendering. Don't confuse with the window width
	U32 height;
	SceneGraph* scene; ///< Current scene
	RenderableDrawer sceneDrawer;
	F32 lodDistance; ///< Distance that used to calculate the LOD

	/// @name Optimization vars
	/// Used in other stages
	/// @{
	Timestamp planesUpdateTimestamp = getGlobTimestamp();

	/// Used to to calculate the frag pos in view space inside a few shader
	/// programs
	Vec2 planes;
	/// Used to to calculate the frag pos in view space inside a few shader
	/// programs
	Vec2 limitsOfNearPlane;
	/// Used to to calculate the frag pos in view space inside a few shader
	/// programs
	Vec2 limitsOfNearPlane2;
	/// @}

private:
	U framesNum; ///< Frame number
	Mat4 viewProjectionMat; ///< Precalculated in case anyone needs it XXX
	U8 samples;

	/// @name For drawing a quad into the active framebuffer
	/// @{
	Vbo quadPositionsVbo; ///< The VBO for quad positions
	Vao quadVao; ///< This VAO is used everywhere except material stage
	/// @}
};


} // end namespace


#endif
