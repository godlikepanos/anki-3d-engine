**AnKi 3D Engine**

Copyright (C) 2009-2011 Panayiotis Christopoulos-Charitos

http://www.ancient-ritual.com

godlike@ancient-ritual.com

.. contents:: Table of Contents


=======
License
=======

Anki is dual licensed. The first option is GLPv3 and the second commercial
license. If you want to use* AnKi to make opensource software (licensed under
GLPv3) then AnKi is provited to you for free but if you want to use it* for
non-GPLv3 licensed software then you have to apply for a commercial license.

\*: as a whole or parts of the source


========
Building
========

AnKi build system is very Linux specific (GNU make only) at the moment. It
also requires a few extra development libraries.

To download the latest version of AnKi from the SVN repository type:

$ svn checkout http://anki-3d-engine.googlecode.com/svn/trunk/ anki


Required external libraries
---------------------------

AnKi requires a few up to date versions of some libraries. The libraries are:
  
- Bullet Physics 2.77
- SDL 1.3
- GLEW 1.5.5
- boost 1.4
- libpng 1.2
- libjpeg 6b
- libpython 2.6

Normally, in order to build AnKi you need to have all of the above libraries. 
Some of them are not provided from the most Linux distros or they are older
versions. The libraries you have to download and build for yourself are Bullet,
SDL and GLEW. The other are pretty common and you can find them almost anywhere.


To ease the building process and to save you some time **some** of the above
libraries are already backed and placed inside the extern directory. The backed
libraries are for Linux and for x86-64 architecture. If you dont trust my
binaries and/or you want to build the libs yourselves you have to download,
build and install the libs in the extern directory manually. The script
*download-externs.sh* downloads the libraries (it requires SVN, mercurial,
CMake, autoconf) and the *do-externs.sh* builds the libraries and installs them
in the extern directory. Open the files and see how it is done.

You wont find any development files for boost, libpng, libjpeg and libpython
inside the extern dir. Get them using your Linux distribution's packet manager.


Building AnKi and optionally generating makefiles
-------------------------------------------------

Inside the build directory you can find 4 build targets containing GNU
makefiles. If you want to build AnKi just type "make" from inside a target.

**WARNING**: Sometimes I forget to update all the targets. The debug is always 
updated though.

AnKi uses a build system that generates the above makefiles. This build system
is no longer part of AnKi and its located in a different repository. This tool
is called gBuildSystem and you can find it in
http://godlike-projects.googlecode.com/svn/trunk/gBuildSystem. Downloaded it
using SVN:

$ svn checkout http://godlike-projects.googlecode.com/svn/trunk/gBuildSystem


gBuildSystem only purpose is to re-generate these makefiles in case you have
made changes in the code structure (renaming/moving/deleting/adding files) or in
the included header files (#include) or your have the external libs in different
paths. gBuildSystem requires the gen.cfg.py files (something like
CMakeLists.txt). gen.cfg.py format is pretty straightforward and minimal.

If you want to generate the makefile for the debug target (for example) do the
following:

#) $ cd <path to anki>/build/debug
#) $ <path to gBuildSystem>/gbs.py

To build:

$ make

And the build process will begin. 
  

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

Development rig: Ubuntu 10.10, AMD Radeon 4870 w/ Catalyst 10.10. So it should
be working on similar systems.


==============================================
Generating source code documentation (doxygen)
==============================================

The AnKi source code uses doxygen style comments in almost every file. To
generate the documentation you need doxygen (http://www.doxygen.org/). From a
terminal type:

#) $ cd docs
#) $ doxygen doxyfile

Then open doxygen.html to see it.
  

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
  
