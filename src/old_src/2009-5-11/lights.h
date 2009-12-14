#ifndef _LIGHTS_H_
#define _LIGHTS_H_

#include "common.h"
#include "renderer.h"
#include "primitives.h"
#include "texture.h"

/*
LIGHTING MODEL

Final intensity:                If = Ia + Id + Is
Ambient intensity:              Ia = Al * Am
Ambient intensity of light:     Al
Ambient intensity of material:  Am
Defuse intensity:               Id = Dl * Dm * LambertTerm
Defuse intensity of light:      Dl
Defuse intensity of material:   Dm
LambertTerm:                    max( Normal dot Light, 0.0 )
Specular intensity:             Is = Sm x Sl x pow( max(R dot E, 0.0), f )
Specular intensity of light:    Sl
Specular intensity of material: Sm
*/

class light_runtime_class_t: public runtime_class_t {}; // for ambiguity reasons

class light_t: public object_t, public light_runtime_class_t
{
	public:
		// the light types
		enum types_e
		{
			POINT,
			DIRECTIONAL,
			SPOT
		};

		ushort type;

		uint gl_id;
		static uint lights_num;

		// funcs
		light_t(): type(POINT) { gl_id = lights_num++; };
		~light_t() {};
		void Update(); // Update the ogl with the changes in pos and dir. Call it before rendering and after camera transf

		// set params
		void SetAmbient ( const vec4_t& col ) { glLightfv( GL_LIGHT0+gl_id, GL_AMBIENT,  &((vec4_t&)col)[0] ); }
		void SetDiffuse ( const vec4_t& col ) { glLightfv( GL_LIGHT0+gl_id, GL_DIFFUSE,  &((vec4_t&)col)[0] ); }
		void SetSpecular( const vec4_t& col ) { glLightfv( GL_LIGHT0+gl_id, GL_SPECULAR, &((vec4_t&)col)[0] ); }
		void SetCutOff  ( float c )           { DEBUG_ERR( (c>90.0||c<0.0)  && c!=180.0) glLightf( GL_LIGHT0+gl_id, GL_SPOT_CUTOFF, c ); }
		void SetExp     ( float exp )         { DEBUG_ERR( exp<0.0 || exp>128.0 ) glLightf( GL_LIGHT0+gl_id, GL_SPOT_EXPONENT, exp ); }

		// get
		vec4_t GetAmbient() const  { vec4_t col; glGetLightfv(GL_LIGHT0+gl_id, GL_AMBIENT,  &col[0]); return col; }
		vec4_t GetDiffuse() const  { vec4_t col; glGetLightfv(GL_LIGHT0+gl_id, GL_DIFFUSE,  &col[0]); return col; }
		vec4_t GetSpecular() const { vec4_t col; glGetLightfv(GL_LIGHT0+gl_id, GL_SPECULAR, &col[0]); return col; }
		float  GetCutOff() const   { float cutoff; glGetLightfv( GL_LIGHT0+gl_id, GL_SPOT_CUTOFF, &cutoff ); return cutoff; }
		float  GetExp() const      { float exp; glGetLightfv( GL_LIGHT0+gl_id, GL_SPOT_EXPONENT, &exp ); return exp; }
		vec3_t GetDir() const      { return world_rotation.GetColumn(0); } // the light direction is always the x axis

		void Render();
};

///                                                                                             /
class light2_t: public object_t, public light_runtime_class_t
{
	public:
		enum types_e
		{
			POINT,
			PROJ_TXTR
		};

		vec3_t diffuse_color;
		vec3_t specular_color;
		types_e type;
};

class point_light_t: public light2_t
{
	public:
		float radius;
};

class proj_light_t: public light2_t
{
	public:
		camera_t camera;
		texture_t txtr;
		bool casts_shadow;

		proj_light_t(): casts_shadow(false) { MakeParent(&camera); }
};


#endif
