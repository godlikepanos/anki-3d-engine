#ifndef ANKI_RENDERER_DBG_H
#define ANKI_RENDERER_DBG_H

#include <boost/array.hpp>
#include <map>
#include "anki/renderer/RenderingPass.h"
#include "anki/gl/Fbo.h"
#include "anki/resource/ShaderProgram.h"
#include "anki/resource/Resource.h"
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
	enum DebugFlag
	{
		DF_NONE = 0,
		DF_SPATIAL = 1,
		DF_MOVABLE = 2,
		DF_FRUSTUMABLE = 4
	};

	Dbg(Renderer& r_);

	/// @name Flag manipulation
	/// @{
	void enableFlag(DebugFlag flag, bool enable = true)
	{
		flags = enable ? flags | flag : flags & ~flag;
	}
	void disableFlag(DebugFlag flag)
	{
		enableFlag(flag, false);
	}
	bool isFlagEnabled(DebugFlag flag) const
	{
		return flags & flag;
	}
	uint getFlagsBitmask() const
	{
		return flags;
	}
	/// @}
	
	void init(const RendererInitializer& initializer);
	void run();
	
	void renderGrid();
	void drawSphere(float radius, int complexity = 4);
	void drawCube(float size = 1.0);
	void drawLine(const Vec3& from, const Vec3& to, const Vec4& color);

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
	uint flags;
	Fbo fbo;
	ShaderProgramResourcePointer sProg;
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
