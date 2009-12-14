import Blender
import os.path
from Blender.Mathutils import *


# MulMatrix
def MulMatrix( m, b ):
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



class bone_pose_t:
	def __init__(self):
		self.rotation = Quaternion( 1.0, 0.0, 0.0, 0.0 )
		self.translation = Vector( 0.0, 0.0, 0.0 )


class bone_anim_t:
	def __init__(self):
		self.keyframes = [] # before the "write to file" phase this array is ether empty
		                    # if the bone doesnt have animation or an array of poses
	
	
class skeleton_anim_t:
	def __init__(self):
		self.bones = []
		self.keyframes = []


########################################################################################
# main                                                                                 #
########################################################################################
def main():
	print "\n\n---- Skeletal Animation Exporter ----"
		
	# get selected
	objarr = Blender.Object.GetSelected() # returns an array of the selected objs
	if len(objarr) != 1:
		print "-->ERROR: You have to select ONE object"
		return 0
	obj = objarr[0]
	if obj.getType() != "Armature":
		print "-->ERROR: Select a skeleton and not a(n) %s" % obj.getType()
		return 0
	
	skeleton = obj.getData( 0, 0 )
	action = obj.getAction()
	
	if action == 0:
		print "-->ERROR: Empty action"
		return 0
	
	
	# init and populate the instances
	skeleton_anim = skeleton_anim_t()
	skeleton_anim.keyframes = action.getFrameNumbers()
	
	bone_names = skeleton.bones.keys()
	bone_names.sort()
	
	for i in range( 0, len(bone_names) ):
			skeleton_anim.bones.append( bone_anim_t() )
			for j in range( 0, len(skeleton_anim.keyframes) ):
				skeleton_anim.bones[i].keyframes.append( bone_pose_t() )
	
	
	# now populate with the correct data
	# for all the kframes
	for i in range( 0, len(skeleton_anim.keyframes) ):
		kframe = skeleton_anim.keyframes[i]
		Blender.Set( "curframe", kframe )
		Blender.Redraw()
		pose = obj.getPose()
		
		# for all bones
		for j in range( 0, len(bone_names) ):
			bone_name = bone_names[j]
			pose_bone = pose.bones[bone_name]
			bone = skeleton.bones[bone_name]
			
			# rotation
			rot = pose_bone.quat.toMatrix()
			rot.resize4x4()
			# translation
			tra = Matrix()
			tra.identity()
			for m in range(0, 3):
				tra[m][3] = pose_bone.loc[m]
			# bone matris at armature space aka MA
			MA = Matrix( bone.matrix["ARMATURESPACE"] )
			MAi = Matrix( MA )
			MAi.invert()
			
			# calc the m4 = MA * tra * rot * MAi
			rot.transpose()
			tra.transpose()
			MA.transpose()
			MAi.transpose()
			
			m4 = MulMatrix( rot, MAi )
			m4 = MulMatrix( tra, m4 )
			m4 = MulMatrix( MA, m4 )
			
			m4.transpose()
			
			# get the final quat and loc
			quat = m4.rotationPart()
			quat = quat.toQuat()
			loc  = m4.translationPart()
			
			quat = pose_bone.quat
			loc = pose_bone.loc
			
			for k in range(0, 4):
				skeleton_anim.bones[j].keyframes[i].rotation[k] = quat[k]
			
			for k in range(0, 3):
				skeleton_anim.bones[j].keyframes[i].translation[k] = loc[k]
	
	Blender.Set( "curframe", 1 )
	Blender.Redraw()	
	
	
	# now do the apropriate for the bones without translation or rotation
	zero_vec = Vector( 0.0, 0.0, 0.0 )
	ident_quat = Quaternion( 1.0, 0.0, 0.0, 0.0 )
	# for all the bones
	for i in range( 0, len(bone_names) ):
		no_anim_num = 0   # how many times we found that the bone has no anim
		bone = skeleton_anim.bones[i]
		# for all the keyframes
		for j in range( 0, len(skeleton_anim.keyframes) ):
			
			if bone.keyframes[j].rotation == ident_quat and bone.keyframes[j].translation == zero_vec:
				no_anim_num = no_anim_num + 1
			
		if no_anim_num == len( skeleton_anim.keyframes ):
			print "Bone \"%s\" has no animation" % bone_names[ i ]
			bone.keyframes = []
	
		
	# write the file
	filename = "/home/godlike/src/3d_engine/models/test/" + obj.getAction().name + "." + obj.getName() + ".anim"
	file = open( filename, "w" )
	
	# the keyframes
	file.write( "KEYFRAMES_NUM %d\n\tKEYFRAMES " % len(skeleton_anim.keyframes) )
	for kf in skeleton_anim.keyframes:
		file.write( "%i " % (kf - 1) )
	file.write( "\n" )
	
	# the bones num
	file.write( "BONES_NUM %d\n" % len(bone_names) )
	
	for i in range( 0, len(bone_names) ):
		bone = skeleton_anim.bones[i]
		file.write( "\tBONE %d NAME %s\n" %( i, bone_names[i] ) )
		
		if len( bone.keyframes ):
			file.write( "\t\tHAS_ANIM 1\n" )
			
			for j in range( 0, len( skeleton_anim.keyframes ) ):
				file.write( "\t\tKEYFRAME %d\n" % (skeleton_anim.keyframes[j] - 1) ) # -1 cause my keyframes start from 0
				
				# write rotation			
				file.write( "\t\t\tROTATION " )
				for k in range( 0, 4 ):
					file.write( "%f " % bone.keyframes[j].rotation[k] )
				file.write( "\n" )
				
				# write translation
				file.write( "\t\t\tTRANSLATION " )
				for k in range( 0, 3 ):
					file.write( "%f " % bone.keyframes[j].translation[k] )
				file.write( "\n" )
			
		else:
			file.write( "\t\tHAS_ANIM 0\n" )
	
	file.close()
	
	
	# done
	print "Exported! \"%s\"" % filename
	return 1

main()