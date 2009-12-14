import Blender
import string
import os.path

################
# MATRIX FUNCS #
################
def MatrixMul( matrix, vector ):
	vec = [0, 0, 0]
	
	for i in range(0, 3):
		for j in range(0, 3):
			vec[i] = vec[i] + matrix[j][i] * vector[j]
		vec[i] = vec[i] + matrix[3][i]
		
	for i in range(0, 3):
		vector[i] = vec[i]
	
		
def PrintMatrix( matrix ):
	for i in range(0, 4):
		print "[",
		for j in range(0, 4):
			f = round( matrix[j][i], 5)
			if f >= 0 :
				print " %04f" % f,
			else:
				print "%04f" % f,
		print "]"

		
########
# main #
########
def main():
	
	print "\n---- Export Mesh ----"
	
	########
	# vars #
	########
	file = 0
	mesh = 0	
	matrix = 0
	path = "c:\\src\\VisualC++\\gmdl_3d_format\\models\\satan\\"
	
	
	######################
	# init and open file #
	######################	
	objs = Blender.Object.GetSelected()

	if len( objs ) < 1: 
		print "-->ERROR: No object selected"
		return 0
	
	if objs[0].getType() != "Mesh": 
		print "-->ERROR: The selected object must link to a mesh and not in a(n) " + objs[0].getType()
		return 0

	mesh = objs[0].getData( 0, 1 )
	
	if not mesh.faceUV:
		print "-->ERROR: The mesh doesnt have UVs"
		return 0

	matrix = objs[0].getMatrix( "localspace" )
	
	filename = path + "mesh.txt"
	file = open( filename, "w" )
		
		
	#########
	# verts #
	#########
	file.write( "VERTS_NUM %d\n" % len(mesh.verts) )
	for vert in mesh.verts:
		coords = [0.0, 0.0, 0.0]
		normal = [0.0, 0.0, 0.0]
		
		coords[0] = vert.co.x
		coords[1] = vert.co.y
		coords[2] = vert.co.z
		normal[0] = vert.no.x
		normal[1] = vert.no.y
		normal[2] = vert.no.z
		
		MatrixMul( matrix, coords )
		MatrixMul( matrix, normal )
		
		file.write( "\tVERT %d COORDS %f %f %f\n" %(vert.index, coords[0], coords[1], coords[2]) )
		#file.write( " NORMAL %f %f %f\n" %(normal[0], normal[1], normal[2]) )
	
	
	#########
	# faces #
	#########
	#first count the faces as triangles
	tris_num = 0
	for face in mesh.faces:
		if len(face.v) == 3:
			tris_num = tris_num + 1
		else:
			tris_num = tris_num + 2
	
	file.write( "FACE_NUM %d\n" % tris_num )
	
	# for every face
	i = 0
	for face in mesh.faces:
			
		file.write( "\tFACE %d VERT_INDECES " % i )
		
		order = [0,1,2]
		# print index
		for j in order:
			file.write( "%d " % face.v[j].index )
		
		file.write( "\n" )
		i = i+1
		
		# if the face is quad then triangulate
		if( len( face.v ) == 4 ):
			order = [0,2,3]
			file.write( "\tFACE %d VERT_INDECES " % i )
			for j in order:
				file.write( "%d " % face.v[j].index )
			file.write( "\n" )	
			i = i+1
	
			
	#######
	# uvs #
	#######
	file.write( "UVS_NUM %d\n" % tris_num )
	
	i = 0
	for face in mesh.faces:
			
		order = [0,1,2]
		file.write( "\tUV %d " % i )
		for j in order:
			file.write( "%f %f " % (face.uv[j].x, face.uv[j].y) )
		
		file.write( "\n" )
		i = i+1
		
		# if the face is quad then triangulate
		if( len( face.v ) == 4 ):
			order = [0,2,3]
			file.write( "\tUV %d " % i )
			for j in order:
				file.write( "%f %f " % (face.uv[j].x, face.uv[j].y) )
			file.write( "\n" )	
			i = i+1
		
		
	########
	# DONE #
	########
	print "Done! File \"%s\" created" % filename
	file.close()
main()