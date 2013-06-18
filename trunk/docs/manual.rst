**AnKi 3D Engine**

Copyright (C) 2009-2013 Panagiotis Christopoulos-Charitos

http://www.anki3d.org

godlike@ancient-ritual.com

.. contents:: Table of Contents


=======
License
=======

Anki is dual licensed. Simply put, you can use* it freely to create opensource 
software under GPLv3 and if you want to use* it for commercial you need to 
apply for a commercial licence. 

\*: as a whole or parts of the source


========
Building
========

AnKi build system is build over the popular CMake.

To download the latest version of AnKi from the SVN repository type: ::

	svn checkout http://anki-3d-engine.googlecode.com/svn/trunk/ anki

to build: ::

	cd path/to/anki
	cd build
	cmake -DCMAKE_BUILD_TYPE=Release ..
	make

to build the doxygen documentation: ::

	make doc


Required external libraries
---------------------------

A great effort was made to keep the number of external dependencies to minimum 
so the only prerequisites are the following:

- X11 development files (Package name under Ubuntu: libx11-dev)
- Mesa GL development files (Package name under Ubuntu: mesa-common-dev)

All the other libraries are inside the source tree and you don't have to worry
about them.

======
Assets
======

Currently there are no assets (models, textures, materials etc) so even if you
build it, the application will fail to run.


===================
System requirements
===================

The engine requires:

- GPU with shader model 4
- Linux OS
- Proprietary GPU drivers

Development rig: Ubuntu 11.10, nVidia 560 Ti. So it should be working on 
similar systems.
  

============
Coding style
============

Every project has some rules and here are some things to remember while coding
AnKi.


Types
-----

The classes, structs, typedefs, enums etc must be capitalized eg *ThisIsAClass*


Functions & variables
---------------------

All functions (including class methods) and all variables are mixed case.

All functions should have a verb inside them eg *doSomething()*

All variables should not have verbs eg *oneVariable*


Constants, macros & enumerators
-------------------------------

All constants, macros and enumerators are capital with undercores eg *#define 
MACRO(x)* or *const int ONE_INT = 10;*

All the constants should be defined without using the preprocessor eg dont write
*#define ONE_INT 10*

All enumerators have the first letters of the enum as prefix eg
*enum CarColors { CC_BLUE, CC_GREEN };*


Parenthesis, braces, comas & operators
--------------------------------------

After opening parenthesis and before closing it there is no spaces, same for
square brackets. Before and after an operator there is always a space

eg *((mat1 * 10) + 10)* or *setWidth(100)* or *int arr[100 + 1];*

After a coma there is a space eg *setSize(10, 20)*


Order in class definitions
--------------------------

class 

{

  friends
	
  pre-nested (very rare)
	
  nested
	
  properties
	
  public
	
  protected
	
  private 
	
}

inlines


Naming shortcuts
----------------

This list contains some of the naming shortcuts we use in AnKi. This is because
we are bored to type:

- Array                        : arr
- Animation                    : anim
- Application                  : app
- Buffer                       : buff
- Camera                       : cam
- Color                        : col
- Controller                   : ctrl
- Current                      : crnt
- Feature                      : feat
- Fragment                     : frag
- Framebuffer Attachable Image : fai
- Geometry                     : geom
- Location                     : loc
- Material                     : mtl
- Matrix                       : mat
- Number                       : num
- Physics                      : phy
- Position                     : pos
- Property                     : prop
- Quadrilateral                : quad
- Quaternion                   : quat
- Resource                     : rsrc
- Rotation                     : rot
- Shader                       : shdr
- Shader Program               : shaderProg or sProg
- Skeletal Animation           : sAnim
- Skeleton                     : skel
- Text                         : txt
- Texture                      : tex
- Transformation               : trf
- Transform Feedback           : trffb
- Translation                  : tsl
- Triangle                     : tri
- Utility                      : util
- Variable                     : var
- Vector                       : vec
- Vertex                       : vert

Anything else should be typed full.


Controllers
-----------

The controllers are part of the scene node objects. They control the node's
behaviour. 

They have an input (script, animation, etc) and they control a scene node. The
naming convention of the controllers is:

<what the controller controls><the input of the contoller>Ctrl

For Example:

MeshSkelNodeCtrl A Mesh is controlled by a SkelNode


GLSL shaders
------------

The same rules apply to GLSL shaders but with a few changes:

All the vars you can find in a GLSL shader program are either attributes,
uniforms or in/out vars (varyings) and everything else. The attributes and
uniforms are mixed case. The in/out vars are mixed case as well but they have a
prefix string that indicates their output. For example if a var is output from
the vertex shader it will have a 'v' before its name. The In detail:

v: Vertex shader
tc: Tessellation control shader
te: Tessellation evaluation shader
g: Geometry shader
f: Fragment shader

All the other variables (locals and globals) inside the code are mixed case but
with a leading and a following underscore. 


Submitting patches
------------------

If you want to update/patch a file (for example Main.cpp) do:

- Make the changes on that file
- Save the differences in a file using "svn diff Main.cpp > /tmp/diff"
- E-mail the "diff" file with subject "[PATCH] Main.cpp updates"


=========
ToDo list
=========

- Continue working on the new coding style in shaders
- Changes in the blending objects problem. The BS will become one stage and the
  PPS will be divided in two steps. The first will apply the SSAO and the EdgeAA
  in the IS_FAI and the second will do the rest
- The second Physics demo: Create a box that is geting moved by the user. It has
  to interact with the other boxes
- Set the gravity of a certain body to a lower value and see how it behaves
- In the Ragdoll bullet demo try to change the distances of the bodies
- Ask in the bullet forum:
	- How to make floating particles like smoke. But first try with one body and
	  manualy setting the gravity
	- What the btCollisionObject::setActivationState takes as parameter?
- Re-enable the stencil tex in Ms.cpp and replace all the stencil buffers with
  that (Smo, Bs) to save memory
- See if the restrictions in FBOs (all FAIs same size) still apply
- See what happens if I write *#pragma anki attribute <randName> 10* where
  randName does not exist. Do the same for tranform feedback varyings
  
