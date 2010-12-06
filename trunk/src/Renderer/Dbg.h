#ifndef DBG_H
#define DBG_H

#include "RenderingPass.h"
#include "Fbo.h"
#include "ShaderProg.h"
#include "RsrcPtr.h"
#include "Math.h"


class Vbo;
class Vao;


/// Debugging stage
class Dbg: public RenderingPass
{
	public:
		Dbg(Renderer& r_, Object* parent);
		void init(const RendererInitializer& initializer);
		void run();

		void renderGrid();
		void drawSphere(float radius, const Transform& trf, const Vec4& col, int complexity = 8);
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
		static const uint POSITION_ATTRIBUTE_ID = 0; ///< The glId of the attribute var for position in the dbg shader
		bool enabled;
		bool showAxisEnabled;
		bool showLightsEnabled;
		bool showSkeletonsEnabled;
		bool showCamerasEnabled;
		Fbo fbo;
		RsrcPtr<ShaderProg> sProg;
		Mat4 viewProjectionMat;

		static const uint MAX_POINTS_PER_DRAW = 100;
		Vec3 positions[MAX_POINTS_PER_DRAW];
		Vec3 colors[MAX_POINTS_PER_DRAW];
		uint pointIndex;
		Vec3 crntCol;
		Vbo* positionsVbo;
		Vbo* colorsVbo;
		Vao* vao;
};


#endif
