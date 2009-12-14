import Blender
import string
import os.path

def MatrixMul( matrix, vector ):
	vec = [0, 0, 0]
	
	for i in range(0, 3):
		for j in range(0, 3):
			vec[i] = vec[i] + matrix[j][i] * vector[j]
		vec[i] = vec[i] + matrix[3][i]
		
	for i in range(0, 3):
		vector[i] = vec[i]
	

####################################################################################
file = 0
mesh = 0
round_to = 4
matrix = 0
path = "c:\\src\\VisualC++\\gmdl_3d_format\\models\\satan\\"			
if not os.path.exists( path ):
	path = "c:\\data\\projects\\VisualC++\\gmdl_3d_format\\models\\satan\\"
	

####################################################################################
def GetMesh():
	global file
	global mesh
	global path
	global matrix
	
	print "Getting mesh data..."
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

	matrix = objs[0].getMatrix()

	# open file
	file = open( path + mesh.name + ".mesh.g", "w" )
	
	print "Done!\n"
	return 1
	

####################################################################################
def VertGroups():
	global mesh
	global file
	print "Saving vertex groups..."
	
	# write vert groups
	group_names = mesh.getVertGroupNames()
	file.write( "GROUPS_NUM " + str(len( group_names )) + "\n" )
	i = 0
	for group_name in group_names:
		file.write( "GROUP " + str(i) + " GROUP_NAME " + group_name + " VERTS_NUM " )
		verts = mesh.getVertsFromGroup( group_name )
		file.write( str(len(verts)) + " VERTS " )
		for vert in verts: 
			file.write( str(vert) + " " )
		file.write( "\n" )
		i = i+1

	print "Done!\n"	
	return 1
	
	
####################################################################################
def Verts():
	global file
	global mesh
	global matrix
	print "Saving vertices..."

	# write vert data
	file.write( "VERTS_NUM " + str( len(mesh.verts) ) + "\n" )
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
		
		file.write( "VERT %d COORDS %f %f %f" %(vert.index, coords[0], coords[1], coords[2]) )
		file.write( " NORMAL %f %f %f\n" %(normal[0], normal[1], normal[2]) )
	
	print "Done!\n"
	return 1


####################################################################################
def Faces():
	print "Saving faces..."

	# WRITE THE FACES
	file.write( "FACE_NUM " + repr( len( mesh.faces ) ) + "\n" )
	
	# for every face
	i = 0
	for face in mesh.faces:
		
		# print type
		if( len( face.v ) != 3 ):
			print "-->ERROR: The mesh must contain only triagles"
			return 0
			
		file.write( "TRIANGLE " + str(i) + " VERTS " )
		
		# print index
		for vert in face.v:
			file.write( str(vert.index) + " " )
		
		# print normal cordS
		x = round( face.no.x, round_to)
		y = round( face.no.y, round_to)
		z = round( face.no.z, round_to)
		file.write( "NORMAL " + str(x) + " " + str(y) + " " + str(z) + " " )
		
		# WRITE UVs
		file.write( "UV " )
		for uv in face.uv:
			x = round( uv.x, round_to )
			y = round( uv.y, round_to )
			file.write( str(x) + " " + str(y) + " " )
		
		file.write( "\n" )
		
		i = i+1
	
	print "Done!\n"	
	return 1


####################################################################################
def main():
	global file
	
	print "\n--------------------------------------------------------"
	print "                       EXPORT MESH"
	print "--------------------------------------------------------\n"
	
	if GetMesh():
		if VertGroups():
			if Verts(): 
			 if Faces():
				print "File created at \"%s\"" %path
	
	
	if file: file.close()
	
	print "\n--------------------------------------------------------"

main()