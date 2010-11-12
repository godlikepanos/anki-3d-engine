import sys
from common import *
reload( sys.modules["common"] )



#===================================================================================================
# GetMaterial                                                                                      =
#===================================================================================================
def GetMaterial( mesh ):
	#check if mesh is the correct class
	if( mesh.__class__.__name__ != "Blender Mesh" ):
		ERROR( "The given func param is not a \"Blender Mesh\" class but a \"" + mesh.__class__.__name__ + "\"" )
		return 0
	
	# check
	mats = mesh.materials
	if( len(mats) < 1 or len(mats) > 1 ):
		ERROR( "Mesh \"" + mesh.getName() + "\" has non or too many materials" )
		return 0
	
	mat = mats[0]
	return mat


#===================================================================================================
# ScriptMaterial                                                                                   =
#===================================================================================================
def ScriptMaterial( mtl, rpath, b_cmnts ):
	#check if mesh is the correct class
	if( mtl.__class__.__name__ != "Blender Material" ):
		ERROR( "The given func param is not a \"Blender Material\" class but a \"" + mtl.__class__.__name__ + "\"" )
		return "error"
	
	ftxt = ""
	
	# write the colors
	col = mtl.getRGBCol()
	ftxt += "DIFFUSE_COL {" + str(col[0]) + " " + str(col[1]) + " " + str(col[2]) + " 1.0}\n"
	
	ftxt += "SPECULAR_COL {" + str(col[0]) + " " + str(col[1]) + " " + str(col[2]) + " 1.0}\n"
	
	# shininess
	# we borow the shininess factor from the specular hardness
	sh = mtl.getHardness()
	if sh > 128:
		WARNING( "Mat \"" + mtl.getName() + ": Choose spec hardness from 0 to 128" )
		sh = 128
	ftxt += "SHININESS " + str(sh) + "\n"
	
	#from now on only user defined vars
	ftxt += "USER_DEFINED_VARS\n{\n"
	
	
	#** TEXTURES **
	texs = mtl.getTextures()
	has_diff = 0
	has_norm = 0
	has_spec = 0
	
	for tex in texs:
		if tex != None:
			tex = tex.tex 
			
			# get the script identifier
			if tex.name == "diffuse_map":
				has_diff = 1
			elif tex.name == "normal_map":
				has_norm = 1
			elif tex.name == "specular_map":
				has_spec = 1
			else:
				WARNING( "Mat \"" + mtl.getName() + "\": Tex name \"" + tex.name + "\" cannot be processed" )
				continue
			
			img = tex.getImage()
			# check if someone forgot to actually put an image
			if img == None:
				WARNING( "Mat \"" + mtl.getName() + "\": There is no image set for \"" + tex.name + "\" texture" )
				continue
				
			ftxt += "\t"
			
			# compute the fname
			fname = img.getFilename()
			if fname.find( rpath ) == -1: # the img is not in the engine's path
				WARNING( "Mat \"" + mtl.getName() + "\": Img \"" + fname + "\" is not in the engine's dir" )		
				continue
				
			relative = fname.replace( rpath, "" )
			
			ftxt += "TEXTURE " + tex.name + " \"" + relative + "\"\n"
		#endif
	#endfor
	ftxt += "}\n"
	
	# shader
	# choose a shader from one of the standards
	shader_str = "ms_mp_"
	if has_diff:
		shader_str += "d"
	else:
		shader_str += "*"
	if has_norm:
		shader_str += "n"
	else:
		shader_str += "*"
	if has_spec:
		shader_str += "s"
	else:
		shader_str += "*"

	shader_str += "*****"
		
		
	shader_str = "shaders/" + shader_str + ".shdr"
	
	ftxt += "SHADER \"" + shader_str + "\"\n"
	
	
	# end
	return ftxt
	
	
	
#===================================================================================================
# ExportMaterial                                                                                   =
#===================================================================================================
def ExportMaterial( path, rpath, mtl, b_cmnts ):
	#check if mesh is the correct class
	if( mtl.__class__.__name__ != "Blender Material" ):
		ERROR( "The given func param is not a \"Blender Material\" class but a \"" + mtl.__class__.__name__ + "\"" )
		return false
	
	INFO( "Trying to export material \"" + mtl.getName() + "\"" )
	
	filename = path + mtl.getName() + ".mtl"
	WriteFile( filename, ScriptMaterial( mtl, rpath, b_cmnts ) )
	
	INFO( "Material exported!! \"" + filename + "\"" )