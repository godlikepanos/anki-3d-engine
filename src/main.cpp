#include <stdio.h>
#include <iostream>
#include <fstream>
#include <typeinfo>
#include "Common.h"

#include "Input.h"
#include "Camera.h"
#include "Math.h"
#include "Renderer.h"
#include "Ui.h"
#include "App.h"
#include "particles.h"
#include "Texture.h"
#include "Mesh.h"
#include "Light.h"
#include "collision.h"
#include "Material.h"
#include "Resource.h"
#include "Scene.h"
#include "Scanner.h"
#include "skybox.h"
#include "map.h"
#include "MeshNode.h"
#include "SkelModelNode.h"
#include "MeshNode.h"
#include "SkelAnim.h"
#include "MeshSkelNodeCtrl.h"
#include "SkelAnimCtrl.h"
#include "SkelNode.h"
#include "LightProps.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "BulletDebuger.h"


// map (hard coded)
Camera* mainCam;
MeshNode* floor__,* sarge,* horse;
SkelModelNode* imp;
PointLight* point_lights[10];
SpotLight* spot_lights[2];

class floor_t: public Camera
{
	public:
		void render()
		{
			R::Dbg::renderCube( true, 1.0 );
		}

		void renderDepth()
		{
			R::Dbg::renderCube( true, 1.0 );
		}
}* floor_;


// Physics
btDefaultCollisionConfiguration* collisionConfiguration;
btCollisionDispatcher* dispatcher;
btDbvtBroadphase* broadphase;
btSequentialImpulseConstraintSolver* sol;
btDiscreteDynamicsWorld* dynamicsWorld;
BulletDebuger debugDrawer;

#define ARRAY_SIZE_X 5
#define ARRAY_SIZE_Y 5
#define ARRAY_SIZE_Z 5

#define MAX_PROXIES (ARRAY_SIZE_X*ARRAY_SIZE_Y*ARRAY_SIZE_Z + 1024)

#define SCALING 1.
#define START_POS_X -5
#define START_POS_Y -5
#define START_POS_Z -3


void initPhysics()
{
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new	btCollisionDispatcher(collisionConfiguration);
	broadphase = new btDbvtBroadphase();
	sol = new btSequentialImpulseConstraintSolver;
	dynamicsWorld = new btDiscreteDynamicsWorld( dispatcher, broadphase, sol, collisionConfiguration );

	dynamicsWorld->setGravity(btVector3(0,-10,0));

	btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(50.),btScalar(50.),btScalar(50.)));

	btTransform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(btVector3(0,-50,0));

	//We can also use DemoApplication::localCreateRigidBody, but for clarity it is provided here:
	{
		btScalar mass(0.);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0,0,0);
		if (isDynamic)
			groundShape->calculateLocalInertia(mass,localInertia);

		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,groundShape,localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		//add the body to the dynamics world
		dynamicsWorld->addRigidBody(body);
	}


	{
		//create a few dynamic rigidbodies
		// Re-using the same collision is better for memory usage and performance

		btCollisionShape* colShape = new btBoxShape(btVector3(SCALING*1,SCALING*1,SCALING*1));
		//btCollisionShape* colShape = new btSphereShape(btScalar(1.));

		/// Create Dynamic Objects
		btTransform startTransform;
		startTransform.setIdentity();

		btScalar	mass(1.0);

		btVector3 localInertia(0,0,0);
		colShape->calculateLocalInertia(mass,localInertia);

		float start_x = START_POS_X - ARRAY_SIZE_X/2;
		float start_y = START_POS_Y;
		float start_z = START_POS_Z - ARRAY_SIZE_Z/2;

		for (int k=0;k<ARRAY_SIZE_Y;k++)
		{
			for (int i=0;i<ARRAY_SIZE_X;i++)
			{
				for(int j = 0;j<ARRAY_SIZE_Z;j++)
				{
					startTransform.setOrigin(SCALING*btVector3(
										btScalar(2.0*i + start_x),
										btScalar(20+2.0*k + start_y),
										btScalar(2.0*j + start_z)));


					//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
					btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
					btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,colShape,localInertia);
					btRigidBody* body = new btRigidBody(rbInfo);

					//body->setActivationState(ISLAND_SLEEPING);

					dynamicsWorld->addRigidBody(body);
					//body->setActivationState(ISLAND_SLEEPING);
				}
			}
		}
	}

	dynamicsWorld->setDebugDrawer(&debugDrawer);
}


//=====================================================================================================================================
// init                                                                                                                               =
//=====================================================================================================================================
void init()
{
	PRINT( "Engine initializing..." );

	initPhysics();

	srand( unsigned(time(NULL)) );
	mathSanityChecks();

	App::initWindow();
	uint ticks = App::getTicks();

	R::init();
	Ui::init();

	// camera
	mainCam = new Camera( R::aspectRatio*toRad(60.0), toRad(60.0), 0.5, 200.0 );
	mainCam->moveLocalY( 3.0 );
	mainCam->moveLocalZ( 5.7 );
	mainCam->moveLocalX( -0.3 );

	// lights
	point_lights[0] = new PointLight();
	point_lights[0]->init( "maps/temple/light0.light" );
	point_lights[0]->setLocalTransformation( Vec3( -1.0, 2.4, 1.0 ), Mat3::getIdentity(), 1.0 );
	point_lights[1] = new PointLight();
	point_lights[1]->init( "maps/temple/light1.light" );
	point_lights[1]->setLocalTransformation( Vec3( 2.5, 1.4, 1.0 ), Mat3::getIdentity(), 1.0 );

	spot_lights[0] = new SpotLight();
	spot_lights[0]->init( "maps/temple/light2.light" );
	spot_lights[0]->setLocalTransformation( Vec3( 1.3, 4.3, 3.0 ), Mat3( Euler(toRad(-20), toRad(20), 0.0) ), 1.0 );
	spot_lights[1] = new SpotLight();
	spot_lights[1]->init( "maps/temple/light3.light" );
	spot_lights[1]->setLocalTransformation( Vec3( -2.3, 6.3, 2.9 ), Mat3( Euler(toRad(-70), toRad(-20), 0.0) ), 1.0 );

	// horse
	horse = new MeshNode();
	horse->init( "meshes/horse/horse.mesh" );
	horse->setLocalTransformation( Vec3( -2, 0, 1 ), Mat3( Euler(-M::PI/2, 0.0, 0.0) ), 0.5 );
	
	// sarge
	sarge = new MeshNode();
	sarge->init( "meshes/sphere/sphere16.mesh" );
	//sarge->setLocalTransformation( Vec3( 0, -2.8, 1.0 ), Mat3( Euler(-M::PI/2, 0.0, 0.0) ), 1.1 );
	sarge->setLocalTransformation( Vec3( 0, 2.0, 2.0 ), Mat3::getIdentity(), 0.4 );
	
	// floor
	floor__ = new MeshNode();
	floor__->init( "maps/temple/Cube.019.mesh" );
	floor__->setLocalTransformation( Vec3(0.0, -0.19, 0.0), Mat3( Euler(-M::PI/2, 0.0, 0.0) ), 0.8 );

	// imp	
	imp = new SkelModelNode();
	imp->init( "models/imp/imp.smdl" );
	imp->setLocalTransformation( Vec3( 0.0, 2.11, 0.0 ), Mat3( Euler(-M::PI/2, 0.0, 0.0) ), 0.7 );
	imp->meshNodes[0]->meshSkelCtrl->skelNode->skelAnimCtrl->skelAnim = rsrc::skelAnims.load( "models/imp/walk.imp.anim" );
	imp->meshNodes[0]->meshSkelCtrl->skelNode->skelAnimCtrl->step = 0.8;

	// crate
	MeshNode* crate = new MeshNode;
	crate->init( "models/crate0/crate0.mesh" );
	crate->scaleLspace = 1.0;


	//
	//floor_ = new floor_t;
	//floor_->material = rsrc::materials.load( "materials/default.mtl" );

	const char* skybox_fnames [] = { "textures/env/hellsky4_forward.tga", "textures/env/hellsky4_back.tga", "textures/env/hellsky4_left.tga",
																	 "textures/env/hellsky4_right.tga", "textures/env/hellsky4_up.tga", "textures/env/hellsky4_down.tga" };
	Scene::skybox.load( skybox_fnames );

	PRINT( "Engine initialization ends (" << App::getTicks()-ticks << ")" );
	cerr.flush();
}


//=====================================================================================================================================
// main                                                                                                                               =
//=====================================================================================================================================
int main( int /*argc*/, char* /*argv*/[] )
{
	float f = M::sin( 10.0 );
	PRINT( f );

	App::printAppInfo();

	init();

	PRINT( "Entering main loop" );
	int ticks = App::getTicks();
	uint ms = 0;
	do
	{
		int ticks_ = App::getTicks();
		I::handleEvents();
		R::prepareNextFrame();

		float dist = 0.2;
		float ang = toRad(3.0);
		float scale = 0.01;

		// move the camera
		static Node* mover = mainCam;

		if( I::keys[ SDLK_1 ] ) mover = mainCam;
		if( I::keys[ SDLK_2 ] ) mover = point_lights[0];
		if( I::keys[ SDLK_3 ] ) mover = spot_lights[0];
		if( I::keys[ SDLK_4 ] ) mover = point_lights[1];
		if( I::keys[ SDLK_5 ] ) mover = spot_lights[1];
		if( I::keys[ SDLK_m ] == 1 ) I::warpMouse = !I::warpMouse;

		if( I::keys[SDLK_a] ) mover->moveLocalX( -dist );
		if( I::keys[SDLK_d] ) mover->moveLocalX( dist );
		if( I::keys[SDLK_LSHIFT] ) mover->moveLocalY( dist );
		if( I::keys[SDLK_SPACE] ) mover->moveLocalY( -dist );
		if( I::keys[SDLK_w] ) mover->moveLocalZ( -dist );
		if( I::keys[SDLK_s] ) mover->moveLocalZ( dist );
		if( !I::warpMouse )
		{
			if( I::keys[SDLK_UP] ) mover->rotateLocalX( ang );
			if( I::keys[SDLK_DOWN] ) mover->rotateLocalX( -ang );
			if( I::keys[SDLK_LEFT] ) mover->rotateLocalY( ang );
			if( I::keys[SDLK_RIGHT] ) mover->rotateLocalY( -ang );
		}
		else
		{
			float accel = 44.0;
			mover->rotateLocalX( ang * I::mouseVelocity.y * accel );
			mover->rotateLocalY( -ang * I::mouseVelocity.x * accel );
		}
		if( I::keys[SDLK_q] ) mover->rotateLocalZ( ang );
		if( I::keys[SDLK_e] ) mover->rotateLocalZ( -ang );
		if( I::keys[SDLK_PAGEUP] ) mover->scaleLspace += scale ;
		if( I::keys[SDLK_PAGEDOWN] ) mover->scaleLspace -= scale ;

		if( I::keys[SDLK_k] ) mainCam->lookAtPoint( point_lights[0]->translationWspace );

		mover->rotationLspace.reorthogonalize();


		Scene::updateAllControllers();
		Scene::updateAllWorldStuff();


		dynamicsWorld->stepSimulation( App::timerTick );
		dynamicsWorld->debugDrawWorld();

		R::render( *mainCam );

		//map.octree.root->bounding_box.render();

		// print some debug stuff
		Ui::setColor( Vec4(1.0, 1.0, 1.0, 1.0) );
		Ui::setPos( -0.98, 0.95 );
		Ui::setFontWidth( 0.03 );
		Ui::printf( "frame:%d time:%dms\n", R::framesNum, App::getTicks()-ticks_ );
		//Ui::print( "Movement keys: arrows,w,a,s,d,q,e,shift,space\nSelect objects: keys 1 to 5\n" );
		Ui::printf( "Mover: Pos(%.2f %.2f %.2f) Angs(%.2f %.2f %.2f)", mover->translationWspace.x, mover->translationWspace.y, mover->translationWspace.z,
								 toDegrees(Euler(mover->rotationWspace).x), toDegrees(Euler(mover->rotationWspace).y), toDegrees(Euler(mover->rotationWspace).z) );

		if( I::keys[SDLK_ESCAPE] ) break;
		if( I::keys[SDLK_F11] ) App::togleFullScreen();
		if( I::keys[SDLK_F12] == 1 ) R::takeScreenshot("gfx/screenshot.jpg");

		/*char str[128];
		if( R::framesNum < 1000 )
			sprintf( str, "capt/%06d.jpg", R::framesNum );
		else
			sprintf( str, "capt2/%06d.jpg", R::framesNum );
		R::takeScreenshot(str);*/

		// std stuff follow
		SDL_GL_SwapBuffers();
		R::printLastError();
		if( 1 )
		{
			//if( R::framesNum == 10 ) R::takeScreenshot("gfx/screenshot.tga");
			App::waitForNextFrame();
		}
		else
			if( R::framesNum == 5000 ) break;
	}while( true );
	PRINT( "Exiting main loop (" << App::getTicks()-ticks << ")" );


	PRINT( "Exiting..." );
	App::quitApp( EXIT_SUCCESS );
	return 0;
}
