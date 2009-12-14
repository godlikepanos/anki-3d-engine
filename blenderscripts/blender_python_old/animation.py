import Blender
import os.path
from Blender.Mathutils import *


######################
#### matrix funcs ####
######################
def PrintMatrix( matrix ):
	for i in range(0, 4):
		print "[",
		for j in range(0, 4):
			f = matrix[i][j]
			if f >= 0 :
				print " %04f" % f,
			else:
				print "%04f" % f,
		print "]"
		
		
def MulMatrix( m, b ):
	c = NewMatrix()
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
	
		
def NewMatrix():
	m4 = Matrix( [1., 0., 0., 0.], [0., 1., 0., 0.], [0., 0., 1., 0.], [0., 0., 0., 1.] )
	return m4


def CpyMatrix( matfrom, matto ):
	for i in range(0, 4):
		for j in range(0, 4):
			matto[i][j] = round( matfrom[i][j], 5 )


def CpyQuat( qfrom, qto ):
	for i in range(0, 4):
		qto[i] = qfrom[i]
		
		
def CpyVec( vfrom, vto ):
	for i in range(0, 3):
		vto[i] = vfrom[i]


identity_mat = NewMatrix()


#################
#### classes ####
#################
class pose_t:
	def __init__(self):
		self.keyframe = 0
		self.matrix = NewMatrix()
		self.quat = Quaternion()
		self.loc  = Vector( 0.0, 0.0, 0.0 )
		self.scale = Vector( 0.0, 0.0, 0.0 )

class bone_ipo_t:
	def __init__(self):
		self.poses = []


###############
#### main  ####
###############
def main():
	
	print "\n\n---- Exporting Bone IPO ----"
	
	obj = Blender.Object.GetSelected()
	keyframes_num = 0
	bone_ipos = {}

	######################################
	########## extract the data ##########
	######################################
	if len( obj ) < 1:
		print "-->ERROR: Object not selected"
		return
	
	obj = obj[0]
	if obj.getType() != "Armature": 
		print "-->ERROR: The selected object must link to an armature and not in a(n) " + obj.getType()
		return
	
	armat = obj.getData(0,1)
	
	if obj.getAction() == 0:
		print "-->ERROR: No action selected"
		return
	
	kframes = obj.getAction().getFrameNumbers()
	# for every nodal keyframe aka keyframe
	for kframe in kframes:
		Blender.Set( "curframe", kframe )
		Blender.Redraw()
		
		#determin if it is the first or the last frame
		first_kframe = 0
		last_kframe = 0
		if kframe == obj.getAction().getFrameNumbers()[0]: first_kframe = 1
		if kframe == obj.getAction().getFrameNumbers()[ len(obj.getAction().getFrameNumbers())-1 ]: last_kframe = 1
		
		# for all bones
		pb_names = obj.getPose().bones.keys()
		for pb_nam in pb_names:
			pb = obj.getPose().bones[ pb_nam ]
			pose = pose_t()
				
			if first_kframe:
				bone_ipos[pb_nam] = bone_ipo_t()
		
			############
			#  matrix  #
			############
			CpyMatrix( pb.localMatrix, pose.matrix )
			
			
			############
			#  kframe  #
			############
			pose.keyframe = kframe


			################
			#  quaternion  #
			################
			m4rot = pb.quat.toMatrix() # the quat to mat3...
			m4rot.resize4x4()          # and now to mat4
			ma = NewMatrix()           # bone's matrix in armature space
			mai = NewMatrix()          # and the inverted
			# set ma and mai
			CpyMatrix( armat.bones[pb_nam].matrix["ARMATURESPACE"], ma )
			CpyMatrix( armat.bones[pb_nam].matrix["ARMATURESPACE"], mai )
			mai.invert()
			# calc the rotation in armature space cause its in bone space
			# use my mul matrix cause blenders produces wrong results
			ma.transpose()
			m4rot.transpose()
			mai.transpose()
			m4total = MulMatrix( ma, m4rot)
			m4total = MulMatrix( m4total, mai) 
			# now convert total m4 to quaternion
			m3tmp = Matrix( [1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0] )
			for ii in range( 0, 3 ):
				for jj in range( 0, 3 ):
					m3tmp[ii][jj] = m4total[ii][jj] # the matrix is transposed
			m3tmp.transpose()
			qtotal = m3tmp.toQuat()
			
			CpyQuat( qtotal, pose.quat )
	
	
			##############
			#  location  #
			##############
			# the pose bone gives the loc in bones space thats why we have to convert it...
			# to armatur space. This means that...
			# new_loc = ToVec( bone.mat_armatspace * ToMat(posebone.loc) * bone.mat_armatspace.inverted() ) 
			ma = NewMatrix()
			mai = NewMatrix()
			m4tmp = NewMatrix()
			vectotal = Vector(0.0, 0.0, 0.0)
			# init them
			CpyMatrix( armat.bones[pb_nam].matrix["ARMATURESPACE"], ma )
			CpyMatrix( ma, mai )
			mai.invert()
			# load the pose's loc to a mat4
			m4tmp[0][3] = pb.loc[0]
			m4tmp[1][3] = pb.loc[1]
			m4tmp[2][3] = pb.loc[2]
			# now calc the m4tmp = ma * m4tmp * mai
			ma.transpose()
			mai.transpose()
			m4tmp = MulMatrix( ma, m4tmp )
			m4tmp = MulMatrix( m4tmp, mai )
			# now create the total vec3
			vectotal[0] = m4tmp[0][3]
			vectotal[1] = m4tmp[1][3]
			vectotal[2] = m4tmp[2][3]
			CpyVec( vectotal, pose.loc )
			
			
			# now put them
			bone_ipos[ pb_nam ].poses.append( pose )
			
			# we want to check the 2 previous matrices
			# and if the middle matrice is the same withe the crnt and the
			# pre-previous we delete the pose
			"""i = len( bone_ipos[pb_nam].poses )-1
			if i-2 >= 0:
				prev = bone_ipos[pb_nam].poses[i-1].matrix
				pprev = bone_ipos[pb_nam].poses[i-2].matrix
				crnt = pose.matrix
				if prev == pprev and prev == crnt:
					#print "I will delete the %d keyframe from %s " % ( kframes[i-1], pb_nam )
					del bone_ipos[pb_nam].poses[i-1]"""
					

	
	###################################
	########## write to file ##########
	####################################
	path = "c:\\src\\VisualC++\\gmdl_3d_format\\models\\satan\\"			
	if not os.path.exists( path ):
		path = "c:\\data\\projects\\VisualC++\\gmdl_3d_format\\models\\satan\\"	
	filename =  path + obj.getAction().name + ".animation.txt"
	file = open( filename, "w" )

	file.write( "FRAMES_NUM %d\n" % kframes[-1] )
	
	file.write( "KEYFRAMES_NUM %d KEYFRAMES " %len(kframes) )
	for kf in kframes:
		file.write( "%d " %(kf-1) )
	file.write("\n")
		
	
	file.write( "POSEBONES_NUM %d\n" % len(bone_ipos) )

	bi = 0
	names = bone_ipos.keys()
	names.sort()
	for nam in names:
		ipo = bone_ipos[ nam ]
		file.write( "\tBONE %d NAME %s " % (bi, nam) )
		file.write( "POSES_NUM %d\n" % (len(ipo.poses)) )
		
		p = 0
		for pose in ipo.poses:
			file.write( "\t\tPOSE %d\n" % p )			
			file.write( "\t\t\tKEYFRAME %d\n" % (pose.keyframe-1) )			
						
			mat = pose.matrix
			file.write( "\t\t\tMATRIX " )
			for i in range(0, 4):
				for j in range(0, 4):
					file.write( "%f " % mat[j][i] )
			file.write( "\n" )
			p = p+1
			
			file.write( "\t\t\tLOCATION " )
			for i in range(0, 3):
				file.write( "%f " % pose.loc[i] )
			file.write( "\n" )
			
			file.write( "\t\t\tQUATERNION " )
			for i in range(0, 4):
				file.write( "%f " % pose.quat[i] )
			file.write( "\n" )
		
		bi = bi+1		

	file.close()
	print "Done! File \"%s\" created successfuly" %filename

main()