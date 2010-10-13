import sys
import os
from struct import pack
from copy import deepcopy
from Blender import Mathutils
from Blender.Mathutils import *
import Blender


#=======================================================================================================================
# multMatrix                                                                                                           =
#=======================================================================================================================
def multMatrix(m, b):
	c = Matrix()
	c[0][0] = m[0][0]*b[0][0] + m[0][1]*b[1][0] + m[0][2]*b[2][0] + m[0][3]*b[3][0]
	c[0][1] = m[0][0]*b[0][1] + m[0][1]*b[1][1] + m[0][2]*b[2][1] + m[0][3]*b[3][1]
	c[0][2] = m[0][0]*b[0][2] + m[0][1]*b[1][2] + m[0][2]*b[2][2] + m[0][3]*b[3][2]
	c[0][3] = m[0][0]*b[0][3] + m[0][1]*b[1][3] + m[0][2]*b[2][3] + m[0][3]*b[3][3]
	c[1][0] = m[1][0]*b[0][0] + m[1][1]*b[1][0] + m[1][2]*b[2][0] + m[1][3]*b[3][0]
	c[1][1] = m[1][0]*b[0][1] + m[1][1]*b[1][1] + m[1][2]*b[2][1] + m[1][3]*b[3][1]
	c[1][2] = m[1][0]*b[0][2] + m[1][1]*b[1][2] + m[1][2]*b[2][2] + m[1][3]*b[3][2]
	c[1][3] = m[1][0]*b[0][3] + m[1][1]*b[1][3] + m[1][2]*b[2][3] + m[1][3]*b[3][3]
	c[2][0] = m[2][0]*b[0][0] + m[2][1]*b[1][0] + m[2][2]*b[2][0] + m[2][3]*b[3][0]
	c[2][1] = m[2][0]*b[0][1] + m[2][1]*b[1][1] + m[2][2]*b[2][1] + m[2][3]*b[3][1]
	c[2][2] = m[2][0]*b[0][2] + m[2][1]*b[1][2] + m[2][2]*b[2][2] + m[2][3]*b[3][2]
	c[2][3] = m[2][0]*b[0][3] + m[2][1]*b[1][3] + m[2][2]*b[2][3] + m[2][3]*b[3][3]
	c[3][0] = m[3][0]*b[0][0] + m[3][1]*b[1][0] + m[3][2]*b[2][0] + m[3][3]*b[3][0]
	c[3][1] = m[3][0]*b[0][1] + m[3][1]*b[1][1] + m[3][2]*b[2][1] + m[3][3]*b[3][1]
	c[3][2] = m[3][0]*b[0][2] + m[3][1]*b[1][2] + m[3][2]*b[2][2] + m[3][3]*b[3][2]
	c[3][3] = m[3][0]*b[0][3] + m[3][1]*b[1][3] + m[3][2]*b[2][3] + m[3][3]*b[3][3]
	return c


#=======================================================================================================================
# rotMat                                                                                                               =
#=======================================================================================================================
rotMat = Matrix()
rotMat[0][0] = 1.0
rotMat[0][1] = 0.0
rotMat[0][2] = 0.0
rotMat[0][3] = 0.0
rotMat[1][0] = 0.0
rotMat[1][1] = 0.0
rotMat[1][2] = 1.0
rotMat[1][3] = 0.0
rotMat[2][0] = 0.0
rotMat[2][1] = -1.0
rotMat[2][2] = 0.0
rotMat[2][3] = 0.0
rotMat[3][0] = 0.0
rotMat[3][1] = 0.0
rotMat[3][2] = 0.0
rotMat[3][3] = 1.0
rotMat.transpose()


#=======================================================================================================================
# Initializer                                                                                                          =
#=======================================================================================================================
class Initializer:
	def __init__(self):
		self.blSkeleton = None # Blender Armature
		self.saveDir = "" # the name of the saved file
		self.flipYZ = 0 #convert from bl to right handed coord system
	


#======================================================================================================================
# getBlSkeletonFromBlObj                                                                                               =
#=======================================================================================================================
def getBlSkeletonFromBlObj(obj):	
	# check if obj is correct class
	if(obj.__class__.__name__ != "Blender Object"):
		raise RuntimeError("The given func param is not a \"Blender Object\" class but a \"" + obj.__class__.__name__ + "\"")
	
	# check modifiers
	if len(obj.modifiers) < 1:
		raise RuntimeError("Obj \"" + obj.getName() + "\" doesnt have modifiers so no armature found")
	
	# search for modifier of skeleton type
	for mod in obj.modifiers:
		if mod.type == Blender.Modifier.Types.ARMATURE:
			aobj = mod[Blender.Modifier.Settings.OBJECT]
			skeleton = Blender.Object.Get(aobj.name).getData(0, 1) # set skeleton
			return skeleton
	
	raise RuntimeError("Obj \"" + obj.getName() + "\" has no modifier of type armature")
		


#=======================================================================================================================
# getAnkiSkeletonScript                                                                                                =
#=======================================================================================================================
def getAnkiSkeletonScript(skeleton, flipYZ):	
	"""ftxt = "" # file text
	
	# write the file
	boneNames = skeleton.bones.keys()
	boneNames.sort() # the bones are written in alpabetical order

	ftxt += str(len(boneNames)) + "\n"

	for boneName in boneNames:	
		bone = skeleton.bones[boneName]
		
		# name
		ftxt += "\"" + bone.name + "\"\n"
		
		# head
		co = bone.head["ARMATURESPACE"]
		if flipYZ:
			ftxt += str(co.x) + " " + str(co.z) + " " + str(-co.y) + "\n"
		else:
			ftxt += str(co.x) + " " + str(co.y) + " " + str(co.z) + "\n"
		
		# tail
		co = bone.tail["ARMATURESPACE"]
		if flipYZ:
			ftxt += str(co.x) + " " + str(co.z) + " " + str(-co.y) + "\n"
		else:
			ftxt += str(co.x) + " " + str(co.y) + " " + str(co.z) + "\n"
		
		# matrix
		m4 = bone.matrix["ARMATURESPACE"].copy()
		
		if flipYZ:
			m4 = multMatrix(m4, rotMat)
		
		for i_ in range(0, 4):
			for j_ in range(0, 4):
				ftxt += str(m4[j_][i_]) + " "
		ftxt += "\n"
		
		# write the parent
		if not bone.parent:
			ftxt += "NULL\n"
		else:
			ftxt += str(boneNames.index(bone.parent.name)) + "\n"					

			
		# write the childs
		ftxt += str(len(bone.children)) + "\n"
		
		for child in bone.children:
			ftxt += str(boneNames.index(child.name)) + " "
		
		ftxt += "\n"	
		
	return ftxt"""

	buff = pack("8s", "ANKISKEL")
	
	boneNames = skeleton.bones.keys()
	boneNames.sort() # the bones are written in alpabetical order
	
	buff += pack("I", len(boneNames))
	
	for boneName in boneNames:	
		bone = skeleton.bones[boneName]
		
		# name
		str_ = bone.name
		buff += pack("I" + str(len(str_)) + "s", len(str_), str_)
		
		# head
		co = bone.head["ARMATURESPACE"]
		if flipYZ:
			buff += pack("fff", co.x, co.z, -co.y)
		else:
			buff += pack("fff", co.x, co.y, co.z)
		
		# tail
		co = bone.tail["ARMATURESPACE"]
		if flipYZ:
			buff += pack("fff", co.x, co.z, -co.y)
		else:
			buff += pack("fff", co.x, co.y, co.z)
	
		# matrix
		m4 = bone.matrix["ARMATURESPACE"].copy()
		
		if flipYZ:
			m4 = multMatrix(m4, rotMat)
			
		for i_ in range(0, 4):
			for j_ in range(0, 4):
				buff += pack("f", m4[j_][i_])
				
		# write the parent
		if not bone.parent:
			buff += pack("I", 0xFFFFFFFF)
		else:
			buff += pack("I", boneNames.index(bone.parent.name))
			
		# write the childs
		buff += pack("I", len(bone.children))
		
		for child in bone.children:
			buff += pack("I", boneNames.index(child.name))
			
	# end!
	return buff
		
	
#=======================================================================================================================
# export                                                                                                               =
#=======================================================================================================================
def export(skeletonInit):
	skeleton = skeletonInit.skeleton
	#check if mesh is the correct class
	if(skeleton.__class__.__name__ != "Armature"):
		raise RuntimeError("The given func param is not a \"Armature\" class but a \"" + skeleton.__class__.__name__ + "\"")
	
	print("Trying to export skeleton \"" + skeleton.name + "\"")
	
	filename = skeletonInit.saveDir + skeleton.name + ".skel"
	file = open(filename, "wb")
	file.write(getAnkiSkeletonScript(skeleton, skeletonInit.flipYZ))
	
	print("Skeleton exported!! \"" + filename + "\"")	
	
