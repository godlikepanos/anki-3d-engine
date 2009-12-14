print "\n\n---- ALL IN ONE EXPORTER ----"

import Blender
import string
import os.path
from Blender import Mathutils
from Blender.Mathutils import *
from Numeric import *


#===================================================================================================
# global vars                                                                                      =
#===================================================================================================
g_engine_path = "/home/godlike/src/3d_engine/"
g_store_dir = ""
		
if not os.path.exists( g_engine_path + g_store_dir ):
	print "[E] path \"%s\" doesnt exist" % g_engine_path + g_store_dir



#===================================================================================================
# GetMesh                                                                                          =
#===================================================================================================
def GetMesh( obj ):
	if obj.getType() != "Mesh": 
		print "[E] The obj \"%s\" must link to a mesh and not to a(n) %s" % ( obj.getName(), obj.getType() )
		return 0

	mesh = obj.getData( 0, 1 ) 
	return mesh


#===================================================================================================
# GetMaterial                                                                                      =
#===================================================================================================
def GetMaterial( obj ):
	mesh = GetMesh(obj)
	if mesh == 0:
		return 0
	
	# check
	mats = mesh.materials
	if( len(mats) < 1 or len(mats) > 1 ):
		print "[E] Obj \"%s\" has no or too many materials" % obj.getName()
		return 0
	
	mat = mats[0]
	return mat


#===================================================================================================
# HasSkeleton                                                                                      =
#===================================================================================================
def HasSkeleton( obj ):
	# check modifiers
	if len(obj.modifiers) < 1:
		return 0
	
	# search for modifier of skeleton type
	for mod in obj.modifiers:
		if mod.type == Blender.Modifier.Types.ARMATURE:
			return 1
	
	return 0
	

#===================================================================================================
# GetSkeleton                                                                                      =
#===================================================================================================
def GetSkeleton( obj ):	
	# check modifiers
	if len(obj.modifiers) < 1:
		print "[E] Obj \"%s\" doesnt have modifiers so no armature found" % obj.getName()
		return 0
	
	# search for modifier of skeleton type
	for mod in obj.modifiers:
		if mod.type == Blender.Modifier.Types.ARMATURE:
			aobj = mod[Blender.Modifier.Settings.OBJECT]
			skeleton = Blender.Object.Get( aobj.name ).getData( 0, 1 ) # set skeleton
			return skeleton
	
	print "[E] Obj \"%s\" has no modifier of type armature" % obj.getName()
	return 0
		


#===================================================================================================
# RepeatTex                                                                                        =
# check if the mesh UVs are in [0,1] if not then the texture must be repeated                      =
#===================================================================================================
def RepeatTex( obj ):
	mesh = GetMesh( obj )
	if mesh == 0:
		return 0
	
	for face in mesh.faces:
		for i in range( 0, len(face.verts) ):
			uvx = face.uv[i].x
			uvy = face.uv[i].y
			
			if uvx < 0.0 or uvx > 1.0:
				return 1
			
			if uvy < 0.0 or uvy > 1.0:
				return 1
			
	return 0
	

		
#===================================================================================================
# ExportMesh                                                                                       =
#===================================================================================================
def ExportMesh( path, obj ):
	print "[I] Trying to export mesh for obj \"%s\"" % obj.getName()
	
	mesh = GetMesh(obj)  # set mesh
	if mesh == 0:
		return 0
	
	# check verts
	if len(mesh.verts) < 3:
		print "[E] The object named \"%s\" has insufficient vert num. Skipping" % obj.getName()		
		return 0

	# check UVs
	if not mesh.faceUV:
		print "[E] The mesh doesnt have UVs"
		return 0
	
	# open file for writing
	filename = path + obj.getName() + ".mesh"
	file = open( filename, "w" )
	
	# material
	mat = GetMaterial(obj)	
	if mat != 0:
		full = path + mat.getName() + ".mat"
		relative = full.replace( g_engine_path, "" )
		file.write( "MATERIAL \"%s\"\n" % relative )
	else:	
		file.write( "MATERIAL \"%s\"\n" % "put_something_here" )
	
	# the verts
	file.write( "VERTS_NUM %d\n" % len(mesh.verts) )
	for vert in mesh.verts:
		vec = Vector( vert.co )
		mat = obj.getMatrix( "worldspace" )
		mat2 = obj.getMatrix( "localspace" )
		
		vec = mat2 * vec
		
		file.write( "\tVERT %d COORDS %f %f %f\n" %(vert.index, vec.x, vec.z, -vec.y) )

	
	# the faces
	# first calc the triangle num
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
				print "[E] Obj \"%s\" More than 1 coords for the %d vert" % (obj.getName(), vert_id)
				mesh.verts[vert_id].sel=1
				return 0
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
					print "[E] Obj \"%s\" More than 1 coords for the %d vert" % (obj.getName(), vert_id)
					mesh.verts[vert_id].sel=1
					return 0
					#print " %f %f %f %f" % ( vertuvs[ vert_id ][0], vertuvs[ vert_id ][1], uvx, uvy )
	
	# one last check
	if( len( vertuvs ) != len( mesh.verts ) ):
		print "[E] There is a difference of verts num and verts uvs"
		return 0
			
	# now write the uvs to file
	file.write( "UVS_NUM %d\n" % len(vertuvs) )
	
	for i in range( 0, len(vertuvs) ):
		file.write( "\tUV %d COORDS %f %f\n" % ( i, vertuvs[i][0], vertuvs[i][1] ) )
	
	file.close()
	print "[I] Mesh exported!! %s" % filename
	
	return 1
	


	
#===================================================================================================
# ExportSkeleton                                                                                   =
#===================================================================================================
def ExportSkeleton( path, obj ):
	
	skeleton = GetSkeleton( obj )
	if skeleton == 0:
		return 0
	
	filename = g.path + g.obj.getName() + ".skel"
	file = open( filename, "w" )
	
	# write the file
	bone_names = skeleton.bones.keys()
	bone_names.sort() # the bones are written in alpabetical order

	file.write( "BONES_NUM %d\n" % len( bone_names ) )

	for bone_nam in bone_names:	
		
		bone = skeleton.bones[ bone_nam ]
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
	return 1
	


#===================================================================================================
# ExportVWeights                                                                                   =
#===================================================================================================
def ExportVWeights( path, obj ):
	print "[I] Trying to export vwights for the \"%s\"" % obj.getName()
	
	mesh = GetMesh( obj )
	if mesh == 0:
		return 0
	skeleton = GetSkeleton( obj )
	if skeleton == 0:
		return 0

	bone_names = skeleton.bones.keys()
	bone_names.sort()
	
	# open file
	filename = path + obj.getName() + ".vw"
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
			print "[W] Obj \"%s\": Vert group \"%s\" cant link to a bone" % (obj.getName(), vgroup_name)
		
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
			print "[E] Cannot have more than 4 bones per vert"
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
	return 1


#===================================================================================================
# ExportMaterial                                                                                   =
#===================================================================================================
def ExportMaterial( path, obj ):
	print "[I] Trying to export material for obj \"%s\"" % obj.getName()
	
	mat = GetMaterial(obj)
	if mat == 0:
		return 0
	
	# file ops
	filename = path + mat.getName() + ".mat"
	file = open( filename, "w" )
	
	# write the colors
	col = mat.getRGBCol()
	file.write( "DIFFUSE_COL %f %f %f\n" %(col[0], col[1], col[2]) )
	
	col = mat.getSpecCol()
	file.write( "SPECULAR_COL %f %f %f\n" %(col[0], col[1], col[2]) )
	
	#** TEXTURES **
	texs = mat.getTextures()
	has_diff = 0
	has_norm = 0
	has_spec = 0
	
	for tex in texs:
		if tex != None:
			tex = tex.tex 
			
			# get the script identifier
			str = ""
			if tex.name == "diff":
				str = "DIFFUSE_MAP"
				has_diff = 1
			elif tex.name == "norm":
				str = "NORMAL_MAP"
				has_norm = 1
			elif tex.name == "spec":
				str = "SPECULAR_MAP"
				has_spec = 1
			else:
				print "[W] Mat \"%s\": Tex name \"%s\" cannot be processed" % (mat.getName(), tex.name)
				continue
			
			img = tex.getImage()
			# check if someone forgot to actually put an image
			if img == None:
				print "[E] Mat \"%s\": There is no image set for \"%s\" tex" % (mat.getName(), tex.name)
				return 0
				
			# create the corect filename of the tex. We want the path to be relative
			full = img.getFilename() # getFilename returnes the full path
			if full.find( g_engine_path ) == -1: # the img is not in the engine's path
				print "[E] Mat \"%s\": Img \"%s\" is not in the engine's dir" % (mat.getName(), full)
				return 0
				
			relative = full.replace( g_engine_path, "" )
			
			# now its time to write to the file
			file.write( "%s \"%s\"\n" %(str, relative) )
	
	
	# shader
	# choose a shader from one of the standards
	shader_str = "mp_"
	if has_diff:
		shader_str = shader_str + "d"
	if has_norm:
		shader_str = shader_str + "n"
	if has_spec:
		shader_str = shader_str + "s"
		
	shader_str = shader_str + ".shdr"
	
	file.write( "SHADER \"%s\"\n" %("shaders/" + shader_str) )
	
	# shininess
	# we borow the shininess factor from the specular hardness
	sh = mat.getHardness()
	if sh > 128:
		print "[W] Obj \"%s\" and mat \"%s\": Choose spec hardness from 0 to 128" % (obj.getName(), mat.getName())
		sh = 128
	file.write( "SHININESS %d\n" % sh )
	
	
	# repeat texes ??
	if RepeatTex( obj ):
		file.write( "REPEAT_TXTR_MAP_S %d\n" % 1 )
		file.write( "REPEAT_TXTR_MAP_T %d\n" % 1 )
	else:
		file.write( "REPEAT_TXTR_MAP_S %d\n" % 0 )
		file.write( "REPEAT_TXTR_MAP_T %d\n" % 0 )
	
	# end
	file.close()
	print "[I] Material exported!! %s" % filename
	return 1
	

#===================================================================================================
# ExportAllSelcted                                                                                 =
#===================================================================================================
def ExportAllSelected( path ):
	objs = Blender.Object.GetSelected()

	if len(objs) < 1:
		print "[E] Not selected objs"
		return 0
	
	# file ops
	filename = path + "map.map"
	file = open( filename, "w" )
	
	for obj in objs:
		ExportMesh( path, obj )
		ExportMaterial( path, obj )
		file.write( "\"%s\"\n" % obj.getName() )
		print " "
		
	file.close()
	return 1	

#===================================================================================================
# main                                                                                             =
#===================================================================================================
def main():

	ExportAllSelected( g_engine_path + g_store_dir )

	print "\nAll Done!"
	
	
main()