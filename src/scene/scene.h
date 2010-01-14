#ifndef _SCENE_H_
#define _SCENE_H_

#include "common.h"
#include "skybox.h"


class node_t;
class light_t;
class camera_t;
class mesh_node_t;
class skel_node_t;
class controller_t;


namespace scene {

// misc
extern skybox_t skybox;
inline vec3_t GetAmbientColor() { return vec3_t( 0.1, 0.05, 0.05 )*1; }

// funcs
extern void RegisterNode( node_t* node );
extern void UnregisterNode( node_t* node );
extern void RegisterController( controller_t* controller );
extern void UnregisterController( controller_t* controller );

extern void UpdateAllWorldStuff();
extern void UpdateAllCotrollers();


// container_t
/// entities container class
template<typename type_t> class container_t: public vec_t<type_t*>
{
}; // end class container_t



// typedefs
typedef container_t<node_t> container_node_t;
typedef container_t<light_t> container_light_t;
typedef container_t<camera_t> container_camera_t;
typedef container_t<mesh_node_t> container_mesh_node_t;
typedef container_t<skel_node_t> container_skel_node_t;

// containers
extern container_node_t       nodes;
extern container_light_t      lights;
extern container_camera_t     cameras;
extern container_mesh_node_t  mesh_nodes;
extern container_skel_node_t  skel_nodes;

extern vec_t<controller_t*>   controllers;


} // end namespace
#endif
