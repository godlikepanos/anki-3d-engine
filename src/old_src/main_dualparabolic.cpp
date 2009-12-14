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

class cube_t: public mesh_t
{
	public:
		cube_t()
		{
			translation_lspace = vec3_t( 3.2, 0.05, 1.75 );
			scale_lspace = 2.0;
			object_t::SetName( "cube" );
		}

		void Render()
		{
			glPushMatrix();
			r::MultMatrix( transformation_wspace );

			r::Color4( vec4_t(1.0, 1.0, 0.0, 1) );

			r::RenderCube( true );

			glPopMatrix();
		}

		void RenderDepth()
		{
			glPushMatrix();
			r::MultMatrix( transformation_wspace );

			r::RenderCube();

			glPopMatrix();
		}

		void UpdateWorldStuff()
		{
			UpdateWorldTransform();
		};
}cube;


class shpere_t: public mesh_t
{
	public:
		shpere_t()
		{
			translation_lspace = vec3_t( -1.2, 1.05, -1.75 );
			scale_lspace = 1.5;
			object_t::SetName( "sphere" );
		}

		void Render()
		{
			glPushMatrix();
			r::MultMatrix( transformation_wspace );

			r::Color4( vec4_t(1.0, 0.5, 0.25, 1) );

			r::RenderSphere( 1.0, 32 );

			glPopMatrix();
		}

		void RenderDepth()
		{
			glPushMatrix();
			r::MultMatrix( transformation_wspace );
			r::RenderSphere( 1.0, 32 );
			glPopMatrix();
		}

		void UpdateWorldStuff()
		{
			UpdateWorldTransform();
		};
}sphere;


class flore_t: public mesh_t
{
	public:

		flore_t()
		{
			scale_lspace = 10.0;
			object_t::SetName( "flore" );
		}

		void Render()
		{
			glPushMatrix();
			r::MultMatrix( transformation_wspace );

			r::Color4( vec4_t(.6, .6, 1., 1) );
			//r::Color4( vec4_t::one * 0.5 );

			GLuint loc = material->shader->GetAttrLocation( "tangent" );
			vec4_t tangent = vec4_t(1,0,0,0);

			float f = 8.0;
			glNormal3fv( &vec3_t( 0.0, 1.0, 0.0 )[0] );
			glBegin( GL_QUADS );
				glTexCoord2f( f, f );
				glVertexAttrib4fv( loc, &tangent[0] );
				glVertex3f( 1.0, 0.0, -1.0 );
				glTexCoord2f( f, 0 );
				glVertexAttrib4fv( loc, &tangent[0] );
				glVertex3f( -1.0, 0.0, -1.0 );
				glTexCoord2f( 0, 0 );
				glVertexAttrib4fv( loc, &tangent[0] );
				glVertex3f( -1.0, 0.0, 1.0 );
				glTexCoord2f( 0, f );
				glVertexAttrib4fv( loc, &tangent[0] );
				glVertex3f( 1.0, 0.0, 1.0 );
			glEnd();

//			glNormal3fv( &(-vec3_t( 0.0, 1.0, 0.0 ))[0] );
//			glBegin( GL_QUADS );
//				glVertex3f( 1.0, 0.0, 1.0 );
//				glVertex3f( -1.0, 0.0, 1.0 );
//				glVertex3f( -1.0, 0.0, -1.0 );
//				glVertex3f( 1.0, 0.0, -1.0 );
//			glEnd();

			glPopMatrix();
		}

		void RenderDepth()
		{
			glPushMatrix();
			r::MultMatrix( transformation_wspace );
			glBegin( GL_QUADS );
				glVertex3f( 1.0, 0.0, -1.0 );
				glVertex3f( -1.0, 0.0, -1.0 );
				glVertex3f( -1.0, 0.0, 1.0 );
				glVertex3f( 1.0, 0.0, 1.0 );
			glEnd();
			glPopMatrix();
		}

		void UpdateWorldStuff()
		{
			UpdateWorldTransform();
		};
}flore;

mesh_t imp, mcube;

point_light_t point_lights[10];
proj_light_t projlights[2];

shader_prog_t shdr_parab;

texture_t parab_tex;
#define PARABOLOID_SIZE 512

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

	main_cam.MoveLocalY( .5 );
	main_cam.MoveLocalZ( 10.5f );
	main_cam.camera_data_user_class_t::SetName("main_cam");

	point_lights[0].SetSpecularColor( vec3_t( 0.4, 0.4, 0.4) );
	point_lights[0].SetDiffuseColor( vec3_t( 1.0, 1.0, 1.0) );
	point_lights[0].translation_lspace = vec3_t( -1.0, 0.3, 1.0 );
	point_lights[0].radius = 4.0;
	point_lights[1].SetSpecularColor( vec3_t( 0.0, 0.0, 1.0) );
	point_lights[1].SetDiffuseColor( vec3_t( 3.0, 0.1, 0.1) );
	point_lights[1].translation_lspace = vec3_t( 2.5, 1.4, 1.0 );
	point_lights[1].radius = 3.0;
	projlights[0].camera.SetAll( ToRad(60), ToRad(60), 0.1, 20.0 );
	projlights[0].texture = ass::LoadTxtr( "gfx/lights/flashlight.tga" );
	projlights[0].SetSpecularColor( vec3_t( 1.0, 1.0, 1.0) );
	projlights[0].SetDiffuseColor( vec3_t( 3.0, 3.0, 4.0)/1.0 );
	projlights[0].translation_lspace = vec3_t( 1.3, 1.3, 3.0 );
	projlights[0].rotation_lspace.RotateYAxis(ToRad(20));
	projlights[0].casts_shadow = true;
	projlights[1].camera.SetAll( ToRad(60), ToRad(60), 0.1, 20.0 );
	projlights[1].texture = ass::LoadTxtr( "gfx/lights/impflash.tga" );
	projlights[1].SetSpecularColor( vec3_t( 1.0, 1.0, 0.0) );
	projlights[1].SetDiffuseColor( vec3_t( 1.0, 1.0, 1.0) );
//	projlights[1].translation_lspace = vec3_t( -1.3, 1.3, 3.0 );
	projlights[1].rotation_lspace.RotateYAxis(ToRad(180));
//	projlights[1].casts_shadow = true;

	projlights[0].MakeParent( &projlights[1] );


	shdr_parab.LoadCompileLink( "shaders/paraboloid.vert", "shaders/paraboloid.frag" );

	imp.Load( "models/imp/imp.mesh" );
	//imp.Load( "maps/temple/column.mesh" );
	imp.translation_lspace = vec3_t( 0.0, 2.11, 0.0 );
	imp.scale_lspace = 0.7;
	imp.rotation_lspace.RotateXAxis( -PI/2 );

	mcube.Load( "meshes/horse/horse.mesh" );
	mcube.translation_lspace = vec3_t( -2, 0, 1 );
	mcube.scale_lspace = 0.5;
	mcube.rotation_lspace.RotateXAxis(ToRad(-90));

	cube.material = ass::LoadMat( "materials/default.mat" );
	sphere.material = ass::LoadMat( "materials/default.mat" );
	flore.material = ass::LoadMat( "materials/checkboard.mat" );

	scene::Register( &sphere );
	scene::Register( &cube );
	scene::Register( &flore );
	scene::Register( &imp );
	scene::Register( &mcube );
	scene::Register( &main_cam );
	scene::Register( &point_lights[0] );
	scene::Register( &point_lights[1] );
	scene::Register( &projlights[0] );
	scene::Register( &projlights[1] );

	glMaterialfv( GL_FRONT, GL_AMBIENT, &vec4_t( 0.1, 0.1, 0.1, 0.1 )[0] );
	glMaterialfv( GL_FRONT, GL_DIFFUSE, &vec4_t( 0.5, 0.0, 0.0, 0.1 )[0] );
	glMaterialfv( GL_FRONT, GL_SPECULAR, &vec4_t( 1., 1., 1., 0.1 )[0] );
	glMaterialf( GL_FRONT, GL_SHININESS, 100.0 );

	const char* skybox_fnames [] = { "env/hellsky2_forward.tga", "env/hellsky2_back.tga", "env/hellsky2_left.tga", "env/hellsky2_right.tga",
																	 "env/hellsky2_up.tga", "env/hellsky2_down.tga" };
	scene::skybox.Load( skybox_fnames );

	flore.material->diffuse_map->TexParameter( GL_TEXTURE_WRAP_S, GL_REPEAT );
	flore.material->diffuse_map->TexParameter( GL_TEXTURE_WRAP_T, GL_REPEAT );

	parab_tex.CreateEmpty( PARABOLOID_SIZE*2, PARABOLOID_SIZE, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE );

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
		if( i::keys[ SDLK_3 ] ) mover = &projlights[0];
		if( i::keys[ SDLK_4 ] ) mover = &point_lights[1];
		if( i::keys[ SDLK_5 ] ) mover = &projlights[1];

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













//		glClearColor( 0.5, 0.0, 0.0, 1.0 );
//		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
//
//		r::SetViewport( 0, 0, PARABOLOID_SIZE, PARABOLOID_SIZE );
//		r::SetProjectionViewMatrices( projlights[0].camera );
//
//		//scene::skybox.Render( main_cam.GetViewMatrix().GetRotationPart() );
//
//		glEnable( GL_DEPTH_TEST );
//		shdr_parab.Bind();
//		//glUniformMatrix4fv( shdr_parab.GetUniLocation("matrix"), 1, false, &(main_cam.GetViewMatrix().Inverted())[0] );
//
//		for( uint i=0; i<scene::meshes.size(); i++ )
//		{
//			mesh_t* mesh = scene::meshes[i];
//
//			if( mesh->material->diffuse_map )
//				mesh->material->diffuse_map->Bind(0);
//
//			mesh->Render();
//		}
//		r::NoShaders();
//
//		parab_tex.Bind();
//		glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0.0, 0.0, 0.0, 0.0, PARABOLOID_SIZE, PARABOLOID_SIZE );
//
//
//
//
//
//
//
//
//
//
//		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
//
//		r::SetViewport( 0, 0, PARABOLOID_SIZE, PARABOLOID_SIZE );
//		r::SetProjectionViewMatrices( projlights[1].camera );
//
//		//scene::skybox.Render( main_cam.GetViewMatrix().GetRotationPart() );
//
//		glEnable( GL_DEPTH_TEST );
//		shdr_parab.Bind();
//		//glUniformMatrix4fv( shdr_parab.GetUniLocation("matrix"), 1, false, &(main_cam.GetViewMatrix().Inverted())[0] );
//
//		for( uint i=0; i<scene::meshes.size(); i++ )
//		{
//			mesh_t* mesh = scene::meshes[i];
//
//			if( mesh->material->diffuse_map )
//				mesh->material->diffuse_map->Bind(0);
//
//			mesh->Render();
//		}
//		r::NoShaders();
//
//		parab_tex.Bind();
//		glCopyTexSubImage2D( GL_TEXTURE_2D, 0, PARABOLOID_SIZE-1, 0.0, 0.0, 0.0, PARABOLOID_SIZE, PARABOLOID_SIZE );

















//		glClearColor( 0.2, 0.2, 0.2, 1.0 );
//		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
//		r::SetViewport( 0, 0, r::w, r::h );
//		r::SetProjectionViewMatrices( main_cam );
//
//		r::NoShaders();
//		r::RenderGrid();
//
//		parab_tex.Bind();
//		glEnable( GL_TEXTURE_2D );
//		r::RenderQuad( 4.0, 2.0 );



		glClearColor( 0.5, 0.0, 0.0, 1.0 );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		r::SetViewport( 0, 0, r::w, r::w );
		r::SetProjectionViewMatrices( main_cam );


		//scene::skybox.Render( main_cam.GetViewMatrix().GetRotationPart() );

		glEnable( GL_DEPTH_TEST );
		shdr_parab.Bind();
		r::RenderGrid();
		//glUniformMatrix4fv( shdr_parab.GetUniLocation("matrix"), 1, false, &(main_cam.GetViewMatrix().Inverted())[0] );

		for( uint i=0; i<scene::meshes.size(); i++ )
		{
			mesh_t* mesh = scene::meshes[i];

			if( mesh->material->diffuse_map )
				mesh->material->diffuse_map->Bind(0);

			mesh->Render();
		}
		r::NoShaders();


		r::dbg::RenderDebug( main_cam );

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
