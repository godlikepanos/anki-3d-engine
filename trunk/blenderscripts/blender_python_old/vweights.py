from common import *
import sys
reload( sys.modules["common"] )


#===================================================================================================
# ScriptVWeights                                                                                   =
#===================================================================================================
def ScriptVWeights( mesh, skeleton, b_cmnts ):
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
	
	if( b_cmnts ):ftxt += "/*VERTS*/ "
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
# ExportVWeights                                                                                   =
#===================================================================================================
def ExportVWeights( path, mesh, skeleton, b_cmnts ):
	#check if mesh is the correct class
	if( mesh.__class__.__name__ != "Blender Mesh" ):
		ERROR( "The given func param is not a \"Blender Mesh\" class but a \"" + mesh.__class__.__name__ + "\"" )
		return 0
	
	#check if skeleton is the correct class
	if( skeleton.__class__.__name__ != "Armature" ):
		ERROR( "The given func param is not a \"Armature\" class but a \"" + skeleton.__class__.__name__ + "\"" )
		return 0
	
	
	INFO( "Trying to export vert weights for mesh \"" + mesh.name + "\"" )
	
	filename = path + mesh.name + ".vw"
	WriteFile( filename, ScriptVWeights( mesh, skeleton, b_cmnts ) )
	
	INFO( "Vert weights exported!! \"" + filename + "\"" )