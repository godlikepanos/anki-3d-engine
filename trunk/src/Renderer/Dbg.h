#ifndef DBG_H
#define DBG_H

#include "Common.h"
#include "RenderingStage.h"
#include "Fbo.h"
#include "ShaderProg.h"
#include "RsrcPtr.h"
#include "Math.h"


/**
 * Debugging stage
 */
class Dbg: public RenderingStage
{
	public:
		Dbg(Renderer& r_);
		void renderGrid();
		void init(const RendererInitializer& initializer);
		void run();
		static void drawSphere(float radius, const Transform& trf, const Vec4& col, int complexity = 8);
		static void drawCube(float size = 1.0);
		static void setColor(const Vec4& color);
		static void setModelMat(const Mat4& modelMat);
		static void drawLine(const Vec3& from, const Vec3& to, const Vec4& color);

		/**
		 * @name Setters & getters
		 */
		/**@{*/
		bool isEnabled() const {return enabled;}
		void enable() {enabled = true;}
		bool isShowSkeletonsEnabled() const {return showSkeletonsEnabled;}
		/// @todo add others
		/**@}*/

	private:
		bool enabled;
		bool showAxisEnabled;
		bool showLightsEnabled;
		bool showSkeletonsEnabled;
		bool showCamerasEnabled;
		Fbo fbo;
		static RsrcPtr<ShaderProg> sProg;
		static const ShaderProg::UniVar* colorUniVar;
		static const ShaderProg::UniVar* modelViewProjectionMat;
		static Mat4 viewProjectionMat;
};


#endif
