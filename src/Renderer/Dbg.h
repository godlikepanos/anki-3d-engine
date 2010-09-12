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
		void init(const RendererInitializer& initializer);
		void run();
		void renderGrid();
		void drawSphere(float radius, const Transform& trf, const Vec4& col, int complexity = 8);
		void drawCube(float size = 1.0);
		void setColor(const Vec4& color);
		void setModelMat(const Mat4& modelMat);
		void drawLine(const Vec3& from, const Vec3& to, const Vec4& color);

		/**
		 * @name Setters & getters
		 */
		/**@{*/
		bool isEnabled() const {return enabled;}
		void setEnabled(bool flag) {enabled = flag;}
		bool isShowSkeletonsEnabled() const {return showSkeletonsEnabled;}
		void setShowSkeletonsEnabled(bool flag) {showSkeletonsEnabled = flag;}
		/// @todo add others
		/**@}*/

	private:
		static const uint POSITION_ATTRIBUTE_ID = 0; ///< The glId of the attribute var for position in the dbg shader
		bool enabled;
		bool showAxisEnabled;
		bool showLightsEnabled;
		bool showSkeletonsEnabled;
		bool showCamerasEnabled;
		Fbo fbo;
		RsrcPtr<ShaderProg> sProg;
		const ShaderProg::UniVar* colorUniVar;
		const ShaderProg::UniVar* modelViewProjectionMatUniVar;
		Mat4 viewProjectionMat;
};


#endif
