import Blender
import os.path


######################
#### matrix funcs ####
######################
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


identity_mat = NewMatrix()


#################
#### classes ####
#################
class obj_trans_ipo_t:
	def __init__(self):
		self.keyframes = []
		self.matrices = []
		


###############
#### main  ####
###############
def main():
	
	print "\n\n---- Exporting Bone IPO ----"
	
	obj = Blender.Object.GetSelected()
	keyframes_num = 0
	trans_ipos = {}

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
	
	
	Blender.Set( "curframe", 1 )
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
		pb_names.sort()
		for pb_nam in pb_names:
			pb = obj.getPose().bones[ pb_nam ]
				
			if first_kframe:
				trans_ipos[pb_nam] = obj_trans_ipo_t()
		
			src_m = NewMatrix()
			CopyMatrix( pb.localMatrix, src_m )
		
			trans_ipos[pb_nam].keyframes.append( kframe )			
			trans_ipos[pb_nam].matrices.append( src_m )	

	
	###################################
	########## write to file ##########
	####################################
	path = "c:\\src\\VisualC++\\gmdl_3d_format\\models\\satan\\"			
	if not os.path.exists( path ):
		path = "c:\\data\\projects\\VisualC++\\gmdl_3d_format\\models\\satan\\"	
	filename =  path + obj.getAction().name + ".armat_ipo.txt"
	file = open( filename, "w" )

	file.write( "BONES_NUM %d\n" % len(trans_ipos) )
	
	file.write( "KEYFRAMES_NUM %d\n" % len(kframes) )
	file.write( "KEYFRAMES " )
	for kf in kframes:
		file.write( "%d " % kf )
	file.write( "\n" )

	l = 0
	names = trans_ipos.keys()
	names.sort()
	for nam in names:
		ipo = trans_ipos[ nam ]
		file.write( "BONE %d NAME %s\n" % (l, nam) )
		
		k = 0
		for mat in ipo.matrices:
			file.write( "MATRIX %d " % k )
			for i in range(0, 4):
				for j in range(0, 4):
					file.write( "%f " % mat[j][i] )
			k = k+1
			file.write( "\n" )
		l = l+1

	file.close()
	print "Done! File \"%s\" created successfuly" %filename

main()