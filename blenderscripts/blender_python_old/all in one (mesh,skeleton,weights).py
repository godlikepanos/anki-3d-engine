print "\n\n---- ALL IN ONE EXPORTER ----"

import Blender
import string
import os.path
from Numeric import *


#===================================================================================================
# global vars                                                                                      =
#===================================================================================================
class globals_t:
	def __init__(self):
		self.mesh = 0
		self.skeleton = 0
		self.obj = 0   # selected object
		self.path = "/home/godlike/src/3d_engine/meshes/cube/"
		
		if not os.path.exists( self.path ):
			print "-->ERROR: path \"%s\" doesnt exists" % self.path

g = globals_t()
		
	
		
#===================================================================================================
# ExportMesh                                                                                       =
#===================================================================================================
def ExportMesh():
	filename = g.path + g.obj.getName() + ".mesh"
	file = open( filename, "w" )
	
	mesh = g.mesh
	
	file.write( "MATERIAL %s\n" % "put_something_here" )
	
	# the verts
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
		
		file.write( "\tVERT %d COORDS %f %f %f\n" %(vert.index, coords[0], coords[1], coords[2]) )

	
	# the faces
	# first count the triangles
	tris_num = 0
	for face in mesh.faces:
		if len(face.v) == 3:
			tris_num = tris_num + 1 # if tris
		else:
			tris_num = tris_num + 2 # if quad
	
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
	

	# the uvs
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
	
	file.close()
	print "Mesh exported!! %s" % filename
	


	
#===================================================================================================
# ExportSkeleton                                                                                   =
#===================================================================================================
def ExportSkeleton():
	filename = g.path + g.obj.getName() + ".skel"
	file = open( filename, "w" )
	
	# write the file
	bone_names = g.skeleton.bones.keys()
	bone_names.sort() # the bones are written in alpabetical order

	file.write( "BONES_NUM %d\n" % len( bone_names ) )

	for bone_nam in bone_names:	
		
		bone = g.skeleton.bones[ bone_nam ]
		file.write( "\tBONE %d\n \t\tNAME %s\n" %( bone_names.index(bone_nam), bone.name) )
		
		"""file.write( "\t\tLENGTH %f\n" % bone.length )"""
		
		# head
		"""co = bone.head["BONESPACE"]
		file.write( "\t\tHEAD_BS %f %f %f\n" %(co.x, co.y, co.z) )"""
		
		co = bone.head["ARMATURESPACE"]
		file.write( "\t\tHEAD_AS %f %f %f\n" %(co.x, co.y, co.z) )
		
		# tail
		"""co = bone.tail["BONESPACE"]
		file.write( "\t\tTAIL_BS %f %f %f\n" %(co.x, co.y, co.z) )"""
		
		co = bone.tail["ARMATURESPACE"]
		file.write( "\t\tTAIL_AS %f %f %f\n" %(co.x, co.y, co.z) )
		
		# matrix
		"""file.write( "\t\tMATRIX_BS " )
		for i_ in range(0, 3):
			for j_ in range(0, 3):
				file.write( "%f " % bone.matrix["BONESPACE"][j_][i_] )
		file.write( "\n" )"""
		
		file.write( "\t\tMATRIX_AS " )
		for i_ in range(0, 4):
			for j_ in range(0, 4):
				file.write( "%f " % bone.matrix["ARMATURESPACE"][j_][i_] )
		file.write( "\n" )
		
		# write the parent
		if not bone.parent:
			file.write( "\t\tPARENT -1\n" )
		else:
			file.write( "\t\tPARENT %d\n" % (bone_names.index( bone.parent.name )) )
			
		# write the childs
		file.write( "\t\tCHILDS_NUM %d CHILD_IDS " % len(bone.children) )
		for child in bone.children:
			file.write( "%s " % bone_names.index( child.name ) )
		file.write( "\n" )
	

	print "Skeleton exported! %s" % filename
	


#===================================================================================================
# ExportVWeights                                                                                   =
#===================================================================================================
def ExportVWeights():
	bone_names = g.skeleton.bones.keys()
	bone_names.sort()
	mesh = g.mesh
	
	# open file
	filename = g.path + g.obj.getName() + ".vw"
	file = open( filename, "w" )
	
	
	# link the vert groups to the bone ids
	vgroup2bone_id = {}   # we give the vgroup name and we get the bone's id in the skeleton
	vgroup_names = mesh.getVertGroupNames()
	
	for vgroup_name in vgroup_names:
		bone_id = -1
		for bone_name in bone_names:
			if bone_name == vgroup_name:
				bone_id = bone_names.index( bone_name )
				break
		
		if bone_id == -1:
			print "-->WARNING: Vert group \"%s\" cant link to a bone" % vgroup_name
		
		vgroup2bone_id[ vgroup_name ] = bone_id
	
	file.write( "VERTS_NUM %d\n" % len( mesh.verts ) )
	
	# for every vert do some shit
	for vert in mesh.verts:
		influences = mesh.getVertexInfluences( vert.index )
		
		influences_num = 0
		sumw = 0.0
		# calc the influences num and the total weight (NOTE:we may have...
		# ...a vert group that doesnt connect to a bone)
		for influence in influences:
			vgroup = influence[0]
			weight = influence[1]
			
			if vgroup2bone_id[ vgroup ] != -1:
				influences_num = influences_num + 1
				sumw = sumw + weight
		
		# a check
		if( influences_num > 4 ):
			print "-->ERROR: Cannot have more than 4 bones per vert"
			file.close()
			return 0
	
		# write to file
		file.write( "\tVERT_ID %d\n\t\tBONES_NUM %d\n" %( vert.index, influences_num ) )
		
		for influence in influences:
			vgroup = influence[0]
			weight = influence[1]
			
			if vgroup2bone_id[ vgroup ] != -1:
				file.write( "\t\t\tBONE_ID %d WEIGHT %f\n" %( vgroup2bone_id[ vgroup ], weight/sumw ) )
	
	
	# finished
	file.close()
	print "Vertex weights exported! %s" % filename

	

#===================================================================================================
# main                                                                                             =
#===================================================================================================
def main():

	# init some globals	
	objs = Blender.Object.GetSelected()

	if len( objs ) != 1: 
		print "-->ERROR: You have to select ONE object"
		return 0

	g.obj = objs[0]  # set g.obj
	
	if g.obj.getType() != "Mesh": 
		print "-->ERROR: The selected object must link to a mesh and not in a(n) " + g.obj.getType()
		return 0

	g.mesh = g.obj.getData( 0, 1 )  # set g.mesh
	
	if not g.mesh.faceUV:
		print "-->ERROR: The mesh doesnt have UVs"
		return 0


	ExportMesh()
	
	# skeleton
	if len(g.obj.modifiers) < 1:
		print "Obj doesnt have modifiers (skeleton & vert weights wont be exported)"
	else:
		# search for modifier of skeleton type
		for mod in g.obj.modifiers:
			if mod.type == Blender.Modifier.Types.ARMATURE:
				aobj = mod[Blender.Modifier.Settings.OBJECT]
				g.skeleton = Blender.Object.Get( aobj.name ).getData( 0, 1 ) # set g.skeleton
		
		if g.skeleton == 0:
			print "There is no modifier of type armature (skeleton & vert weights wont be exported)"
		else:
			ExportSkeleton()
			ExportVWeights()

	print "All Done!"
	
	
main()