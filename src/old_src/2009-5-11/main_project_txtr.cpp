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
#include "scanner.h"

light_t lights[1];

camera_t main_cam( r::aspect_ratio*ToRad(60.0), ToRad(60.0), 0.5, 100.0 );
camera_t light_cam( ToRad(90.0), ToRad(90.0), 0.10, 10.0 );

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

			r::Color4( vec4_t(1.0, 1., 1., 1) );

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


texture_t* proj;

shader_prog_t* shdr, *shdr1;

#define SHADOW_QUALITY 512


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
	scn::Init();
	r::Init();
	hud::Init();

	main_cam.MoveLocalY( 2.5 );
	main_cam.MoveLocalZ( 5.0f );

	/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// initialize the TESTS
	lights[0].SetAmbient( vec4_t( 0.4, 0.9, 0.4, 1.0 ) );
	lights[0].SetDiffuse( vec4_t( 1.0, 1.0, 1.0, 1.0 ) );
	lights[0].local_translation = vec3_t( -1.0, 1.0, 1.0 );
	lights[0].type = light_t::DIRECTIONAL;
	//lights[0].SetCutOff( 0.0 );
	//lights[0].SetExp( 120.0 );

	proj = ass::LoadTxtr( "gfx/flashlight.tga" );

	lights[0].MakeParent( &light_cam );
	//light_cam.RotateLocalY( -PI/2 );

	shdr = ass::LoadShdr( "shaders/test.shdr" );
	//shdr1 = ass::LoadShdr( "shaders/glass.shader" );

	glMaterialfv( GL_FRONT, GL_AMBIENT, &vec4_t( 0.1, 0.1, 0.1, 0.1 )[0] );
	glMaterialfv( GL_FRONT, GL_DIFFUSE, &vec4_t( 0.5, 0.0, 0.0, 0.1 )[0] );
	glMaterialfv( GL_FRONT, GL_SPECULAR, &vec4_t( 1., 1., 1., 0.1 )[0] );
	glMaterialf( GL_FRONT, GL_SHININESS, 120.0 );

	/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	lvl::Register( (object_t*)&cube );
	lvl::Register( (object_t*)&sphere );
	lvl::Register( (object_t*)&flore );
	lvl::Register( &lights[0] );
	lvl::Register( &main_cam );
	lvl::Register( &light_cam );

	light_cam.MakeParent( &main_cam );

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

		mover->local_rotation.Reorthogonalize();

		lvl::InterpolateAllModels();
		//lvl::UpdateAllLights();
		lvl::UpdateAllWorldTrfs();
		lvl::UpdateAllCameras();

		// RENDER
		glClearColor( 0.1f, 0.1f, 0.1f, 0.0f );
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		r::SetProjectionViewMatrices( main_cam );
		r::SetViewport( 0, 0, r::w, r::h );
		lvl::UpdateAllLights();

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDisable(GL_POLYGON_OFFSET_FILL);

		//r::RenderGrid();
		r::SetGLState_Solid();
		glEnable( GL_LIGHTING );

		shdr->Bind();

		//glUniform1i( shdr->GetUniLocation("t0"), 0 );

		glActiveTexture( GL_TEXTURE0 + 0 );
		proj->Bind();

		const float mBias[] = {0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0};
		glMatrixMode( GL_TEXTURE );
		glLoadMatrixf( mBias );
		r::MultMatrix( light_cam.projection_mat );
		r::MultMatrix( light_cam.view_mat );
		r::MultMatrix( main_cam.world_transformation );
		glMatrixMode(GL_MODELVIEW);

		lvl::RenderAll();

		shdr1->UnBind();
		glMatrixMode( GL_TEXTURE );
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);

		// Debug stuff
//		r::SetGLState_Solid();
//		glEnable( GL_TEXTURE_2D );
//		glDisable( GL_LIGHTING );
//		depth_map.Bind();
//		float size = 2.0f;
//		r::Color3( vec3_t( 1.0, 1.0, 1.0 ) );
//		glBegin(GL_QUADS);
//			glNormal3f( 0.0f, 0.0f, 1.0f);
//			glTexCoord2f(0.0, 0.0); glVertex3f(-size, -size,  -5);
//			glTexCoord2f(1.0, 0.0); glVertex3f( size, -size,  -5);
//			glTexCoord2f(1.0, 1.0); glVertex3f( size,  size,  -5);
//			glTexCoord2f(0.0, 1.0); glVertex3f(-size,  size,  -5);
//		glEnd();
		// END RENDER


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
