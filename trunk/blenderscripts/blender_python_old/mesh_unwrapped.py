import Blender
import string
import os.path
from Numeric import *

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


##########
# IsZero #
##########
def IsZero( f ):
	return ( fabs(f) > 1.0e-6 )

		
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
	path = "/home/godlike/src/3d_engine/meshes/imp/"
	
	
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
			
		file.write( "\tFACE %d VERT_IDS " % i )
		
		order = [0,1,2]
		# print index
		for j in order:
			file.write( "%d " % face.v[j].index )
		
		file.write( "\n" )
		i = i+1
		
		# if the face is quad then triangulate
		if( len( face.v ) == 4 ):
			order = [0,2,3]
			file.write( "\tFACE %d VERT_IDS " % i )
			for j in order:
				file.write( "%d " % face.v[j].index )
			file.write( "\n" )	
			i = i+1
	

	#######
	# uvs #
	#######	
	
	# create and populate tha vertuvs array
	vertuvs = {}
	for vert in mesh.verts:
		vertuvs[ vert.index ] = [ -1.0, -1.0 ]
	
	
	for face in mesh.faces:
			
		order = [0,1,2] # verts order
		
		for j in order:
			vert_id = face.verts[j].index
			
			uvx = face.uv[j].x
			uvy = face.uv[j].y
			
			# if we put a new value in the array OR the values already set and they are the same then
			if vertuvs[ vert_id ][0]==-1.0 or ( vertuvs[ vert_id ][0] == uvx and vertuvs[ vert_id ][1] == uvy ):
				vertuvs[ vert_id ][0] = uvx
				vertuvs[ vert_id ][1] = uvy
			else:
				print "-->ERROR: More than 1 coords for the %d vert" % vert_id
				#print " %f %f %f %f" % ( vertuvs[ vert_id ][0], vertuvs[ vert_id ][1], uvx, uvy )
			
		# do the same if quat for the other forming triangle
		if( len( face.verts ) == 4 ):
			order = [0,2,3]
			
			for j in order:
				vert_id = face.verts[j].index
				
				uvx = face.uv[j].x
				uvy = face.uv[j].y
				
				# if we put a new value in the array OR the values already set and they are the same then
				if vertuvs[ vert_id ][0]==-1.0 or ( vertuvs[ vert_id ][0] == uvx and vertuvs[ vert_id ][1] == uvy ):
					vertuvs[ vert_id ][0] = uvx
					vertuvs[ vert_id ][1] = uvy
				else:
					print "-->ERROR: More than 1 coords for the %d vert" % vert_id
					#print " %f %f %f %f" % ( vertuvs[ vert_id ][0], vertuvs[ vert_id ][1], uvx, uvy )
	
	# one last check
	if( len( vertuvs ) != len( mesh.verts ) ):
		print "-->ERROR: There is a difference of verts num and verts uvs"
			
	# now write the uvs to file
	file.write( "UVS_NUM %d\n" % len(vertuvs) )
	
	for i in range( 0, len(vertuvs) ):
		file.write( "\tUV %d COORDS %f %f\n" % ( i, vertuvs[i][0], vertuvs[i][1] ) )
		
	########
	# DONE #
	########
	print "Done! File \"%s\" created" % filename
	file.close()
main()