import sys
import os
from copy import deepcopy
from Blender import Mathutils
from Blender.Mathutils import *
import Blender


#=======================================================================================================================
# Initializer                                                                                                          =
#=======================================================================================================================
class Initializer:
	def __init__(self):
		self.obj = None
		self.saveDir = "" # the name of the saved file
		self.flipYZ = 0 #convert from bl to right handed coord system


# MulMatrix
def MulMatrix(m, b):
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



class BonePose:
	def __init__(self):
		self.rotation = Quaternion(1.0, 0.0, 0.0, 0.0)
		self.translation = Vector(0.0, 0.0, 0.0)


class BoneAnim:
	def __init__(self):
		self.keyframes = [] # before the "write to file" phase this array is ether empty
		                    # if the bone doesnt have animation or an array of poses
	
	
class SkelAnim:
	def __init__(self):
		self.bones = []
		self.keyframes = []


#=======================================================================================================================
# getAnkiScript                                                                                                        =
#=======================================================================================================================
def getAnkiScript(obj, flipYZ, action):
	skeleton = obj.getData(0, 0) 	
	
	b_cmnts = 1
	
	# init and populate the instances
	skelAnim = SkelAnim()
	skelAnim.keyframes = action.getFrameNumbers()
	
	boneNames = deepcopy(skeleton.bones.keys())
	boneNames.sort()
	
	#skelAnim.bones = [BoneAnim() for anim in range(len(boneNames))]
	
	for i in range(0, len(boneNames)):
		skelAnim.bones.append(BoneAnim())
		for j in range(0, len(skelAnim.keyframes)):
			skelAnim.bones[i].keyframes.append(BonePose())
	
	
	# now populate with the correct data
	# for all the kframes
	for i in range(0, len(skelAnim.keyframes)):
		kframe = skelAnim.keyframes[i]
		Blender.Set("curframe", kframe)
		Blender.Redraw()
		pose = obj.getPose()
		
		# for all bones
		for j in range(0, len(boneNames)):
			boneName = boneNames[j]
			poseBone = pose.bones[boneName]
			bone = skeleton.bones[boneName]
			
			# rotation
			rot = poseBone.quat.toMatrix()
			rot.resize4x4()
			# translation
			tra = Matrix()
			tra.identity()
			for m in range(0, 3):
				tra[m][3] = poseBone.loc[m]
			# bone matris at armature space aka MA
			MA = Matrix(bone.matrix["ARMATURESPACE"])
			MAi = Matrix(MA)
			MAi.invert()
			
			# calc the m4 = MA * tra * rot * MAi
			rot.transpose()
			tra.transpose()
			MA.transpose()
			MAi.transpose()
			
			m4 = MulMatrix(rot, MAi)
			m4 = MulMatrix(tra, m4)
			m4 = MulMatrix(MA, m4)
			
			m4.transpose()
			
			# get the final quat and loc
			quat = m4.rotationPart()
			quat = quat.toQuat()
			loc  = m4.translationPart()
			
			quat = poseBone.quat
			loc = poseBone.loc
			
			for k in range(0, 4):
				skelAnim.bones[j].keyframes[i].rotation[k] = quat[k]
			
			for k in range(0, 3):
				skelAnim.bones[j].keyframes[i].translation[k] = loc[k]
	
	Blender.Set("curframe", 1)
	Blender.Redraw()	
	
	
	# now do the apropriate for the bones without translation or rotation
	zeroVec = Vector(0.0, 0.0, 0.0)
	identQuat = Quaternion(1.0, 0.0, 0.0, 0.0)
	# for all the bones
	for i in range(0, len(boneNames)):
		noAnimNum = 0   # how many times we found that the bone has no anim
		bone = skelAnim.bones[i]
		# for all the keyframes
		for j in range(0, len(skelAnim.keyframes)):
			
			if bone.keyframes[j].rotation == identQuat and bone.keyframes[j].translation == zeroVec:
				noAnimNum = noAnimNum + 1
			
		if noAnimNum == len(skelAnim.keyframes):
			print("Bone \"%s\" has no animation" % boneNames[i])
			bone.keyframes = []
	
		
	# write to ftxt
	ftxt = ""
	
	# the keyframes
	ftxt += str(len(skelAnim.keyframes)) + "\n"
	
	for kf in skelAnim.keyframes:
		ftxt += str(kf - 1) + " "
	ftxt += "\n"
	
	# the bones num
	ftxt += str(len(boneNames)) + "\n"
	
	# for all bones
	for i in range(0, len(boneNames)):
		bone = skelAnim.bones[i]
		
		if len(bone.keyframes):
			ftxt += "1\n"

			# for all keyframes			
			for j in range(0, len(skelAnim.keyframes)):				
				# write rotation			
				for k in range(0, 4):
					ftxt += str(bone.keyframes[j].rotation[k]) + " "
				ftxt += "\n"
				
				# write translation
				for k in range(0, 3):
					ftxt += str(bone.keyframes[j].translation[k]) + " "
				ftxt += "\n"
			
		else:
			ftxt += "0\n"
	
	return ftxt



#=======================================================================================================================
# export                                                                                                               =
#=======================================================================================================================
def export(init):
	obj = init.obj
	
	if obj.getType() != "Armature":
		raise RuntimeError("Select a skeleton and not a(n) " + obj.getType())

	print("Trying to export skeleton animation")
	
	action = obj.getAction()
	if action == 0:
		raise RuntimeError("Empty action for obj " + obj.getName())
	
	filename = init.saveDir + action.name + "." + obj.getName() + ".anim"	
	file = open(filename, "w")
	file.write(getAnkiScript(obj, init.flipYZ, action))
	
	print("Skeleton animation exported!! \"" + filename + "\"")	
	