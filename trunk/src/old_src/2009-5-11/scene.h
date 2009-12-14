#ifndef _SCENE_H_
#define _SCENE_H_

#include "common.h"
#include "primitives.h"
#include "spatial.h"
#include "lights.h"
#include "geometry.h"
#include "model.h"
#include "skybox.h"

namespace scene {

extern vector<object_t*> objects;
extern vector<spatial_t*> spatials;
extern vector<light_t*> lights;
extern vector<model_t*> models;
extern vector<mesh_t*> meshes;
extern vector<camera_t*> cameras;

// object_t
extern void Register( object_t* obj );
extern void UnRegister( object_t* obj );

// spatial_t
extern void Register( spatial_t* spa );
extern void UnRegister( spatial_t* spa );

// light_t
extern void Register( light_t* light );
extern void UnRegister( light_t* light );

// mesh_t
extern void Register( mesh_t* mesh );
extern void UnRegister( mesh_t* mesh );

// model_t
extern void Register( model_t* mdl );
extern void UnRegister( model_t* mdl );

// camera_t
extern void Register( camera_t* mdl );
extern void UnRegister( camera_t* mdl );

// MISC
extern void UpdateAllWorldTrfs();
extern void RenderAllObjs();
extern void InterpolateAllModels();
extern void UpdateAllLights();
extern void UpdateAllCameras();

extern skybox_t skybox;

} // end namespace
#endif
