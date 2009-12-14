from common import *
import sys
reload( sys.modules["common"] )




#===================================================================================================
# GetSkeleton                                                                                      =
#===================================================================================================
def GetSkeleton( obj ):	
	# check if obj is correct class
	if( obj.__class__.__name__ != "Blender Object" ):
		ERROR( "The given func param is not a \"Blender Object\" class but a \"" + obj.__class__.__name__ + "\"" )
		return 0
	
	# check modifiers
	if len(obj.modifiers) < 1:
		ERROR( "Obj \"" + obj.getName() + "\" doesnt have modifiers so no armature found" )
		return 0
	
	# search for modifier of skeleton type
	for mod in obj.modifiers:
		if mod.type == Blender.Modifier.Types.ARMATURE:
			aobj = mod[Blender.Modifier.Settings.OBJECT]
			skeleton = Blender.Object.Get( aobj.name ).getData( 0, 1 ) # set skeleton
			return skeleton
	
	ERROR( "Obj \"" + obj.getName() + "\" has no modifier of type armature" )
	return 0
		


#===================================================================================================
# ScriptSkeleton                                                                                   =
#===================================================================================================
def ScriptSkeleton( skeleton, b_cmnts ):
	# check if skeleton is correct class
	if( skeleton.__class__.__name__ != "Armature" ):
		ERROR( "The given func param is not a \"Armature\" class but a \"" + skeleton.__class__.__name__ + "\"" )
		return "error"
	
	ftxt = "" # file text
	
	# write the file
	bone_names = skeleton.bones.keys()
	bone_names.sort() # the bones are written in alpabetical order

	if( b_cmnts ): ftxt += "/*BONES*/ "
	ftxt += str( len(bone_names) ) + "\n"

	for bone_nam in bone_names:	
		bone = skeleton.bones[ bone_nam ]
		if( b_cmnts ): ftxt += "\t/*\"" + bone.name + "\" BONE_ID " + str(bone_names.index(bone_nam)) + "*/\n"
		
		# name
		if( b_cmnts ): ftxt += "\t\t/*BONE_NAME*/ "	
		ftxt += "\"" + bone.name + "\"\n"
		
		# head
		co = bone.head["ARMATURESPACE"]
		if( b_cmnts ): ftxt +=  "\t\t/*HEAD_ARMATURE_SPACE*/ "
		ftxt += str(co.x) + " " + str(co.y) + " " + str(co.z) + "\n"
		
		# tail
		co = bone.tail["ARMATURESPACE"]				
		if( b_cmnts ): ftxt +=  "\t\t/*TAIL_ARMATURE_SPACE*/ "
		
		ftxt += str(co.x) + " " + str(co.y) + " " + str(co.z) + "\n"
		
		# matrix		
		if( b_cmnts ): ftxt += "\t\t/*MATRIX_ARMATURE_SPACE*/ "
				
		for i_ in range(0, 4):
			for j_ in range(0, 4):
				ftxt += str( bone.matrix["ARMATURESPACE"][j_][i_] ) + " "
		ftxt += "\n"
		
		# write the parent
		if( b_cmnts ): ftxt += "\t\t/*PARENT_ID*/ "
				
		if not bone.parent:
			ftxt += "NULL\n"
		else:
			ftxt += str( bone_names.index( bone.parent.name ) )
			if( b_cmnts ): ftxt += " // " + bone.parent.name
			ftxt += "\n"
					

			
		# write the childs
		if( b_cmnts ): ftxt += "\t\t/*CHILD_IDS_NUM*/ "
		ftxt += str( len(bone.children) ) + " "
		if( b_cmnts ): ftxt += "/*CHILD_IDS*/ "
		
		for child in bone.children:
			ftxt += str( bone_names.index( child.name ) ) + " "
			if( b_cmnts ): ftxt += "/*" + child.name + "*/ "
		
		ftxt += "\n"	
		
	return ftxt
	
	
#===================================================================================================
# ExportSkeleton                                                                                   =
#===================================================================================================
def ExportSkeleton( path, skeleton, b_cmnts ):
	#check if mesh is the correct class
	if( skeleton.__class__.__name__ != "Armature" ):
		ERROR( "The given func param is not a \"Armature\" class but a \"" + skeleton.__class__.__name__ + "\"" )
		return false
	
	INFO( "Trying to export skeleton \"" + skeleton.name + "\"" )
	
	filename = path + skeleton.name + ".skel"
	WriteFile( filename, ScriptSkeleton( skeleton, b_cmnts ) )
	
	INFO( "Skeleton exported!! \"" + filename + "\"" )	