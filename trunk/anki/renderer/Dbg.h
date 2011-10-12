#ifndef ANKI_RENDERER_DBG_H
#define ANKI_RENDERER_DBG_H

#include <boost/array.hpp>
#include <map>
#include "anki/renderer/RenderingPass.h"
#include "anki/gl/Fbo.h"
#include "anki/resource/ShaderProgram.h"
#include "anki/resource/RsrcPtr.h"
#include "anki/math/Math.h"
#include "anki/gl/Vbo.h"
#include "anki/gl/Vao.h"
#include "anki/renderer/SceneDbgDrawer.h"
#include "anki/renderer/CollisionDbgDrawer.h"


namespace anki {


/// Debugging stage
class Dbg: public SwitchableRenderingPass
{
	public:
		Dbg(Renderer& r_);
		void init(const RendererInitializer& initializer);
		void run();

		void renderGrid();
		void drawSphere(float radius, int complexity = 4);
		void drawCube(float size = 1.0);
		void drawLine(const Vec3& from, const Vec3& to, const Vec4& color);

		/// @name Accessors
		/// @{
		bool getShowSkeletonsEnabled() const
		{
			return showSkeletonsEnabled;
		}
		bool& getShowSkeletonsEnabled()
		{
			return showSkeletonsEnabled;
		}
		void setShowSkeletonsEnabled(const bool x)
		{
			showSkeletonsEnabled = x;
		}
		/// @todo add others
		/// @}

		/// @name Render functions. Imitate the GL 1.1 immediate mode
		/// @{
		void begin(); ///< Initiates the draw
		void end(); ///< Draws
		void pushBackVertex(const Vec3& pos); ///< Something like glVertex
		/// Something like glColor
		void setColor(const Vec3& col) {crntCol = col;}
		/// Something like glColor
		void setColor(const Vec4& col) {crntCol = Vec3(col);}
		void setModelMat(const Mat4& modelMat);
		/// @}

	private:
		bool showAxisEnabled;
		bool showLightsEnabled;
		bool showSkeletonsEnabled;
		bool showCamerasEnabled;
		bool showVisibilityBoundingShapesFlag;
		Fbo fbo;
		RsrcPtr<ShaderProgram> sProg;
		static const uint MAX_POINTS_PER_DRAW = 256;
		boost::array<Vec3, MAX_POINTS_PER_DRAW> positions;
		boost::array<Vec3, MAX_POINTS_PER_DRAW> colors;
		Mat4 modelMat;
		uint pointIndex;
		Vec3 crntCol;
		Vbo positionsVbo;
		Vbo colorsVbo;
		Vao vao;
		SceneDbgDrawer sceneDbgDrawer;
		CollisionDbgDrawer collisionDbgDrawer;


		/// This is a container of some precalculated spheres. Its a map that
		/// from sphere complexity it returns a vector of lines (Vec3s in
		/// pairs)
		std::map<uint, std::vector<Vec3> > complexityToPreCalculatedSphere;
};


} // end namespace


#endif
