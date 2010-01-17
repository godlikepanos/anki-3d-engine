#ifndef _LIGHT_PROPS_H_
#define _LIGHT_PROPS_H_

#include "common.h"
#include "resource.h"
#include "gmath.h"


class texture_t;


/// Light properties resource
class light_props_t: public resource_t
{
	// data
	PROPERTY_R( vec3_t, diffuse_col, GetDiffuseColor )
	PROPERTY_R( vec3_t, specular_col, GetSpecularColor )
	PROPERTY_R( float, radius, GetRadius ) ///< For point lights
	PROPERTY_R( bool, casts_shadow, CastsShadow ) ///< For spot lights
	PROPERTY_R( float, distance, GetDistance ) ///< For spot lights. A.K.A.: camera's zfar
	PROPERTY_R( float, fov_x, GetFovX ) ///< For spot lights
	PROPERTY_R( float, fov_y, GetFovY ) ///< For spot lights
		
	private:
		texture_t* texture; ///< For spot lights
	public:
		const texture_t* GetTexture() const { DEBUG_ERR(texture==NULL); return texture; }
	
	// funcs	
	public:
		light_props_t():
			diffuse_col(0.5),
			specular_col(0.5),
			radius(1.0),
			casts_shadow(false),
			distance(3.0),
			fov_x(m::PI/4.0),
			fov_y(m::PI/4.0),
			texture(NULL) 
		{}
		virtual ~light_props_t() { /* ToDo */ }
		bool Load( const char* filename );
		void Unload();
};

#endif
