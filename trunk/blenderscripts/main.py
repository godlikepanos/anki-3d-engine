print( "\n\n\n\n\n\n\n---- STARTING EXPORTER ----" )

import sys
import Blender

"""from common import *
reload( sys.modules["common"] )"""

import mesh
reload(sys.modules["mesh"])

import skeleton
reload(sys.modules["skeleton"])

from material import *
reload(sys.modules["material"])


def foo(m):
	print(m.blMesh)


objs = Blender.Object.GetSelected()

if len(objs) < 1:
	raise RuntimeError("Not selected objs")

for obj in objs:
	mi = mesh.Initializer()
	
	mi.blMesh = mesh.getBlMeshFromBlObj(obj)
	mi.blSkeleton = skeleton.GetSkeleton(obj)
	mi.saveDir = "/home/godlike/src/anki/models/imp/"
	#mtl = GetMaterial( mi.mesh )
	mi.mtlName = "models/imp/imp.mtl"
	
	
	mesh.export(mi)
	#ExportSkeleton( path, skel, cmnts )
	#ExportVWeights( path, mesh, skel, cmnts )
	#ExportMaterial( path, epath, mtl, cmnts )


print("---- ENDING EXPORTER ----")