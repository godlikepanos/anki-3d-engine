#ifndef _LIGHT_MTL_H_
#define _LIGHT_MTL_H_

#include "common.h"
#include "resource.h"
#include "gmath.h"


class texture_t;


/// Light material resource
class light_mtl_t: public resource_t
{
	// data
	PROPERTY_R( vec3_t, diffuse_col, GetDiffuseColor );
	PROPERTY_R( vec3_t, specular_col, GetSpecularColor );
		
	private:
		texture_t* texture;
	public:
		const texture_t* GetTexture() const { DEBUG_ERR(texture==NULL); return texture; }
	
	// funcs	
	public:
		light_mtl_t(): diffuse_col(0.5), specular_col(0.5), texture(NULL) {}
		virtual ~light_mtl_t() { /* ToDo */ }
		bool Load( const char* filename );
		void Unload();
};

#endif
