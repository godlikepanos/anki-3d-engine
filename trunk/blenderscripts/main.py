print( "\n\n\n\n\n\n\n---- STARTING EXPORTER ----" )

import sys

from common import *
reload( sys.modules["common"] )

from mesh import *
reload( sys.modules["mesh"] )

from skeleton import *
reload( sys.modules["skeleton"] )

from material import *
reload( sys.modules["material"] )


cmnts = true
path = "/home/godlike/src/3d_engine/models/imp/"
epath = "/home/godlike/src/3d_engine/"

objs = Blender.Object.GetSelected()

if len(objs) < 1:
	ERROR( "Not selected objs" )

for obj in objs:
	mi = mesh_init_t()
	
	mi.mesh = GetMesh( obj )
	mi.skeleton = GetSkeleton( obj )
	mi.save_path = path
	mi.write_comments = true
	mtl = GetMaterial( mi.mesh )
	mi.material_filename = "models/imp/" + mtl.getName() + ".mtl"
	
	
	
	ExportMesh( mi )
	#ExportSkeleton( path, skel, cmnts )
	#ExportVWeights( path, mesh, skel, cmnts )
	#ExportMaterial( path, epath, mtl, cmnts )


print( "---- ENDING EXPORTER ----" )