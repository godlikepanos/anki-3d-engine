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
#include "fbos.h"
#include "skybox.h"

camera_t main_cam( r::aspect_ratio*ToRad(60.0), ToRad(60.0), 0.5, 200.0 );

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


vec3_t point( 1.0, 2.0, 0.5 );


skybox_t skybox;


/*
=======================================================================================================================================
main                                                                                                                                  =
=======================================================================================================================================
*/
int main( int argc, char* argv[] )
{
	//mem::Enable( mem::THREADS );
	MathSanityChecks();

	hndl::InitWindow( r::w, r::h, "GODlike's Engine" );
	scn::Init();
	r::Init();
	hud::Init();

	main_cam.MoveLocalY( 2.5 );
	main_cam.MoveLocalZ( 5.0f );

	lvl::Register( (object_t*)&cube );
	lvl::Register( (object_t*)&sphere );
	lvl::Register( (object_t*)&flore );
	lvl::Register( &main_cam );

	const char* skybox_fnames [] = { "env/hellsky4_forward.tga", "env/hellsky4_back.tga", "env/hellsky4_left.tga", "env/hellsky4_right.tga",
																	 "env/hellsky4_up.tga", "env/hellsky4_down.tga" };
	skybox.Load( skybox_fnames );


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

		if( i::keys[SDLK_k] ) main_cam.LookAtPoint( point );

		mover->local_rotation.Reorthogonalize();

		lvl::InterpolateAllModels();
		lvl::UpdateAllWorldTrfs();
		lvl::UpdateAllCameras();

		r::SetProjectionViewMatrices( main_cam );
		r::SetViewport( 0, 0, r::w, r::h );
		glClearColor( 0.1f, 0.1f, 0.2f, 0.0f );
		glClear( GL_DEPTH_BUFFER_BIT );

		skybox.Render( main_cam.view_mat.GetRotationPart() );

		r::NoShaders();

		glDisable( GL_BLEND );
		glEnable( GL_DEPTH_TEST );
		glDisable( GL_TEXTURE_2D );

		lvl::RenderAll();

		r::RenderGrid();

		glPointSize( 10.0 );
		glBegin( GL_POINTS );
			r::Color3( vec3_t(1.0, 0.0, 1.0) );
			glVertex3fv( &point[0] );
		glEnd();


		///                                                                                            /
		vec4_t pos_viewspace = main_cam.view_mat * vec4_t(point, 1.0);
		vec4_t pos_clipspace = main_cam.projection_mat * pos_viewspace;
		vec4_t point_scrspace = pos_clipspace / pos_clipspace.w;


		float depth;
		glReadPixels( i::mouse_pos.x, r::h-i::mouse_pos.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth );


		int viewpoint[] = {0, 0, r::w, r::h};
		vec3_t point_rc; // point recreated
		r::UnProject( i::mouse_pos.x, (float)viewpoint[3]-i::mouse_pos.y, depth, main_cam.view_mat, main_cam.projection_mat, viewpoint, point_rc.x, point_rc.y, point_rc.z );


		/// test for deffered
		// do some crap for pos_eyespace
		vec3_t v[4];
		{
			//int pixels[4][2]={ { 0,0 },{0,r::h},{r::w,r::h},{r::w,0} };
			int pixels[4][2]={ {r::w,r::h}, {0,r::h}, { 0,0 }, {r::w,0} };
			int viewport[4]={ 0,0,r::w,r::h };

			mat3_t view_rotation = main_cam.view_mat.GetRotationPart();

			float d[3];
			for( int i=0; i<4; i++ )
			{
				r::UnProject( pixels[i][0],pixels[i][1],10, main_cam.view_mat, main_cam.projection_mat, viewport, d[0],d[1],d[2] );
				v[i] = vec3_t ( d[0], d[1], d[2] );
				v[i] -= main_cam.world_translation;
				v[i].Normalize();
				v[i] = v[i]*view_rotation;
			}
		}

		// operations that happen in the shader:
		vec2_t planes;
		planes.x = -main_cam.zfar / (main_cam.zfar - main_cam.znear);
		planes.y = -main_cam.zfar * main_cam.znear / (main_cam.zfar - main_cam.znear);
		point_rc.z = - planes.y / (planes.x + depth);

		//vec3_t view(  )

		//pos.xy = view.xy / view.z*pos.z;


		/// print some debug stuff
		hud::SetColor( vec4_t(1.0, 1.0, 1.0, 1.0) );
		hud::SetPos( -0.98, 0.95 );
		hud::SetFontWidth( 0.03 );
		hud::Printf( "frame:%d time:%dms\n", r::frames_num, StopBench() );

		hud::Printf( "mouse: %f %f\n", i::mouse_pos.x, i::mouse_pos.y );
		hud::Printf( "depth: %f\n", depth );
		hud::Printf( "wspac: %f %f %f %f\n", point.x, point.y, point.z, 1.0 );
		hud::Printf( "vspac: %f %f %f %f\n", pos_viewspace.x, pos_viewspace.y, pos_viewspace.z, pos_viewspace.w );
		hud::Printf( "vsp_r: %f %f %f\n", point_rc.x, point_rc.y, point_rc.z );

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
