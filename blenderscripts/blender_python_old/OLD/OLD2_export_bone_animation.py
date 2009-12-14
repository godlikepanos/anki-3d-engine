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
	
	g_obj = Blender.Object.GetSelected()
	g_keyframes_num = 0
	g_matrices_d = {}
	g_matrix_indeces_d = {}
	
	######################################
	########## extract the data ##########
	######################################
	if len( g_obj ) < 1:
		print "-->ERROR: Object not selected"
		return
	
	g_obj = g_obj[0]
	if g_obj.getType() != "Armature": 
		print "-->ERROR: The selected object must link to an armature and not in a(n) " + g_obj.getType()
		return
	
	if g_obj.getAction() == 0:
		print "-->ERROR: No action selected"
		return
	
	list = g_obj.getAction().getFrameNumbers()
	g_keyframes_num = list[ len(list) -1 ]
	
	
	Blender.Set( "curframe", 1 )
	for key in range( 1, g_keyframes_num+1 ):
		Blender.Set( "curframe", key )
		Blender.Redraw()
		
		pb_names = g_obj.getPose().bones.keys()
	
		pb_names.sort()
		for pb_nam in pb_names:
			pb = g_obj.getPose().bones[ pb_nam ]
				
			
			if key==1:
				g_matrices_d[pb_nam] = []
				g_matrix_indeces_d[pb_nam] = []

			index_mat = NewMatrix()
			src_m = NewMatrix()
			CopyMatrix( pb.localMatrix, src_m )
			
			i = 0
			found = 0
			for mat in g_matrices_d[pb_nam]:
				if IdenticalMatrices( mat, src_m ):
					found = 1
					break
				i = i+1
			
			if found:
				g_matrix_indeces_d[pb_nam].append( i )
			elif IdenticalMatrices( src_m, index_mat ):
				g_matrix_indeces_d[pb_nam].append( -1 )	
			else:
				g_matrices_d[pb_nam].append( src_m )
				g_matrix_indeces_d[pb_nam].append( len(g_matrices_d[pb_nam])-1 )

			# search and if you find in all the bone's matrice_indeces the -1 the bone doesnt have any animation
			# then delete the appropriate
			if key == g_keyframes_num:
				anim = 0
				for i in g_matrix_indeces_d[pb_nam]:
					if i != -1:
						anim = 1
				if not anim:
					del g_matrix_indeces_d[pb_nam]
					del g_matrices_d[pb_nam]
								
	
	###################################
	########## write to file ##########
	####################################
	path = "c:\\src\\VisualC++\\gmdl_3d_format\\models\\satan\\"			
	if not os.path.exists( path ):
		path = "c:\\data\\projects\\VisualC++\\gmdl_3d_format\\models\\satan\\"	
	filename = path + g_obj.getAction().name + ".armat_ipo.txt"
	file = open( filename, "w" )

	file.write( "BONES_NUM " + str( len(g_matrices_d) ) + "\n" )
	file.write( "KEYFRAMES_NUM " + str( g_keyframes_num ) + "\n" )

	# for every bone
	for pb_nam in g_matrices_d.keys():
		file.write( "BONE * NAME " + pb_nam + "\n" )
		file.write( "MATRICES_NUM " + str( len(g_matrices_d[pb_nam]) ) + "\n" )
		
		for mat in g_matrices_d[pb_nam]:
			file.write( "MATRIX * " )
			for i in range(0, 4):
				for j in range(0, 4):
					file.write( "%f " % mat[j][i] )
			file.write( "\n" )
	
		file.write( "MATRIX_INDECES " )
		for i in g_matrix_indeces_d[pb_nam]:
			file.write( str(i) + " " )
		file.write( "\n" )

	file.close()
	print "Done! File \"%s\" created successfuly" %filename

main()