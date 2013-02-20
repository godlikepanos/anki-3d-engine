#include <iostream>
#include <GL/glew.h>
#include "MeshData.h"
#include "Collision.h"
#include "App.h"
#include "Shredder.h"
#include "Camera.h"
#include "Input.h"
#include "MainRenderer.h"
#include "RendererInitializer.h"
#include "SceneGraph.h"
#include "anki/gl/GlException.h"


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


//==============================================================================
//                                                                             =
//==============================================================================
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


//==============================================================================
// initEngine                                                                  =
//==============================================================================
void initEngine(int argc, char** argv)
{
	new App(argc, argv);
	app->initWindow();

	glClearColor(0.1, 0.1, 0.1, 1.0);
	glClearDepth(1.0);
	glClearStencil(0);
	glDepthFunc(GL_LEQUAL);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	GlStateMachineSingleton::get().enable(GL_BLEND, lse)eton::get().setBlendingEnabled(false);
	glDisable(GL_STENCIL_TEST);
	glPolygonMode(GL_FRONT, GL_FILL);
	glDepthMask(true);
	glDepthFunc(GL_LESS);
	glViewport(0, 0, app->getWindowWidth(), app->getWindowHeight());

	Camera* cam = new Camera(float(app->getWindowWidth()) / app->getWindowHeight() * toRad(60.0), toRad(60.0), 0.5, 200.0);
	cam->moveLocalY(3.0);
	cam->moveLocalZ(5.7);
	cam->moveLocalX(-0.3);
	app->setActiveCam(cam);
}


//==============================================================================
// main                                                                        =
//==============================================================================
int main(int argc, char** argv)
{
	try
	{
		initEngine(argc, argv);
		Shredder shredder("/home/godlike/src/anki/maps/sponza/walls.mesh", 1.0);

		while(true)
		{
			// Input
			app->getInput().handleEvents();
			float dist = 0.2;
			float ang = toRad(3.0);
			SceneNode* mover = app->getActiveCam();
			if(app->getInput().keys[SDL_SCANCODE_A])
			{
				mover->moveLocalX(-dist);
			}
			if(app->getInput().keys[SDL_SCANCODE_D])
			{
				mover->moveLocalX(dist);
			}
			if(app->getInput().keys[SDL_SCANCODE_LSHIFT])
			{
				mover->moveLocalY(dist);
			}
			if(app->getInput().keys[SDL_SCANCODE_SPACE])
			{
				mover->moveLocalY(-dist);
			}
			if(app->getInput().keys[SDL_SCANCODE_W])
			{
				mover->moveLocalZ(-dist);
			}
			if(app->getInput().keys[SDL_SCANCODE_S])
			{
				mover->moveLocalZ(dist);
			}
			if(app->getInput().keys[SDL_SCANCODE_Q])
			{
				mover->rotateLocalZ(ang);
			}
			if(app->getInput().keys[SDL_SCANCODE_E])
			{
				mover->rotateLocalZ(-ang);
			}
			if(app->getInput().keys[SDL_SCANCODE_UP])
			{
				mover->rotateLocalX(ang);
			}
			if(app->getInput().keys[SDL_SCANCODE_DOWN])
			{
				mover->rotateLocalX(-ang);
			}
			if(app->getInput().keys[SDL_SCANCODE_LEFT])
			{
				mover->rotateLocalY(ang);
			}
			if(app->getInput().keys[SDL_SCANCODE_RIGHT])
			{
				mover->rotateLocalY(-ang);
			}

			mover->getLocalTransform().rotation.reorthogonalize();

			// Update
			app->getSceneGraph().updateAllControllers();
			app->getSceneGraph().updateAllWorldStuff();

			// Render
			glClear(GL_COLOR_BUFFER_BIT);
			glColor3fv(&Vec3(1.0)[0]);
			//glPolygonMode(GL_FRONT, GL_LINE);

			// Draw
			glMatrixMode(GL_PROJECTION);
			Mat4 projMat = app->getActiveCam()->getProjectionMatrix().getTransposed();
			glLoadMatrixf(&projMat[0]);
			glMatrixMode(GL_MODELVIEW);
			Mat4 viewMat = app->getActiveCam()->getViewMatrix().getTransposed();
			glLoadMatrixf(&viewMat[0]);

			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(3, GL_FLOAT, 0, &(shredder.getOriginalMesh().getVertCoords()[0]));
			glNormalPointer(GL_FLOAT, 0, &(shredder.getOriginalMesh().getVertNormals()[0]));
			glDrawElements(GL_QUADS, shredder.getOriginalMesh().getVertIndeces().size(), GL_UNSIGNED_SHORT,
			               &shredder.getOriginalMesh().getVertIndeces()[0]);
			glDisableClientState(GL_VERTEX_ARRAY);

			app->swapBuffers();
			app->waitForNextFrame();
			ON_GL_FAIL_THROW_EXCEPTION();
		}
	}
	catch(std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
