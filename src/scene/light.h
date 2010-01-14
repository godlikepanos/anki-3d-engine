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

#ifndef _LIGHT_H_
#define _LIGHT_H_

#include "common.h"
#include "texture.h"
#include "node.h"
#include "camera.h"

class light_mtl_t;


/// light_t (A)
class light_t: public node_t
{
	public:
		enum type_e { LT_POINT, LT_SPOT };

		type_e type;
		light_mtl_t* light_mtl; ///< Later we will add a controller
	
		light_t( type_e type_ ): node_t(NT_LIGHT), type(type_) {}
		void Init( const char* );
		void Deinit();
};


/// point_light_t
class point_light_t: public light_t
{
	public:
		float radius;

		point_light_t(): light_t(LT_POINT) {}
		void Render();
};


/// spot_light_t
class spot_light_t: public light_t
{
	public:
		camera_t camera;
		bool casts_shadow;

		spot_light_t(): light_t(LT_SPOT), casts_shadow(false) { AddChild( &camera ); }
		float GetDistance() const { return camera.GetZFar(); }
		void  SetDistance( float d ) { camera.SetZFar(d); }
		void  Render();
};


#endif
