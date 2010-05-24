#include "Renderer.hpp"
#include "Camera.h" /// @todo remove this


//=====================================================================================================================================
// Vars                                                                                                                               =
//=====================================================================================================================================
bool Renderer::textureCompression = false;
int  Renderer::maxTextureUnits = -1;
bool Renderer::mipmapping = true;
int  Renderer::maxAnisotropy = 8;
float Renderer::quadVertCoords [][2] = { {1.0,1.0}, {0.0,1.0}, {0.0,0.0}, {1.0,0.0} };


//=====================================================================================================================================
// Constructor                                                                                                                        =
//=====================================================================================================================================
Renderer::Renderer():
	ms( *this ),
	is( *this ),
	pps( *this ),
	dbg( *this ),
	width( 640 ),
	height( 480 )
{
}

//=====================================================================================================================================
// init                                                                                                                               =
//=====================================================================================================================================
void Renderer::init()
{
	aspectRatio = float(width)/height;

	// init the stages. Careful with the order!!!!!!!!!!
	ms.init();
	is.init();
	pps.init();
	dbg.init();
}


//=====================================================================================================================================
// render                                                                                                                             =
//=====================================================================================================================================
void Renderer::render( Camera& cam_ )
{
	cam = &cam_;

	ms.run();
	is.run();
	pps.run();
	dbg.run();

	++framesNum;
}


//=====================================================================================================================================
// drawQuad                                                                                                                           =
//=====================================================================================================================================
void Renderer::drawQuad( int vertCoordsUniLoc )
{
	DEBUG_ERR( vertCoordsUniLoc == -1 );
	glVertexAttribPointer( vertCoordsUniLoc, 2, GL_FLOAT, false, 0, quadVertCoords );
	glEnableVertexAttribArray( vertCoordsUniLoc );
	glDrawArrays( GL_QUADS, 0, 4 );
	glDisableVertexAttribArray( vertCoordsUniLoc );
}

//=====================================================================================================================================
// setProjectionMatrix                                                                                                                =
//=====================================================================================================================================
void Renderer::setProjectionMatrix( const Camera& cam )
{
	glMatrixMode( GL_PROJECTION );
	loadMatrix( cam.getProjectionMatrix() );
}


//=====================================================================================================================================
// setViewMatrix                                                                                                                      =
//=====================================================================================================================================
void Renderer::setViewMatrix( const Camera& cam )
{
	glMatrixMode( GL_MODELVIEW );
	loadMatrix( cam.getViewMatrix() );
}


//=====================================================================================================================================
// unproject                                                                                                                          =
//=====================================================================================================================================
Vec3 Renderer::unproject( const Vec3& windowCoords, const Mat4& modelViewMat, const Mat4& projectionMat, const int view[4] )
{
	Mat4 invPm = projectionMat * modelViewMat;
	invPm.invert();

	// the vec is in ndc space meaning: -1<=vec.x<=1 -1<=vec.y<=1 -1<=vec.z<=1
	Vec4 vec;
	vec.x = (2.0*(windowCoords.x-view[0]))/view[2] - 1.0;
	vec.y = (2.0*(windowCoords.y-view[1]))/view[3] - 1.0;
	vec.z = 2.0*windowCoords.z - 1.0;
	vec.w = 1.0;

	Vec4 final = invPm * vec;
	final /= final.w;
	return Vec3( final );
}


//=====================================================================================================================================
// ortho                                                                                                                              =
//=====================================================================================================================================
Mat4 Renderer::ortho( float left, float right, float bottom, float top, float near, float far )
{
	float difx = right-left;
	float dify = top-bottom;
	float difz = far-near;
	float tx = -(right+left) / difx;
	float ty = -(top+bottom) / dify;
	float tz = -(far+near) / difz;
	Mat4 m;

	m(0,0) = 2.0 / difx;
	m(0,1) = 0.0;
	m(0,2) = 0.0;
	m(0,3) = tx;
	m(1,0) = 0.0;
	m(1,1) = 2.0 / dify;
	m(1,2) = 0.0;
	m(1,3) = ty;
	m(2,0) = 0.0;
	m(2,1) = 0.0;
	m(2,2) = -2.0 / difz;
	m(2,3) = tz;
	m(3,0) = 0.0;
	m(3,1) = 0.0;
	m(3,2) = 0.0;
	m(3,3) = 1.0;

	return m;
}


//=====================================================================================================================================
// getLastError                                                                                                                       =
//=====================================================================================================================================
const uchar* Renderer::getLastError()
{
	return gluErrorString( glGetError() );
}


//=====================================================================================================================================
// printLastError                                                                                                                     =
//=====================================================================================================================================
void Renderer::printLastError()
{
	GLenum errid = glGetError();
	if( errid != GL_NO_ERROR )
		ERROR( "OpenGL Error: " << gluErrorString( errid ) );
}

