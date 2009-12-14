
import Blender



#=================
# matrices funcs
#=================
def NewMatrix():
	n = range(4)	
	for i in range(0, 4):
		n[i] = range(4)
		
	for i in range(0, 4):
		for j in range(0, 4):
			if i!=j: n[i][j] = 0.0
			else:    n[i][j] = 1.0
	return n


def PrintMatrix( matrix ):
	for i in range(0, 4):
		print "[",
		for j in range(0, 4):
			f = round( matrix[i][j], 5)
			if f >= 0 :
				print " %04f" % f,
			else:
				print "%04f" % f,
			
		print "]"


def ReverseMatrix( matrix_ ):
	matrix = matrix_[:]

	for i in range(0, 4):
		for j in range(0, 4):
			if j < i:
				tmp = matrix[i][j]
				matrix[i][j] = matrix[j][i]
				matrix[j][i] = tmp
	
	return matrix


def CopyMatrix( matfrom, matto ):
	for i in range(0, 4):
		for j in range(0, 4):
			matto[i][j] = matfrom[i][j]
			

def IdenticalMatrices( m0, m1 ):
	for i in range(0, 4):
		for j in range(0, 4):
			if m0[i][j] != m1[i][j]: return 0
	return 1


#==============
# global vars
#==============
path = "c:\\src\\VisualC++\\gmdl_3d_format\\models\\"
file = 0
obj = 0
matrices = {} # create the array that holds all the unique matrices
poses = {} # this dict holds the matrix indeces for every bone. poses["bone_name"][frame-1] = matrix_index


#==============
# frames funcs
#==============
def GetFramesNum():
	global obj
	
	list = obj.getAction().getFrameNumbers()
	return list[ len(list) -1 ]


def GetFirstFrame():
	global obj
	return obj.getAction().getFrameNumbers()[0]


def MoveToFrame( num ):
	Blender.Set( "curframe", num )
	Blender.Redraw()
	

#==============
# GetData
#==============
def GetData():
	global file
	global obj
	
	print "Extracting data..."
	objs = Blender.Object.GetSelected()

	if len( objs ) < 1: 
		print "-->ERROR: No object selected"
		return 0
	
	if objs[0].getType() != "Armature": 
		print "-->ERROR: The selected object must link to an armature and not in a(n) " + objs[0].getType()
		return 0

	obj = objs[0]

	# see if it has actions
	if obj.getAction == 0:
		print "-->ERROR: No action selected"
		return 0
	
	print "Done!\n"
	return 1


#==============
# GetAnim
#==============
def GetAnim():
	global metrices
	global poses

	print "Extracting animation data..."
	crntFra = Blender.Get( "curframe" )
	
	MoveToFrame(1)
	
	pose_bone_names = obj.getPose().bones.keys()
	
	for pb_nam in pose_bone_names: # for every pose bone
		
		poses[pb_nam] = range( GetFramesNum() )
		matrices[pb_nam] = []
		
		for f in range( GetFirstFrame(), GetFramesNum()+1 ): # for every frame of that bone
			MoveToFrame(f)
			
			pb = obj.getPose().bones[ pb_nam ]
			
			found = 0
			i = -1 # set it as -1 cause in case of not exec of the below for
			for i in range( 0, len(matrices[pb_nam]) ): # for all matrices of tha bone
				if IdenticalMatrices( matrices[pb_nam][i], pb.localMatrix ) == 1:
					found = 1
					break
			
			if not found:
				ide_mat = NewMatrix()
				# if the local matrix is the identity matrix then put -1 to save space
				if IdenticalMatrices( pb.localMatrix, ide_mat ):
					i = -1
				else:
					tmp = NewMatrix()
					CopyMatrix( pb.localMatrix, tmp )
					matrices[pb_nam].append( tmp )
					i = i + 1
				
			poses[pb_nam][f-1] = i
		# } end for every frame	
		
		# now do a check
		# check if the bone has moved at all
		ide_mat = NewMatrix()
		if len(matrices[pb_nam]) == 0 :
			del poses[pb_nam]
				
		
		MoveToFrame(1)
	# end for every bone
	
	
	MoveToFrame( crntFra )
	print "Done!\n"
	return 1


#==============
# WriteFile
#==============
def WriteFile():
	global matrices
	global poses
	global path
	global obj
	
	print "Writing data to file..."
	
	# open file
	file = open( path + obj.getAction().name + ".armat_ipo.g", "w" )
	
	# write header
	file.write( "BONES_NUM " + str(len(poses)) + "\n" )
	file.write( "KEYFRAMES_NUM " + str(GetFramesNum()) + "\n" )
	
	# for every bone
	i = 0
	for pose_nam in poses.keys():
		file.write( "BONE " + str(i) + " NAME " + str(pose_nam) + "\n" )
		
		file.write( "MATRICES_NUM " + str(len(matrices[pose_nam])) + "\n" )
		
		# write matrices
		for j in range( 0, len(matrices[pose_nam]) ):
			file.write( "MATRIX " + str(j) + " " )
			ReverseMatrix( matrices[pose_nam][j] )
			for k in range(0, 4):
				for l in range(0, 4):
					file.write( str(matrices[pose_nam][j][k][l]) + " " )
			file.write( "\n" )
		
		# write matreix inteces
		file.write( "MAT_INDECES " )
		for j in range(0, GetFramesNum()):
			file.write( str(poses[pose_nam][j]) + " " )
			
		i = i + 1
		file.write("\n")
	file.close()
	
	print "Done!\n"
	return 1


#==============
# main
#==============
def main():

	print "\n--------------------------------------------------------"
	print "                     EXPORT ANIMATIONS"
	print "--------------------------------------------------------\n"
	
	if GetData():
		if GetAnim():
			WriteFile()
	
	"""print poses
	print 
	
	names = matrices.keys()
	for nam in names:
		for mat in matrices[nam]:
			PrintMatrix( mat )
			print
		break
		
	
	
	pb = obj.getPose().bones.values()[0];

	print "HEAD:  " + str(pb.head)
	print "TAIL:  " + str(pb.tail)
	print
	
	mat = NewMatrix()
	CopyMatrix( pb.localMatrix, mat )
	ReverseMatrix( mat )
	
	PrintMatrix(mat)"""
	
	print "\n--------------------------------------------------------"
		

main()