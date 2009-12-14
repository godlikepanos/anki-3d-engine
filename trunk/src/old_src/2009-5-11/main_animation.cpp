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
#include "level.h"

const float fovy = ToRad(60.0);
camera_t main_cam( r::aspect_ratio*fovy, fovy, 0.5, 100.0 );
camera_t other_cam( ToRad(60.0), ToRad(60.0), 1.0, 10.0 );


light_t lights[1];

model_data_t mdata;
model_t model;
skeleton_anim_t anim;
texture_t txtr, txtr1;

float frame_asd = 0.0;

texture_t render_target;

class cube_t: public object_t
{
	public:
		void Render()
		{
			glPushMatrix();
			r::MultMatrix( /*model.world_transformation */ world_transformation );

			r::SetGLState_Solid();
			glEnable( GL_TEXTURE_2D );

			glActiveTexture( GL_TEXTURE0 );
			glBindTexture( GL_TEXTURE_2D, render_target.gl_id );

			//glEnable( GL_BLEND );

			r::Color4( vec4_t(1.0, 1.0, 1.0, 1) );
			r::RenderCube();

			glPopMatrix();
		}
}cube;


/*
=======================================================================================================================================
main                                                                                                                                  =
=======================================================================================================================================
*/
int main( int argc, char* argv[] )
{
	//mem::Enable( mem::THREADS );
	MathSanityChecks();
	hndl::InitWindow( r::w, r::h, "mAlice Engine" );
	r::Init();
	hud::Init();

	main_cam.MoveLocalY( 2.5 );
	main_cam.MoveLocalZ( 5.0f );

	other_cam.MoveLocalY( 2.5 );
	other_cam.MoveLocalZ( 5.0f );
	other_cam.RotateLocalY( ToRad(-30.0) );
	other_cam.MoveLocalX( -3.0f );

	ass::LoadMat( "materials/imp/imp.mat" );

	// initialize the TESTS
	lights[0].SetAmbient( vec4_t( 0.4, 0.4, 0.4, 1.0 ) );
	lights[0].SetDiffuse( vec4_t( 1.5, 1.5, 1.0, 1.0 ) );
	lights[0].local_translation = vec3_t( -1.0, 1.0, 0.0 );
	lights[0].type = light_t::DIRECTIONAL;
	//lights[0].SetCutOff( 0.0 );
	//lights[0].SetExp( 120.0 );


	txtr.Load( "models/imp/imp_d.tga" );
	txtr1.Load( "models/imp/imp_local.tga" );
	mdata.Load( "models/test/imp.mdl" );
	model.Init( &mdata );
//	model.local_translation = vec3_t( 1.0, 0.0, 0.0 );
	model.local_rotation.Set( euler_t( -PI/2, 0.0, 0.0) );
	//model.SetChilds( 1, &cube );
	//model.GetBone("arm.L.001")->AddChilds( 1, &cube );
	anim.Load( "models/test/walk.imp.anim" );
	model.Play( &anim, 0, 0.7, model_t::START_IMMEDIATELY );


	render_target.Load( "gfx/no_tex.tga" );


	//cube.local_translation = vec3_t( 2.2, 0.05, 1.75 );
	cube.local_translation = vec3_t( 3.2, 0.05, 1.75 );
	cube.local_scale = 2.0;
	//cube.local_rotation.Set( euler_t( ToRad(-10.0), 0.0, ToRad(-25.0)) );
	//cube.local_scale = 0.5;
	//cube.parent = model.GetBone("arm.L.001");


	lvl::Register( (model_t*)&model );
	lvl::Register( (object_t*)&cube );
	lvl::Register( &main_cam );
	lvl::Register( &other_cam );
	lvl::Register( (light_t*)&lights[0] );

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
		if( i::keys[ SDLK_2 ] ) mover = &model;
		if( i::keys[ SDLK_3 ] ) mover = &lights[0];
		if( i::keys[ SDLK_4 ] ) mover = &other_cam;

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

		mover->local_rotation.Reorthogonalize();

		lvl::InterpolateAllModels();
		lvl::UpdateAllWorldTrfs();
		lvl::UpdateAllLights();
		lvl::UpdateAllFrustumPlanes();



		// render to txtr
			glClearColor( 0.0, 0.0, 0.1, 1.0 );
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			const int width = 128;
			float a = 1;
			r::SetViewport( 0, 0, width, width/a );
			r::SetProjectionMatrix( other_cam );
			r::SetViewMatrix( other_cam );

			lvl::RenderAll();

			glActiveTexture( GL_TEXTURE0 );
			glBindTexture( GL_TEXTURE_2D, render_target.gl_id );
			glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 0, 0, width, width/a, 0 );
		// end render to txtr


		glClearColor( 0.1f, 0.1f, 0.1f, 0.0f );
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		r::SetViewport( 0, 0, r::w, r::h );
		r::SetProjectionMatrix( main_cam );
		r::SetViewMatrix( main_cam );

		r::RenderGrid();

		// render rest
		model.Deform();
		lvl::RenderAll();


		r::SetGLState_Solid();
		material_t* mat = ass::LoadMat( "materials/grass0/grass0.mat" );
		mat->Use();
		float size = 0.5f;
		glBegin(GL_QUADS);
			glNormal3f( 0.0f, 0.0f, 1.0f);
			glTexCoord2f(0.0, 0.0); glVertex3f(-size, -size,  size);
			glTexCoord2f(1.0, 0.0); glVertex3f( size, -size,  size);
			glTexCoord2f(1.0, 1.0); glVertex3f( size,  size,  size);
			glTexCoord2f(0.0, 1.0); glVertex3f(-size,  size,  size);
		glEnd();



		// print some debug stuff
		hud::SetColor( &vec3_t( 1.0, 1.0, 1.0 )[0] );
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

	cout << "Appl quits after " << hndl::GetTicks() << " ticks" << endl;

	hndl::QuitApp( EXIT_SUCCESS );
	return 0;
}
