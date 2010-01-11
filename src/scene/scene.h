#ifndef _SCENE_H_
#define _SCENE_H_

#include "common.h"
#include "light.h"
#include "mesh_node.h"
#include "skybox.h"


namespace scene {

// misc
extern skybox_t skybox;
inline vec3_t GetAmbientColor() { return vec3_t( 0.1, 0.05, 0.05 )*1; }


extern void RegisterNodeAndChilds( node_t* node );
extern void UnregisterNodeAndChilds( node_t* node );
extern void UpdateAllWorldStuff();
extern void UpdateAllSkeletonNodes();


// container_t
/// entities container class
template<typename type_t> class container_t: public vec_t<type_t*>
{
}; // end class container_t



// conaiteners
typedef container_t<node_t> container_node_t;
typedef container_t<light_t> container_light_t;
typedef container_t<camera_t> container_camera_t;
typedef container_t<mesh_node_t> container_mesh_node_t;
typedef container_t<skel_node_t> container_skel_node_t;


extern container_node_t       nodes;
extern container_light_t      lights;
extern container_camera_t     cameras;
extern container_mesh_node_t  mesh_nodes;
extern container_skel_node_t  skel_nodes;


} // end namespace
#endif
