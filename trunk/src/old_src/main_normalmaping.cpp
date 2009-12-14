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

camera_t main_cam( r::aspect_ratio*ToRad(60.0), ToRad(60.0), 0.1, 100.0 );


mesh_t imp, mcube;

point_light_t point_lights[1];

shader_prog_t* shdr_norm;

/*
=======================================================================================================================================
main                                                                                                                                  =
=======================================================================================================================================
*/
int main( int argc, char* argv[] )
{
	MathSanityChecks();

	hndl::InitWindow( r::w, r::h, "GODlike's Engine" );
	r::Init();
	hud::Init();

	main_cam.MoveLocalY( 2.5 );
	main_cam.MoveLocalZ( 5.0f );
	main_cam.camera_data_user_class_t::SetName("main_cam");

	point_lights[0].SetSpecularColor( vec3_t( 0.4, 0.4, 0.4) );
	point_lights[0].SetDiffuseColor( vec3_t( 1.0, 1.0, 1.0) );
	point_lights[0].local_translation = vec3_t( .6, 0., 0.0 );
	point_lights[0].radius = 4.0;

	imp.Load( "meshes/sarge/sarge.mesh" );
	imp.local_translation = vec3_t( 0.0, 2.11, 0.0 );
	imp.local_scale = 0.7;
	imp.local_rotation.RotateXAxis( -PI/2 );

	mcube.Load( "meshes/cube/cube.mesh" );
	mcube.local_translation = vec3_t( 0, 0, 0 );
	mcube.local_scale = 0.5;

	scene::Register( (object_t*)&imp );
	scene::Register( (object_t*)&mcube );
	scene::Register( &main_cam );
	scene::Register( &point_lights[0] );

	glMaterialfv( GL_FRONT, GL_AMBIENT, &vec4_t( 0.1, 0.1, 0.1, 0.1 )[0] );
	glMaterialfv( GL_FRONT, GL_DIFFUSE, &vec4_t( 0.5, 0.0, 0.0, 0.1 )[0] );
	glMaterialfv( GL_FRONT, GL_SPECULAR, &vec4_t( 1., 1., 1., 0.1 )[0] );
	glMaterialf( GL_FRONT, GL_SHININESS, 100.0 );


	shdr_norm = ass::LoadShdr( "shaders/old/test_normalmaping.shdr" );

	imp.material->shader = shdr_norm;
	mcube.material->shader = shdr_norm;


	/************************************************************************************************************************************
	*																													MAIN LOOP                                                                 *
	*************************************************************************************************************************************/
	do
	{
		StartBench();
		i::HandleEvents();
		r::PrepareNextFrame();

		float dist = 0.1;
		float ang = ToRad(2.0);
		float scale = 0.01;

		// move the camera
		static object_t* mover = &main_cam;

		if( i::keys[ SDLK_1 ] ) mover = &main_cam;
		if( i::keys[ SDLK_2 ] ) mover = &point_lights[0];

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

		if( i::keys[SDLK_k] ) main_cam.LookAtPoint( point_lights[0].world_translation );

		mover->local_rotation.Reorthogonalize();

		scene::InterpolateAllModels();
		scene::UpdateAllWorldStuff();


		glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
		r::SetProjectionViewMatrices( main_cam );
		r::SetViewport( 0, 0, r::w, r::h );

		glDisable( GL_BLEND );
		glEnable( GL_DEPTH_TEST );

		r::PrintLastError();



		for( uint i=0; i<scene::objects.size(); i++ )
		{
			object_t* obj = scene::objects[i];
			if( obj->type == object_t::MESH )
			{
				shdr_norm->Bind();

				glPushMatrix();
				glLoadIdentity();

				obj->material->diffuse_map->BindToTxtrUnit(0);
				glUniform1i( shdr_norm->GetUniLocation("diffuse_map"), 0 );
				obj->material->normal_map->BindToTxtrUnit(1);
				glUniform1i( shdr_norm->GetUniLocation("normal_map"), 1 );

				vec3_t light_pos_eye_space = main_cam.GetViewMatrix() * point_lights[0].world_translation;
				glLightfv( GL_LIGHT0, GL_POSITION, &vec4_t(light_pos_eye_space, 1.0)[0] );

				glLightfv( GL_LIGHT0, GL_DIFFUSE,  &(vec4_t( point_lights[0].GetDiffuseColor(), 1.0 ))[0] );
				glLightfv( GL_LIGHT0, GL_SPECULAR,  &(vec4_t( point_lights[0].GetSpecularColor(), 1.0 ))[0] );

				glPopMatrix();

				obj->Render();

				mesh_t* mesh = static_cast<mesh_t*>( obj );
				r::NoShaders();
				//mesh->RenderNormals();
				//mesh->RenderTangents();
			}
			else
			{
				r::NoShaders();
				obj->Render();
			}
		}


		// print some debug stuff
		hud::SetColor( vec4_t(1.0, 1.0, 1.0, 1.0) );
		hud::SetPos( -0.98, 0.95 );
		hud::SetFontWidth( 0.03 );
		hud::Printf( "frame:%d time:%dms\n", r::frames_num, StopBench() );

		if( i::keys[SDLK_ESCAPE] ) break;
		if( i::keys[SDLK_F11] ) hndl::TogleFullScreen();
		if( i::keys[SDLK_F12] ) r::TakeScreenshot( "gfx/screenshot.tga" );
		SDL_GL_SwapBuffers();
		r::PrintLastError();
		hndl::WaitForNextFrame();
		//if( r::frames_num == 5000 ) break;
	}while( true );

	INFO( "Appl quits after " << hndl::GetTicks() << " ticks" );

	hndl::QuitApp( EXIT_SUCCESS );
	return 0;
}
