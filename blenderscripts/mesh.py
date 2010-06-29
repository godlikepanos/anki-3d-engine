import sys
import common
import os
from copy import copy
from Blender import Mathutils
from Blender.Mathutils import *
from Numeric import *


#=======================================================================================================================
# Initializer                                                                                                      =
#=======================================================================================================================
class Initializer:
	def __init__(self):
		self.blMesh = None  # Blender Mesh
		self.blSkeleton = None # Blender Armature
		self.mtlName = "" # Material name
		self.saveDir = "" # the name of the saved file
	

#=======================================================================================================================
# Vert                                                                                                                 =
#=======================================================================================================================
class Vert:
	def __init__(self):
		self.x = 0.0
		self.y = 0.0
		self.z = 0.0
		self.s = 0.0
		self.t = 0.0
		self.nextId = -1 # shows the next vertId. Is != -1 if the vert is problematic


#=======================================================================================================================
# Tri                                                                                                                  =
#=======================================================================================================================
class Tri:
	def __init__(self):
		self.vertIds = [-1, -1, -1]


#=======================================================================================================================
# getBlMeshFromBlObj                                                                                                   =
#=======================================================================================================================
def getBlMeshFromBlObj(obj):
	if(obj.__class__.__name__ != "Blender Object"):
		raise RuntimeError("The given func param is not a \"Blender Object\" class but a \"" + obj.__class__.__name__ + "\"")
	
	if obj.getType() != "Mesh": 
		raise RuntimeError("The obj \"" + obj.getName() + "\" must link to a mesh and not to a(n) " + obj.getType())

	mesh = obj.getData(0, 1) 
	return mesh


#=======================================================================================================================
# getAnkiVertWeights                                                                                                   =
#=======================================================================================================================
def getAnkiVertWeights(mesh, skeleton):
	boneNames = skeleton.bones.keys()
	boneNames.sort()
	
	# init text
	ftxt = ""
	
	# link the vert groups to the bone ids
	vgroup2boneId = {}   # we give the vgroup name and we get the bone's id in the skeleton
	vgroupNames = mesh.getVertGroupNames()
	
	for vgroupName in vgroupNames:
		boneId = -1
		for boneName in boneNames:
			if boneName == vgroupName:
				boneId = boneNames.index(boneName)
				break
		
		if boneId == -1:
			print("Vert group \"" + vgroupName + "\" cant link to a bone")
		
		vgroup2boneId[vgroupName] = boneId


	# vert weights num
	ftxt += str(len(mesh.verts)) + "\n"
	
	# for every vert do some shit
	for vert in mesh.verts:
		influences = mesh.getVertexInfluences(vert.index)
		
		influencesNum = 0
		sumw = 0.0
		# calc the influences num and the total weight (NOTE:we may have a vert group that doesnt connect to a bone)
		for influence in influences:
			vgroup = influence[0]
			weight = influence[1]
			
			if vgroup2boneId[vgroup] != -1:
				influencesNum = influencesNum + 1
				sumw = sumw + weight
		
		# a check
		if(influencesNum > 4):
			raise RuntimeError("Cannot have more than 4 bones per vert")
	
		# write influences num
		ftxt += str(influencesNum) + "\n"
				
		for influence in influences:
			vgroup = influence[0]
			weight = influence[1]
			
			if vgroup2boneId[vgroup] != -1:	
				# write bone id
				ftxt += str(vgroup2boneId[vgroup]) + " "
				# write weight for that bone
				ftxt += str(weight/sumw) + "\n"
	# end for all verts
	
	return ftxt
	

#=======================================================================================================================
# getAnkiMeshScript                                                                                                    =
#=======================================================================================================================
def	getAnkiMeshScript(mesh, skeleton, mtlName):
	# check verts number
	vertsNum = len(mesh.verts)
	if vertsNum < 3:
		raise RuntimeError("The mesh named \"" + mesh.name + "\" has insufficient vert num")
	
	# declare some vars
	ankiVerts = {}
	ankiTris = [Tri() for tri in range(len(mesh.faces))]
	hasUvs = 0


	# if it has UVs	
	if mesh.faceUV:
		hasUvs = 1
		
		# for all faces
		for i in range(len(mesh.faces)):
			face = mesh.faces[i]
			if len(face.verts) != 3:
				raise RuntimeError("Only triangles are accepted")
			
			ankiTris[i] = Tri() # create new tri
			
			# for verts in the face
			for j in [0, 1, 2]:
				vertId = face.verts[j].index
			
				# vert does not exist
				if not ankiVerts.has_key(vertId):
					ankiVerts[vertId] = Vert()
					ankiVerts[vertId].x = mesh.verts[vertId].co.x
					ankiVerts[vertId].y = mesh.verts[vertId].co.y
					ankiVerts[vertId].z = mesh.verts[vertId].co.z
					ankiVerts[vertId].s = face.uv[j].x
					ankiVerts[vertId].t = face.uv[j].y
					ankiVerts[vertId].nextId = -1
				
					ankiTris[i].vertIds[j] = vertId
				else:
					prevId = -1
					while 1:
						# if in end of the list, create new vert and link list
						if vertId == -1:
							ankiVerts[vertsNum] = copy.copy(ankiVerts[prevId])
							ankiVerts[vertsNum].s = face.uv[j].x
							ankiVerts[vertsNum].t = face.uv[j].y
							ankiVerts[vertsNum].nextId = -1
	
							ankiVerts[prevId].nextId = vertsNum
						
							ankiTris[i].vertIds[j] = vertsNum
						
							vertsNum = vertsNum + 1
							break
						# a vert with the same UVs exists
						elif ankiVerts[vertId].s == face.uv[j].x and ankiVerts[vertId].t == face.uv[j].y:
							ankiTris[i].vertIds[j] = vertId
							break;
						# move to next node
						else:
							prevId = vertId
							vertId = ankiVerts[vertId].nextId
	# no UVs
	else:
		hasUvs = 0
		
		# set the verts
		for i in range(len(mesh.verts)):
			vert = mesh.verts[i]
			
			ankiVerts[i] = Vert()
			ankiVerts[i].x = vert.co.x
			ankiVerts[i].y = vert.co.y
			ankiVerts[i].z = vert.co.z
		
		# set the faces
		for i in range(len(mesh.faces)):
			face = mesh.faces[i]
			
			if len(face.verts) != 3:
				raise RuntimeError("Only triangles are accepted")
		
			for j in [0, 1, 2]:
				ankiTris[i].vertIds[j] = face.verts[j].index
		
		
	# write to ftxt
	
	# write mtl name
	ftxt = "\"" + mtlName + "\"\n"
	
	# write verts
	ftxt += str(len(ankiVerts)) + "\n"
	for i in range(len(ankiVerts)):
		ankiVert = ankiVerts[i]
		ftxt += str(ankiVert.x) + " " + str(ankiVert.y) + " " + str(ankiVert.z) + "\n"
		
	# write the tris
	ftxt += str(len(ankiTris)) + "\n"
	for ankiTri in ankiTris:
		ftxt += str(ankiTri.vertIds[0]) + " " + str(ankiTri.vertIds[1]) + " " + str(ankiTri.vertIds[2]) + "\n"
		
	# write the UVs
	if hasUvs:
		ftxt += str(len(ankiVerts)) + "\n"
		for i in range(len(ankiVerts)):
			ftxt += str(ankiVerts[i].s) + " " + str(ankiVerts[i].t) + "\n"
	else:
		ftxt += "0\n"
	
	# write the vert weights
	if skeleton != None:
		ftxt += getAnkiVertWeights(mesh, skeleton)
	else:
		ftxt += "0\n"

	return ftxt
			
		
#=======================================================================================================================
# export                                                                                                               =
#=======================================================================================================================
def export(meshInit):
	mesh = meshInit.blMesh
	skeleton = meshInit.blSkeleton
	
	#check if mesh is the correct class
	if mesh.__class__.__name__ != "Blender Mesh":
		raise RuntimeError("The given func param is not a \"Blender Mesh\" class but a \"" + mesh.__class__.__name__ + "\"")
	
	if skeleton != None:
		#check if skeleton is the correct class
		if(skeleton.__class__.__name__ != "Armature"):
			raise RuntimeError("The given func param is not a \"Armature\" class but a \"" + skeleton.__class__.__name__ + "\"")
	
	print("Trying to export mesh \"" + mesh.name + "\"")
	filename = os.path.abspath(meshInit.saveDir + mesh.name + ".mesh")
	common.WriteFile(filename, getAnkiMeshScript(mesh, skeleton, meshInit.mtlName))
	print("Mesh exported!! \"" + filename + "\"")	