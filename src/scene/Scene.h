#ifndef _SCENE_H_
#define _SCENE_H_

#include "Common.h"
#include "skybox.h"


class Node;
class Light;
class Camera;
class MeshNode;
class SkelNode;
class Controller;


namespace Scene {

// misc
extern Skybox skybox;
inline Vec3 getAmbientColor() { return Vec3( 0.1, 0.05, 0.05 )*1; }
inline Vec3 getSunPos() { return Vec3( 0.0, 1.0, -1.0 ) * 50.0; }

// funcs
extern void registerNode( Node* node );
extern void unregisterNode( Node* node );
extern void registerController( Controller* controller );
extern void unregisterController( Controller* controller );

extern void updateAllWorldStuff();
extern void updateAllControllers();


// Container
/// entities container class
template<typename Type> class Container: public Vec<Type*>
{
}; // end class Container



// typedefs
typedef Container<Node> NodeContainer;
typedef Container<Light> LightContainer;
typedef Container<Camera> CameraContainer;
typedef Container<MeshNode> MeshContainer;
typedef Container<SkelNode> SkelNodeContainer;

// containers
extern NodeContainer      nodes;
extern LightContainer     lights;
extern CameraContainer    cameras;
extern MeshContainer      meshNodes;
extern SkelNodeContainer  skelNodes;

extern Vec<Controller*> controllers;


} // end namespace
#endif
