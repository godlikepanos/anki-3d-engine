import Blender
import os.path


##################
#  matrix funcs  #
##################
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

		
def NewMatrix():
	n = range(4)	
	for i in range(0, 4):
		n[i] = range(4)		
	for i in range(0, 4):
		for j in range(0, 4):
			if i!=j: n[i][j] = 0.0
			else:    n[i][j] = 1.0
	return n


def CopyMatrix( matfrom, matto ):
	for i in range(0, 4):
		for j in range(0, 4):
			matto[i][j] = round( matfrom[i][j], 5 )
			

def IdenticalMatrices( m0, m1 ):
	for i in range(0, 4):
		for j in range(0, 4):
			a = round(m0[i][j], 5)
			b = round(m1[i][j], 5) 
			if a != b: return 0
	return 1

##########
#  main  #
##########
def main():
	
	print "\n---- Exporting Bone IPO ----"
	
	obj = Blender.Object.GetSelected()
	frames_num = 0
	matrices = []
	matrix_indeces = {}
	index_mat = NewMatrix()
	
	matrices.append( index_mat )  # the first matrix is allways the indentity mat
	
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
	
	if obj.getAction() == 0:
		print "-->ERROR: No action selected"
		return
	
	list = obj.getAction().getFrameNumbers()
	frames_num = list[ len(list) -1 ]
	
	
	Blender.Set( "curframe", 1 )
	for frame in range( 1, frames_num+1 ):
		Blender.Set( "curframe", frame )
		Blender.Redraw()
		
		pb_names = obj.getPose().bones.keys()
	
		pb_names.sort()
		for pb_nam in pb_names:
			pb = obj.getPose().bones[ pb_nam ]
				
			
			if frame==1:
				matrix_indeces[pb_nam] = []

			src_m = NewMatrix()
			CopyMatrix( pb.localMatrix, src_m )
			
			i = 0
			found = 0
			for mat in matrices:
				if IdenticalMatrices( mat, src_m ):
					found = 1
					break
				i = i+1
			
			if found:
				matrix_indeces[pb_nam].append( i )
			#elif IdenticalMatrices( src_m, index_mat ):
				#matrix_indeces[pb_nam].append( -1 )	
			else:
				matrices.append( src_m )
				matrix_indeces[pb_nam].append( len(matrices)-1 )
												
	
	###################################
	########## write to file ##########
	####################################
	path = "c:\\src\\VisualC++\\gmdl_3d_format\\models\\satan\\"			
	if not os.path.exists( path ):
		path = "c:\\data\\projects\\VisualC++\\gmdl_3d_format\\models\\satan\\"	
	filename = path + obj.getAction().name + ".armat_ipo.txt"
	file = open( filename, "w" )

	file.write( "BONES_NUM %d\n" % len(matrix_indeces) )
	file.write( "FRAMES_NUM %d\n" % frames_num )
	file.write( "MATRICES_NUM %d\n" % len(matrices) )

	# matrices
	k = 0
	for mat in matrices:
		file.write( "MATRIX %d " % k )
		for i in range(0, 4):
			for j in range(0, 4):
				file.write( "%f " % mat[j][i] )
		file.write( "\n" )
		k = k+1
		
		
	# for every bone
	k = 0
	pb_names = matrix_indeces.keys()
	pb_names.sort()
	for pb_nam in pb_names:
		file.write( "BONE %d NAME %s\n" % (k, pb_nam) )
		
		file.write( "MATRIX_INDECES " )
		for i in matrix_indeces[pb_nam]:
			file.write( "%s " % i )
		file.write( "\n" )
		k = k+1

	file.close()
	print "Done! File \"%s\" created successfuly" %filename

main()