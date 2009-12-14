print( "\n\n\n\n\n\n\n---- STARTING EXPORTER ----" )

import sys

from common import *
reload( sys.modules["common"] )

from mesh import *
reload( sys.modules["mesh"] )

from skeleton import *
reload( sys.modules["skeleton"] )

from vweights import *
reload( sys.modules["vweights"] )

from material import *
reload( sys.modules["material"] )


cmnts = true
path = "/home/godlike/src/3d_engine/models/imp/"
epath = "/home/godlike/src/3d_engine/"

objs = Blender.Object.GetSelected()

if len(objs) < 1:
	ERROR( "Not selected objs" )

for obj in objs:
	mesh = GetMesh( obj )
	mtl = GetMaterial( mesh )
	skel = GetSkeleton( obj )
	
	ExportMesh( path, mesh, skel, "models/imp/" + mtl.getName() + ".mtl" , cmnts )
	ExportSkeleton( path, skel, cmnts )
	#ExportVWeights( path, mesh, skel, cmnts )
	#ExportMaterial( path, epath, mtl, cmnts )


print( "---- ENDING EXPORTER ----" )