#ifndef ANKI_RENDERER_DRAWER_H
#define ANKI_RENDERER_DRAWER_H

#include "anki/math/Math.h"
#include "anki/gl/Vbo.h"
#include "anki/gl/Vao.h"
#include "anki/resource/Resource.h"
#include <array>
#include <map>


namespace anki {


/// Draws simple primitives
class DebugDrawer
{
public:
	DebugDrawer();

	void drawGrid();
	void drawSphere(float radius, int complexity = 4);
	void drawCube(float size = 1.0);
	void drawLine(const Vec3& from, const Vec3& to, const Vec4& color);

	/// @name Render functions. Imitate the GL 1.1 immediate mode
	/// @{
	void begin(); ///< Initiates the draw
	void end(); ///< Draws
	void pushBackVertex(const Vec3& pos); ///< Something like glVertex
	/// Something like glColor
	void setColor(const Vec3& col)
	{
		crntCol = col;
	}
	/// Something like glColor
	void setColor(const Vec4& col)
	{
		crntCol = Vec3(col);
	}
	void setModelMat(const Mat4& modelMat);
	/// @}

private:
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

	/// This is a container of some precalculated spheres. Its a map that
	/// from sphere complexity it returns a vector of lines (Vec3s in
	/// pairs)
	std::map<uint, std::vector<Vec3>> complexityToPreCalculatedSphere;
};


}  // namespace anki


#endif
