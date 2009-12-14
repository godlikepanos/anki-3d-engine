#ifndef _MODEL_H_
#define _MODEL_H_

#include "common.h"
#include "resource.h"

class mesh_data_t;

class model_t: public resource_t
{
	public:
		vector<mesh_data_t*> meshes;

		bool Load( const char* filename );
		void Unload();
};

#endif
