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
#include "Texture.h"
#include "Node.h"
#include "Camera.h"

class LightProps;


/// Light (A)
class Light: public Node
{
	public:
		enum Type { LT_POINT, LT_SPOT };

		Type type;
		LightProps* lightProps; ///< Later we will add a controller
	
		Light( Type type_ ): Node(NT_LIGHT), type(type_) {}
		//void init( const char* );
		void deinit();
};


/// PointLight
class PointLight: public Light
{
	public:
		float radius;

		PointLight(): Light(LT_POINT) {}
		void init( const char* );
		void render();
};


/// SpotLight
class SpotLight: public Light
{
	public:
		Camera camera;
		bool castsShadow;

		SpotLight(): Light(LT_SPOT), castsShadow(false) { addChild( &camera ); }
		float getDistance() const { return camera.getZFar(); }
		void  setDistance( float d ) { camera.setZFar(d); }
		void  init( const char* );
		void  render();
};


#endif
