#ifndef _LIGHTS_H_
#define _LIGHTS_H_

#include "common.h"
#include "primitives.h"
#include "texture.h"
#include "object.h"
#include "camera.h"

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

class light_data_user_class_t: public data_user_class_t {}; // for ambiguity reasons

// light_t (A)
class light_t: public object_t, public light_data_user_class_t
{
	public:
		enum types_e
		{
			POINT,
			SPOT
		};

	protected:
		vec3_t diffuse_color;
		vec3_t specular_color;
		types_e type;
		void RenderDebug();

	public:
		light_t( types_e type_ ): object_t(LIGHT), type(type_) {}


		// Gets & Sets
		const vec3_t GetDiffuseColor() const { return diffuse_color; }
		const vec3_t GetSpecularColor() const { return specular_color; }
		types_e GetType() const { return type; }
		void SetDiffuseColor( const vec3_t col ) { diffuse_color = col; }
		void SetSpecularColor( const vec3_t col ) { specular_color = col; }
};


// point_light_t
class point_light_t: public light_t
{
	public:
		float radius;

		point_light_t(): light_t(POINT) {}
		void Render();
		void RenderDepth() {}
};


// spot_light_t
class spot_light_t: public light_t
{
	public:
		camera_t camera;
		texture_t* texture;
		bool casts_shadow;

		spot_light_t(): light_t(SPOT), texture(NULL), casts_shadow(false) { MakeParent(&camera); }
		float GetDistance() const { return camera.GetZFar(); }
		void  SetDistance( float d ) { camera.SetZFar(d); }
		void Render();
		void RenderDepth() {};
};


#endif
