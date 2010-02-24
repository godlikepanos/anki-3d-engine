#ifndef _LIGHTPROPS_H_
#define _LIGHTPROPS_H_

#include "common.h"
#include "Resource.h"
#include "gmath.h"


class Texture;


/// Light properties resource
class LightProps: public Resource
{
	// data
	PROPERTY_R( vec3_t, diffuseCol, getDiffuseColor )
	PROPERTY_R( vec3_t, specularCol, getSpecularColor )
	PROPERTY_R( float, radius, getRadius ) ///< For point lights
	PROPERTY_R( bool, castsShadow_, castsShadow ) ///< For spot lights
	PROPERTY_R( float, distance, getDistance ) ///< For spot lights. A.K.A.: camera's zfar
	PROPERTY_R( float, fovX, getFovX ) ///< For spot lights
	PROPERTY_R( float, fovY, getFovY ) ///< For spot lights
		
	private:
		Texture* texture; ///< For spot lights
	public:
		const Texture* getTexture() const { DEBUG_ERR(texture==NULL); return texture; }
	
	// funcs	
	public:
		LightProps():
			diffuseCol(0.5),
			specularCol(0.5),
			radius(1.0),
			castsShadow_(false),
			distance(3.0),
			fovX(m::PI/4.0),
			fovY(m::PI/4.0),
			texture(NULL) 
		{}
		virtual ~LightProps() { /* ToDo */ }
		bool load( const char* filename );
		void unload();
};

#endif
