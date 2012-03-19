#include "anki/renderer/Drawer.h"
#include "anki/resource/ShaderProgram.h"


namespace anki {


//==============================================================================
DebugDrawer::DebugDrawer()
{
	sProg.load("shaders/Dbg.glsl");

	positionsVbo.create(GL_ARRAY_BUFFER, sizeof(positions), NULL,
		GL_DYNAMIC_DRAW);
	colorsVbo.create(GL_ARRAY_BUFFER, sizeof(colors), NULL, GL_DYNAMIC_DRAW);
	vao.create();
	const int positionAttribLoc = 0;
	vao.attachArrayBufferVbo(positionsVbo, positionAttribLoc, 3, GL_FLOAT,
		GL_FALSE, 0, NULL);
	const int colorAttribLoc = 1;
	vao.attachArrayBufferVbo(colorsVbo, colorAttribLoc, 3, GL_FLOAT, GL_FALSE,
		0, NULL);

	pointIndex = 0;
	modelMat.setIdentity();
	crntCol = Vec3(1.0, 0.0, 0.0);
}


//==============================================================================
void DebugDrawer::drawLine(const Vec3& from, const Vec3& to, const Vec4& color)
{
	setColor(color);
	begin();
		pushBackVertex(from);
		pushBackVertex(to);
	end();
}


//==============================================================================
void DebugDrawer::drawGrid()
{
	Vec4 col0(0.5, 0.5, 0.5, 1.0);
	Vec4 col1(0.0, 0.0, 1.0, 1.0);
	Vec4 col2(1.0, 0.0, 0.0, 1.0);

	const float SPACE = 1.0; // space between lines
	const int NUM = 57;  // lines number. must be odd

	const float GRID_HALF_SIZE = ((NUM - 1) * SPACE / 2);

	setColor(col0);

	begin();

	for(int x = - NUM / 2 * SPACE; x < NUM / 2 * SPACE; x += SPACE)
	{
		setColor(col0);

		// if the middle line then change color
		if(x == 0)
		{
			setColor(col1);
		}

		// line in z
		pushBackVertex(Vec3(x, 0.0, -GRID_HALF_SIZE));
		pushBackVertex(Vec3(x, 0.0, GRID_HALF_SIZE));

		// if middle line change col so you can highlight the x-axis
		if(x == 0)
		{
			setColor(col2);
		}

		// line in the x
		pushBackVertex(Vec3(-GRID_HALF_SIZE, 0.0, x));
		pushBackVertex(Vec3(GRID_HALF_SIZE, 0.0, x));
	}

	// render
	end();
}


//==============================================================================
void DebugDrawer::drawSphere(float radius, int complexity)
{
	std::vector<Vec3>* sphereLines;

	// Pre-calculate the sphere points5
	//
	std::map<uint, std::vector<Vec3> >::iterator it =
		complexityToPreCalculatedSphere.find(complexity);

	if(it != complexityToPreCalculatedSphere.end()) // Found
	{
		sphereLines = &(it->second);
	}
	else // Not found
	{
		complexityToPreCalculatedSphere[complexity] = std::vector<Vec3>();
		sphereLines = &complexityToPreCalculatedSphere[complexity];

		float fi = Math::PI / complexity;

		Vec3 prev(1.0, 0.0, 0.0);
		for(float th = fi; th < Math::PI * 2.0 + fi; th += fi)
		{
			Vec3 p = Mat3(Euler(0.0, th, 0.0)) * Vec3(1.0, 0.0, 0.0);

			for(float th2 = 0.0; th2 < Math::PI; th2 += fi)
			{
				Mat3 rot(Euler(th2, 0.0, 0.0));

				Vec3 rotPrev = rot * prev;
				Vec3 rotP = rot * p;

				sphereLines->push_back(rotPrev);
				sphereLines->push_back(rotP);

				Mat3 rot2(Euler(0.0, 0.0, Math::PI / 2));

				sphereLines->push_back(rot2 * rotPrev);
				sphereLines->push_back(rot2 * rotP);
			}

			prev = p;
		}
	}

	// Render
	//
	modelMat = modelMat * Mat4(Vec3(0.0), Mat3::getIdentity(), radius);

	begin();
	for(const Vec3& p : *sphereLines)
	{
		if(pointIndex >= MAX_POINTS_PER_DRAW)
		{
			end();
			begin();
		}

		pushBackVertex(p);
	}
	end();
}


//==============================================================================
void DebugDrawer::drawCube(float size)
{
	Vec3 maxPos = Vec3(0.5 * size);
	Vec3 minPos = Vec3(-0.5 * size);

	std::array<Vec3, 8> points = {{
		Vec3(maxPos.x(), maxPos.y(), maxPos.z()),  // right top front
		Vec3(minPos.x(), maxPos.y(), maxPos.z()),  // left top front
		Vec3(minPos.x(), minPos.y(), maxPos.z()),  // left bottom front
		Vec3(maxPos.x(), minPos.y(), maxPos.z()),  // right bottom front
		Vec3(maxPos.x(), maxPos.y(), minPos.z()),  // right top back
		Vec3(minPos.x(), maxPos.y(), minPos.z()),  // left top back
		Vec3(minPos.x(), minPos.y(), minPos.z()),  // left bottom back
		Vec3(maxPos.x(), minPos.y(), minPos.z())   // right bottom back
	}};

	std::array<uint, 24> indeces = {{0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6,
		7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7}};

	begin();
		for(uint id : indeces)
		{
			pushBackVertex(points[id]);
		}
	end();
}


}  // namespace anki
