import sys
from common import *
reload( sys.modules["common"] )


#===================================================================================================
# mesh_init_t                                                                                      =
#===================================================================================================
class mesh_init_t:
	mesh = NULL
	skeleton = NULL
	material_filename = "*put material filename*"
	write_comments = false
	save_path = ""
	

#===================================================================================================
# GetMesh                                                                                          =
#===================================================================================================
def GetMesh( obj ):
	if( obj.__class__.__name__ != "Blender Object" ):
		ERROR( "The given func param is not a \"Blender Object\" class but a \"" + obj.__class__.__name__ + "\"" )
		return 0
	
	if obj.getType() != "Mesh": 
		ERROR( "The obj \"" + obj.getName() + "\" must link to a mesh and not to a(n) " + obj.getType() )
		return 0

	mesh = obj.getData( 0, 1 ) 
	return mesh


#===================================================================================================
# ScriptVWeights                                                                                   =
#===================================================================================================
def ScriptVWeights( mesh_init ):
	mesh = mesh_init.mesh
	skeleton = mesh_init.skeleton
	b_cmnts = mesh_init.write_comments

	#check if mesh is the correct class
	if( mesh.__class__.__name__ != "Blender Mesh" ):
		ERROR( "The given func param is not a \"Blender Mesh\" class but a \"" + mesh.__class__.__name__ + "\"" )
		return "error"
	
	#check if skeleton is the correct class
	if( skeleton.__class__.__name__ != "Armature" ):
		ERROR( "The given func param is not a \"Armature\" class but a \"" + skeleton.__class__.__name__ + "\"" )
		return "error"
	
	bone_names = skeleton.bones.keys()
	bone_names.sort()
	
	# init text
	ftxt = ""
	
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
			WARNING( "Vert group \"" + vgroup_name + "\" cant link to a bone" )
		
		vgroup2bone_id[ vgroup_name ] = bone_id
	
	if( b_cmnts ):ftxt += "/*VERT_WEIGHTS*/ "
	ftxt += str( len( mesh.verts ) ) + "\n"
	
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
			ERROR( "Cannot have more than 4 bones per vert" );
			return "error"
	
		# write to file
		if( b_cmnts ): ftxt += "\t/*VERT_ID " + str( vert.index ) + "*/\n"
		
		if( b_cmnts ): ftxt += "\t/*BONE_CONNECTIONS*/ "
		ftxt += str( influences_num ) + "\n"
				
		for influence in influences:
			vgroup = influence[0]
			weight = influence[1]
			
			if vgroup2bone_id[ vgroup ] != -1:	
				if( b_cmnts ): ftxt += "\t\t/*BONE_ID*/ "
				ftxt += str( vgroup2bone_id[ vgroup ] ) + " "
				if( b_cmnts ): ftxt += "/*WEIGHT*/ "
				ftxt += str( weight/sumw ) + "\n"
	# end for all verts
	
	return ftxt
	
		
#===================================================================================================
# ScriptMesh                                                                                       =
#===================================================================================================
def ScriptMesh( mesh_init ):
	mesh = mesh_init.mesh
	b_cmnts = mesh_init.write_comments
	skeleton = mesh_init.skeleton

	#check if mesh is the correct class
	if( mesh.__class__.__name__ != "Blender Mesh" ):
		ERROR( "The given func param is not a \"Blender Mesh\" class but a \"" + mesh.__class__.__name__ + "\"" )
		return "error"

	# check verts number
	if len(mesh.verts) < 3:
		ERROR( "The mesh named \"" + mesh.name + "\" has insufficient vert num. Skipping" )
		return "error"

	# check UVs
	"""if not mesh.faceUV:
		ERROR( "The mesh named \"" + mesh.name + "\" doesnt have UVs" )
		return "error"""
	
	# init txt
	ftxt = ""
	
	# material
	if(b_cmnts): ftxt += "/*MATERIAL*/ "
	ftxt += "\"" + mesh_init.material_filename + "\"\n"
	
	# the verts
	if(b_cmnts): ftxt += "/*VERTS*/ "
	
	ftxt += str( len(mesh.verts) ) + "\n"
	
	for vert in mesh.verts:
		vec = Vector( vert.co )
		
		if(b_cmnts): ftxt += "\t/*VERT_ID "+str(vert.index)+" COORDS*/ "
		
		ftxt += str( vec.x ) + " " + str( vec.y ) + " " + str( vec.z ) + "\n"

	
	# the faces
	# first calc the triangles num
	tris_num = 0
	for face in mesh.faces:
		if len(face.v) == 3:
			tris_num = tris_num + 1 # if tris
		else:
			tris_num = tris_num + 2 # if quad

	if(b_cmnts): ftxt += "/*FACES*/ "
	ftxt += str( tris_num ) + "\n"
	
	# for every face
	i = 0
	for face in mesh.faces:
		
		if(b_cmnts): ftxt += "\t/*FACE_ID " + str(i) + " VERT_IDS*/ "
		
		order = [0,1,2]
		# print index
		for j in order:
			ftxt += str( face.v[j].index ) + " "
		
		i = i+1
		
		ftxt += "\n"
		
		# if the face is quad then triangulate
		if( len( face.v ) == 4 ):
			order = [0,2,3]
		
			if(b_cmnts): ftxt += "\t/*FACE_ID " + str(i) + " VERT_IDS*/ "
			
			for j in order:
				ftxt += str( face.v[j].index ) + " "
				
			ftxt += "\n"
			i = i+1
	

	# the uvs
	# create and populate the vertuvs array
	vertuvs = {}
	
	# if has UVs
	if mesh.faceUV:
		for vert in mesh.verts:
			vertuvs[ vert.index ] = [ -1000.0, -1000.0 ]
	
		for face in mesh.faces:
			
			order = [0,1,2] # verts order
		
			for j in order:
				vert_id = face.verts[j].index
			
				uvx = face.uv[j].x
				uvy = face.uv[j].y
			
				# if we put a new value in the array OR the values already set and they are the same then
				if vertuvs[ vert_id ][0]==-1000.0 or ( vertuvs[ vert_id ][0] == uvx and vertuvs[ vert_id ][1] == uvy ):
					vertuvs[ vert_id ][0] = uvx
					vertuvs[ vert_id ][1] = uvy
				else:
					ERROR( "The mesh \"" + mesh.name + "\" More than 1 coords for the " + str(vert_id) + " vert" )
					mesh.verts[vert_id].sel=1
					return "error"
					#print " %f %f %f %f" % ( vertuvs[ vert_id ][0], vertuvs[ vert_id ][1], uvx, uvy )
			
			# do the same if quat for the other forming triangle
			if( len( face.verts ) == 4 ):
				order = [0,2,3]
			
				for j in order:
					vert_id = face.verts[j].index
				
					uvx = face.uv[j].x
					uvy = face.uv[j].y
				
					# if we put a new value in the array OR the values already set and they are the same then
					if vertuvs[ vert_id ][0]==-1000.0 or ( vertuvs[ vert_id ][0] == uvx and vertuvs[ vert_id ][1] == uvy ):
						vertuvs[ vert_id ][0] = uvx
						vertuvs[ vert_id ][1] = uvy
					else:
						ERROR( "The mesh \"" + mesh.name + "\" More than 1 coords for the " + str(vert_id) + " vert" )
						mesh.verts[vert_id].sel=1
						return "error"
						#print " %f %f %f %f" % ( vertuvs[ vert_id ][0], vertuvs[ vert_id ][1], uvx, uvy )
	# endif has UVs
				
	# now put the UVs in the ftxt
	if(b_cmnts): ftxt += "/*VERT_TEX_COORDS*/ "
		
	ftxt += str( len(vertuvs) ) + "\n"
	
	for i in range( 0, len(vertuvs) ):
		if(b_cmnts): ftxt += "\t/*VERT_ID " + str(i) + " UV_COORDS*/ "
				
		ftxt += str( vertuvs[i][0] ) + " " + str( vertuvs[i][1] ) + "\n"
		
	# and now the vertex weights
	if skeleton != NULL:
		ftxt += ScriptVWeights( mesh_init )
	else:
		if b_cmnts:
			ftxt += "/*VERT_WEIGHTS*/ "
		ftxt += "0"
	
	
	return ftxt


#===================================================================================================
# ExportMesh                                                                                       =
#===================================================================================================
def ExportMesh( mesh_init ):
	mesh = mesh_init.mesh
	skeleton = mesh_init.skeleton
	
	#check if mesh is the correct class
	if mesh.__class__.__name__ != "Blender Mesh":
		ERROR( "The given func param is not a \"Blender Mesh\" class but a \"" + mesh.__class__.__name__ + "\"" )
		return false
	
	if skeleton != NULL:
		#check if skeleton is the correct class
		if( skeleton.__class__.__name__ != "Armature" ):
			ERROR( "The given func param is not a \"Armature\" class but a \"" + skeleton.__class__.__name__ + "\"" )
			return false
	
	INFO( "Trying to export mesh \"" + mesh.name + "\"" )
	filename = mesh_init.save_path + mesh.name + ".mesh"
	WriteFile( filename, ScriptMesh( mesh_init ) )
	INFO( "Mesh exported!! \"" + filename + "\"" )	
	
	