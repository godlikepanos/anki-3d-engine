#include "skybox.h"
#include "Renderer.h"
#include "Math.h"
#include "Camera.h"
#include "Scene.h"
#include "App.h"
#include "MainRenderer.h"


/*static float coords [][4][3] =
{
	// front
	{ { 1,  1, -1}, {-1,  1, -1}, {-1, -1, -1}, { 1, -1, -1} },
	// back
	{ {-1,  1,  1}, { 1,  1,  1}, { 1, -1,  1}, {-1, -1,  1} },
	// left
	{ {-1,  1, -1}, {-1,  1,  1}, {-1, -1,  1}, {-1, -1, -1} },
	// right
	{ { 1,  1,  1}, { 1,  1, -1}, { 1, -1, -1}, { 1, -1,  1} },
	// up
	{ { 1,  1,  1}, {-1,  1,  1}, {-1,  1, -1}, { 1,  1, -1} },
	//
	{ { 1, -1, -1}, {-1, -1, -1}, {-1, -1,  1}, { 1, -1,  1} }
};*/



/*
=======================================================================================================================================
load                                                                                                                   =
=======================================================================================================================================
*/
bool Skybox::load(const char* filenames[6])
{
	for(int i=0; i<6; i++)
	{
		textures[i].loadRsrc(filenames[i]);
	}

	noise.loadRsrc("gfx/noise2.tga");
	noise->setRepeat(true);

	shader.loadRsrc("shaders/ms_mp_skybox.glsl");

	return true;
}


/*
=======================================================================================================================================
render                                                                                                                 =
=======================================================================================================================================
*/
void Skybox::Render(const Mat3& /*rotation*/)
{
	/*glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glPushMatrix();

	shader->bind();
	glUniform1i(shader->findUniVar("colormap")->getLoc(), 0);
	shader->findUniVar("noisemap")->setTexture(*noise, 1);
	glUniform1f(shader->findUniVar("timer")->getLoc(), (rotation_ang/(2*PI))*100);
	glUniform3fv(shader->findUniVar("sceneAmbientCol")->getLoc(), 1, &(Vec3(1.0, 1.0, 1.0) / app->getScene().getAmbientCol())[0]);

	// set the rotation matrix
	Mat3 tmp(rotation);
	tmp.rotateYAxis(rotation_ang);
	app->getMainRenderer().loadMatrix(Mat4(tmp));

	rotation_ang += 0.0001;
	if(rotation_ang >= 2*PI) rotation_ang = 0.0;



	const float ffac = 0.001; // fault factor. To eliminate the artefacts in the edge of the quads caused by texture filtering
	float uvs [][2] = { {1.0-ffac, 1.0-ffac}, {0.0+ffac, 1.0-ffac}, {0.0+ffac, 0.0+ffac}, {1.0-ffac, 0.0+ffac} };

	for(int i=0; i<6; i++)
	{
		textures[i]->bind(0);
		glBegin(GL_QUADS);
			glTexCoord2fv(uvs[0]);
			glVertex3fv(coords[i][0]);
			glTexCoord2fv(uvs[1]);
			glVertex3fv(coords[i][1]);
			glTexCoord2fv(uvs[2]);
			glVertex3fv(coords[i][2]);
			glTexCoord2fv(uvs[3]);
			glVertex3fv(coords[i][3]);
		glEnd();
	}

	glPopMatrix();*/
}
