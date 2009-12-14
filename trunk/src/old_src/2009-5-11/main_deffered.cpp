#include <stdio.h>
#include <iostream>
#include <fstream>
#include "common.h"

#include "input.h"
#include "camera.h"
#include "math.h"
#include "renderer.h"
#include "hud.h"
#include "handlers.h"
#include "particles.h"
#include "primitives.h"
#include "texture.h"
#include "geometry.h"
#include "shaders.h"
#include "lights.h"
#include "collision.h"
#include "model.h"
#include "spatial.h"
#include "material.h"
#include "assets.h"
#include "scene.h"
#include "scanner.h"
#include "skybox.h"


light_t lights[1];

camera_t main_cam( r::aspect_ratio*ToRad(60.0), ToRad(60.0), 0.5, 100.0 );

class cube_t: public object_t
{
	public:
		cube_t()
		{
			local_translation = vec3_t( 3.2, 0.05, 1.75 );
			local_scale = 2.0;
		}

		void Render()
		{
			glPushMatrix();
			r::MultMatrix( world_transformation );

			r::Color4( vec4_t(1.0, 1.0, 0.0, 1) );

			r::RenderCube( true );

			glPopMatrix();
		}
}cube;


class shpere_t: public object_t
{
	public:
		shpere_t()
		{
			local_translation = vec3_t( -1.2, 1.05, -1.75 );
			local_scale = 1.5;
		}

		void Render()
		{
			glPushMatrix();
			r::MultMatrix( world_transformation );

			r::Color4( vec4_t(1.0, 0.5, 0.25, 1) );

			r::RenderSphere( 1.0, 32 );

			glPopMatrix();
		}
}sphere;


class flore_t: public object_t
{
	public:
		flore_t()
		{
			local_scale = 10.0;
		}

		void Render()
		{
			glPushMatrix();
			r::MultMatrix( world_transformation );

			r::Color4( vec4_t(.6, .6, 1., 1) );

			glNormal3fv( &vec3_t( 0.0, 1.0, 0.0 )[0] );
			glBegin( GL_QUADS );
				glVertex3f( 1.0, 0.0, -1.0 );
				glVertex3f( -1.0, 0.0, -1.0 );
				glVertex3f( -1.0, 0.0, 1.0 );
				glVertex3f( 1.0, 0.0, 1.0 );
			glEnd();

			glPopMatrix();
		}
}flore;

mesh_t imp;


/*
=======================================================================================================================================
main                                                                                                                                  =
=======================================================================================================================================
*/
int main( int argc, char* argv[] )
{
	MathSanityChecks();

	hndl::InitWindow( r::w, r::h, "GODlike's Engine" );
	scn::Init();
	r::Init();
	hud::Init();

	main_cam.MoveLocalY( 2.5 );
	main_cam.MoveLocalZ( 5.0f );

	lights[0].SetAmbient( vec4_t( 0.4, 0.9, 0.4, 1.0 ) );
	lights[0].SetDiffuse( vec4_t( 1.0, 1.0, 1.0, 1.0 ) );
	lights[0].local_translation = vec3_t( -1.0, 0.3, 1.0 );

	imp.Init( ass::LoadMeshD( "models/test/imp.mesh" ) );
	imp.local_rotation.RotateXAxis( -PI/2 );

	scene::Register( (object_t*)&cube );
	scene::Register( (object_t*)&sphere );
	scene::Register( (object_t*)&flore );
	scene::Register( (object_t*)&imp );
	scene::Register( &main_cam );
	scene::Register( &lights[0] );

	glMaterialfv( GL_FRONT, GL_AMBIENT, &vec4_t( 0.1, 0.1, 0.1, 0.1 )[0] );
	glMaterialfv( GL_FRONT, GL_DIFFUSE, &vec4_t( 0.5, 0.0, 0.0, 0.1 )[0] );
	glMaterialfv( GL_FRONT, GL_SPECULAR, &vec4_t( 1., 1., 1., 0.1 )[0] );
	glMaterialf( GL_FRONT, GL_SHININESS, 100.0 );

	const char* skybox_fnames [] = { "env/hellsky4_forward.tga", "env/hellsky4_back.tga", "env/hellsky4_left.tga", "env/hellsky4_right.tga",
																	 "env/hellsky4_up.tga", "env/hellsky4_down.tga" };
	scene::skybox.Load( skybox_fnames );


	/************************************************************************************************************************************
	*																													MAIN LOOP                                                                 *
	*************************************************************************************************************************************/
	do
	{
		StartBench();
		i::HandleEvents();
		r::PrepareNextFrame();

		float dist = 0.2;
		float ang = ToRad(3.0);
		float scale = 0.01;

		// move the camera
		static object_t* mover = &main_cam;

		if( i::keys[ SDLK_1 ] ) mover = &main_cam;
		if( i::keys[ SDLK_3 ] ) mover = &lights[0];

		if( i::keys[SDLK_a] ) mover->MoveLocalX( -dist );
		if( i::keys[SDLK_d] ) mover->MoveLocalX( dist );
		if( i::keys[SDLK_LSHIFT] ) mover->MoveLocalY( dist );
		if( i::keys[SDLK_SPACE] ) mover->MoveLocalY( -dist );
		if( i::keys[SDLK_w] ) mover->MoveLocalZ( -dist );
		if( i::keys[SDLK_s] ) mover->MoveLocalZ( dist );
		if( i::keys[SDLK_UP] ) mover->RotateLocalX( ang );
		if( i::keys[SDLK_DOWN] ) mover->RotateLocalX( -ang );
		if( i::keys[SDLK_LEFT] ) mover->RotateLocalY( ang );
		if( i::keys[SDLK_RIGHT] ) mover->RotateLocalY( -ang );
		if( i::keys[SDLK_q] ) mover->RotateLocalZ( ang );
		if( i::keys[SDLK_e] ) mover->RotateLocalZ( -ang );
		if( i::keys[SDLK_PAGEUP] ) mover->local_scale += scale ;
		if( i::keys[SDLK_PAGEDOWN] ) mover->local_scale -= scale ;

		if( i::keys[SDLK_k] ) main_cam.LookAtPoint( lights[0].world_translation );


		mover->local_rotation.Reorthogonalize();

		scene::InterpolateAllModels();
		//scene::UpdateAllLights();
		scene::UpdateAllWorldTrfs();
		scene::UpdateAllCameras();

		///   RENDER TO FBO
		r::dfr::MaterialPass( main_cam );


		///  POST PROCESSING
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
		glOrtho( 0.0, r::w, 0.0, r::h, -1.0, 1.0 );

		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();

		glClearColor( 0.1f, 0.1f, 0.1f, 0.0f );
		//glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); no need for that

		r::dfr::AmbientPass( main_cam, vec3_t(0.3, 0.3, 0.3) );
		//r::d::AmbientPass( vec3_t( 1.0, 1.0, 1.0 ) );
		r::dfr::LightingPass( main_cam );


		// print some debug stuff
		hud::SetColor( vec4_t(1.0, 1.0, 1.0, 1.0) );
		hud::SetPos( -0.98, 0.95 );
		hud::SetFontWidth( 0.03 );
		hud::Printf( "frame:%d time:%dms\n", r::frames_num, StopBench() );

		if( i::keys[SDLK_ESCAPE] ) break;
		if( i::keys[SDLK_F11] ) hndl::TogleFullScreen();
		SDL_GL_SwapBuffers();
		r::PrintLastError();
		hndl::WaitForNextFrame();
		//if( r::frames_num == 5000 ) break;
	}while( true );

	INFO( "Appl quits after " << hndl::GetTicks() << " ticks" );

	hndl::QuitApp( EXIT_SUCCESS );
	return 0;
}
