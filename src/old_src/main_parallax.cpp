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
#include "mesh.h"
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

camera_t main_cam( r::aspect_ratio*ToRad(60.0), ToRad(60.0), 0.5, 100.0 );

point_light_t point_lights[10];
proj_light_t projlights[2];

mesh_t floor__;

shader_prog_t* shdr_parallax;

texture_t* diffuse_map, * normal_map, * height_map;

/*
=======================================================================================================================================
main                                                                                                                                  =
=======================================================================================================================================
*/
int main( int argc, char* argv[] )
{
	MathSanityChecks();

	hndl::InitWindow( r::w, r::h, "AnKi Engine" );
	r::Init();
	hud::Init();


	main_cam.MoveLocalZ( 2 );
	main_cam.camera_data_user_class_t::SetName("main_cam");

	point_lights[0].SetSpecularColor( vec3_t( 0.4, 0.4, 0.4) );
	point_lights[0].SetDiffuseColor( vec3_t( 1.0, .0, .0)*10 );
	//point_lights[0].translation_lspace = vec3_t( -1.0, 0.3, 1.0 );
	point_lights[0].radius = 4.0;


	floor__.Load( "meshes/test3/Plane.mesh" );
	diffuse_map = ass::LoadTxtr( "textures/stone.001.diff.tga" );
	normal_map = ass::LoadTxtr( "textures/stone.001.norm.tga" );
	height_map = ass::LoadTxtr( "textures/stone.001.height.tga" );

	shdr_parallax = ass::LoadShdr( "shaders/old/parallax.glsl" );


	scene::Register( &floor__ );
	scene::Register( &point_lights[0] );
	scene::Register( &main_cam );

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
		if( i::keys[ SDLK_2 ] ) mover = &point_lights[0];
		if( i::keys[ SDLK_3 ] ) mover = &floor__;

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
		if( i::keys[SDLK_PAGEUP] ) mover->scale_lspace += scale ;
		if( i::keys[SDLK_PAGEDOWN] ) mover->scale_lspace -= scale ;

		if( i::keys[SDLK_k] ) main_cam.LookAtPoint( point_lights[0].translation_wspace );

		mover->rotation_lspace.Reorthogonalize();

		scene::InterpolateAllModels();
		scene::UpdateAllWorldStuff();


		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glEnable( GL_DEPTH_TEST );
		r::SetViewport( 0, 0, r::w, r::h );
		r::SetProjectionViewMatrices( main_cam );
		r::NoShaders();

		//r::RenderGrid();

		shdr_parallax->Bind();

		shdr_parallax->LocTexUnit( "texture", *(diffuse_map), 0 );
		shdr_parallax->LocTexUnit( "heightMap", *(height_map), 1 );
		shdr_parallax->LocTexUnit( "normalMap", *(normal_map), 2 );

		glUniform3fv( shdr_parallax->GetUniLocation("lightPos"), 1, &(main_cam.GetViewMatrix() * point_lights[0].translation_wspace)[0] );
		//glUniformMatrix4fv( shdr_parallax->GetUniLocation("model"), 1, true, &floor__.transformation_wspace[0] );
		//glUniform3fv( shdr_parallax->GetUniLocation("eyePosition"), 1, &(main_cam.translation_wspace)[0] );


		//floor__.Render();
		{
			glPushMatrix();
			//floor__.transformation_wspace.Set( m3 );
			r::MultMatrix( floor__.transformation_wspace );

			//GLuint loc = shdr_parallax->GetAttrLocation( "tangent" );

			glEnableClientState( GL_VERTEX_ARRAY );
			glEnableClientState( GL_NORMAL_ARRAY );
			glEnableClientState( GL_TEXTURE_COORD_ARRAY );
			//glEnableVertexAttribArray( loc );

			glVertexPointer( 3, GL_FLOAT, sizeof(vertex_t), &floor__.mesh_data->verts[0].coords[0] );
			glNormalPointer( GL_FLOAT, sizeof(vertex_t), &floor__.mesh_data->verts[0].normal[0] );
			glTexCoordPointer( 2, GL_FLOAT, 0, &floor__.mesh_data->uvs[0] );
			//glVertexAttribPointer( loc, 4, GL_FLOAT, 0, 0, &floor__.mesh_data->vert_tangents[0] );

			glDrawElements( GL_TRIANGLES, floor__.mesh_data->vert_list.size(), GL_UNSIGNED_SHORT, &floor__.mesh_data->vert_list[0] );

			glDisableClientState( GL_VERTEX_ARRAY );
			glDisableClientState( GL_NORMAL_ARRAY );
			glDisableClientState( GL_TEXTURE_COORD_ARRAY );
			//glDisableVertexAttribArray( loc );

			glPopMatrix();
		}



		r::dbg::RunStage( main_cam );


		// print some debug stuff
		hud::SetColor( vec4_t(1.0, 1.0, 1.0, 1.0) );
		hud::SetPos( -0.98, 0.95 );
		hud::SetFontWidth( 0.03 );
		hud::Printf( "frame:%d time:%dms\n", r::frames_num, StopBench() );

		if( i::keys[SDLK_ESCAPE] ) break;
		if( i::keys[SDLK_F11] ) hndl::TogleFullScreen();
		if( i::keys[SDLK_F12] == 1 ) r::TakeScreenshot("gfx/screenshot.tga");
//		char str[128];
//		sprintf( str, "capt/%05d.tga", r::frames_num );
//		r::TakeScreenshot(str);
		SDL_GL_SwapBuffers();
		r::PrintLastError();
		hndl::WaitForNextFrame();
		//if( r::frames_num == 5000 ) break;
	}while( true );

	INFO( "Appl quits after " << hndl::GetTicks() << " ticks" );

	hndl::QuitApp( EXIT_SUCCESS );
	return 0;
}
