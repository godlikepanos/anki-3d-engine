#include <iostream>
#include <GL/glew.h>
#include "MeshData.h"
#include "Collision.h"
#include "App.h"
#include "Shredder.h"


struct TempMesh
{
	Vec<Vec3> vertCoords;
	Vec<MeshData::Triangle> tris;
	Vec<Vec2> texCoords;
	//Vec<MeshData::VertexWeight> vertWeights;
};


void cutMesh(const Plane& plane, const TempMesh& in, TempMesh& fMesh, TempMesh& bMesh)
{
	for(uint i = 0; i < in.tris.size(); i++)
	{
		const MeshData::Triangle& tri = in.tris[i];
		float testA = plane.test(in.vertCoords[tri.vertIds[0]]);
		float testB = plane.test(in.vertCoords[tri.vertIds[1]]);
		float testC = plane.test(in.vertCoords[tri.vertIds[2]]);

		if(testA < 0.0 && testB < 0.0 && testC < 0.0)
		{

		}
		else if(testA >= 0.0 && testB >= 0.0 && testC >= 0.0)
		{

		}
	}
}


//======================================================================================================================
//                                                                                                                     =
//======================================================================================================================
void shredMesh(const char* filename, float octreeMinSize)
{
	MeshData md(filename);

		//
		// Get min max
		//
		const Vec<Vec3>& vc = md.getVertCoords();
		Vec3 min(vc[0]), max(vc[0]);
		for(uint i = 1; i < vc.size(); i++)
		{
			for(uint j = 0; j < 3; j++)
			{
				if(vc[i][j] < min[j])
				{
					min[j] = vc[i][j];
				}

				if(vc[i][j] > max[j])
				{
					max[j] = vc[i][j];
				}
			}
		}
		std::cout << "min(" << min << "), max(" << max << ")" << std::endl;

		//
		// Calc planes
		//
		Vec<Plane> planes;
		// Planes in x
		for(float dist = floor(min.x / octreeMinSize) * octreeMinSize; dist < max.x; dist += octreeMinSize)
		{
			planes.push_back(Plane(Vec3(1, 0, 0), dist));
		}
		std::cout << "Planes num: " << planes.size() << std::endl;

		//
		//
		//
		//Vec<TempMesh> meshes;

		/*for(Vec<Plane>::iterator plane = planes.begin(); plane != planes.end(); ++plane)
		{
			for(uint i = 0; i < md.getTris().size(); i++)
			{
				const MeshData::Triangle& tri = md.getTris()[i];
				float testA = plane->test(md.getVertCoords()[tri.vertIds[0]]);
				float testB = plane->test(md.getVertCoords()[tri.vertIds[1]]);
				float testC = plane->test(md.getVertCoords()[tri.vertIds[2]]);

				TempMesh* backMesh;

				if(testA < 0.0 && testB < 0.0 && testC < 0.0)
				{

				}
				else if(testA >= 0.0 && testB >= 0.0 && testC >= 0.0)
				{

				}
			}
			float dist0 = plane.getDistance();
			float db;
			float s =
		}*/
}


//======================================================================================================================
// main                                                                                                                =
//======================================================================================================================
int main(int argc, char** argv)
{
	try
	{
		new App(argc, argv);
		app->initWindow();

		Camera* cam = new Camera(app->getMainRenderer().getAspectRatio()*toRad(60.0), toRad(60.0), 0.5, 200.0);
		cam->moveLocalY(3.0);
		cam->moveLocalZ(5.7);
		cam->moveLocalX(-0.3);
		app->setActiveCam(cam);

		Shredder shredder("/home/godlike/src/anki/maps/sponza/walls.mesh", 1.0);

		while(true)
		{
			glClear(GL_COLOR_BUFFER_BIT);

			glColor3fv(&Vec3(1.0)[0]);

			// Draw
			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(3, GL_FLOAT, 0, &(shredder.getOriginalMesh().getVertCoords()[0]));
			glDrawElements(GL_QUADS, shredder.getOriginalMesh().getVertIndeces().size(), GL_UNSIGNED_BYTE, &shredder.getOriginalMesh().getVertIndeces()[0]);
			glDisableClientState(GL_VERTEX_ARRAY);

			app->swapBuffers();
			app->waitForNextFrame();
		}
	}
	catch(std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
