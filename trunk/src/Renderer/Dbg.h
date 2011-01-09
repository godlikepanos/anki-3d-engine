#ifndef DBG_H
#define DBG_H

#include <boost/array.hpp>
#include "RenderingPass.h"
#include "Fbo.h"
#include "ShaderProg.h"
#include "RsrcPtr.h"
#include "Math.h"
#include "Vbo.h"
#include "Vao.h"
#include "SceneDbgDrawer.h"


/// Debugging stage
class Dbg: public RenderingPass
{
	public:
		Dbg(Renderer& r_);
		void init(const RendererInitializer& initializer);
		void run();

		void renderGrid();
		void drawSphere(float radius, int complexity = 8);
		void drawCube(float size = 1.0);
		void drawLine(const Vec3& from, const Vec3& to, const Vec4& color);

		/// @name Accessors
		/// @{
		bool isEnabled() const {return enabled;}
		void setEnabled(bool flag) {enabled = flag;}
		bool isShowSkeletonsEnabled() const {return showSkeletonsEnabled;}
		void setShowSkeletonsEnabled(bool flag) {showSkeletonsEnabled = flag;}
		/// @todo add others
		/// @}

		/// @name Render functions. Imitate the GL 1.1 immediate mode
		/// @{
		void begin(); ///< Initiates the draw
		void end(); ///< Draws
		void pushBackVertex(const Vec3& pos); ///< Something like glVertex
		void setColor(const Vec3& col) {crntCol = col;} ///< Something like glColor
		void setColor(const Vec4& col) {crntCol = Vec3(col);} ///< Something like glColor
		void setModelMat(const Mat4& modelMat);
		/// @}

	private:
		bool enabled;
		bool showAxisEnabled;
		bool showLightsEnabled;
		bool showSkeletonsEnabled;
		bool showCamerasEnabled;
		Fbo fbo;
		RsrcPtr<ShaderProg> sProg;
		static const uint MAX_POINTS_PER_DRAW = 250;
		boost::array<Vec3, MAX_POINTS_PER_DRAW> positions;
		boost::array<Vec3, MAX_POINTS_PER_DRAW> colors;
		Mat4 modelMat;
		uint pointIndex;
		Vec3 crntCol;
		Vbo positionsVbo;
		Vbo colorsVbo;
		Vao vao;
		SceneDbgDrawer sceneDbgDrawer;
};


#endif
