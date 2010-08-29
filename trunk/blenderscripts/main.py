print( "\n\n\n\n\n\n\n---- STARTING EXPORTER ----" )

import sys
import Blender

import mesh
reload(sys.modules["mesh"])

import skeleton
reload(sys.modules["skeleton"])

from material import *
reload(sys.modules["material"])

import skelanim
reload(sys.modules["skelanim"])


objs = Blender.Object.GetSelected()

if len(objs) < 1:
	raise RuntimeError("Not selected objs")

for obj in objs:
	mi = mesh.Initializer()
	mi.blMesh = mesh.getBlMeshFromBlObj(obj)
	mi.blSkeleton = skeleton.getBlSkeletonFromBlObj(obj)
	mi.saveDir = "/home/godlike/src/anki/models/imp/"
	#mtl = GetMaterial( mi.mesh )
	mi.mtlName = "models/imp/imp.mtl"
	mi.flipYZ = 1
	mesh.export(mi)
	
	si = skeleton.Initializer()
	si.skeleton = skeleton.getBlSkeletonFromBlObj(obj)
	si.saveDir = "/home/godlike/src/anki/models/imp/"
	si.flipYZ = 1
	skeleton.export(si)
	
	"""ai = skelanim.Initializer()
	ai.obj = obj
	ai.saveDir = "/home/godlike/src/anki/models/imp/"
	ai.flipYZ = 1
	skelanim.export(ai)"""


print("---- ENDING EXPORTER ----")