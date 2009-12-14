import Blender
import os.path

def PrintMatrixR( matrix ):
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

	print "\n\n---- Export Armature ----"

	########
	# vars #
	########
	file = 0
	path = "c:\\src\\VisualC++\\3d_engine\\models\\imp\\"
	armat = 0
	mesh = 0


	##################################
	# make some checks and open file #
	##################################
	objs = Blender.Object.GetSelected()

	if len( objs ) < 1: 
		print "-->ERROR: No object selected"
		return 0
	
	obj = objs[0]
	
	if obj.getType() != "Mesh": 
		print "-->ERROR: The selected object must be a mesh and not in a(n) " + objs[0].getType()
		return 0

	mesh = obj.getData(0,1)

	# get the armature
	if len( obj.modifiers )<1:
		print "-->ERROR: Mesh doesnt have modifiers"
		return 0

	for mod in obj.modifiers:
		if mod.type == Blender.Modifier.Types.ARMATURE:
			aobj = mod[Blender.Modifier.Settings.OBJECT]
			armat = Blender.Object.Get( aobj.name ).getData( 0, 1 )
	
	if armat == 0:
		print "-->ERROR: There is no modifier of type armature"
		return 0
	
	filename = path + "armature.txt"
	file = open( filename, "w" )
	
	
	# create bones dictonary
	bone_names = armat.bones.keys()
	bones_d = {}
	bone_names.sort()
	i = 0
	for bone_nam in bone_names:
		bones_d[ bone_nam ] = i
		i = i+1
	
	###############
	# write bones #
	###############	
	file.write( "BONES_NUM " + str(len( bone_names )) + "\n" )

	i = 0
	bone_names.sort() # the bones are written in alpabetical order
	for bone_nam in bone_names:
		
		vgroup_names = mesh.getVertGroupNames()
		vgroup_names.sort()
		vg_i = 0
		for vgroup_nam in vgroup_names:
			if vgroup_nam == bone_nam:
				break
			vg_i = vg_i+1
		if vg_i == len(vgroup_names): vg_i=-1
			
		
		bone = armat.bones[ bone_nam ]
		file.write( "\tBONE %d\n \t\tNAME %s\n" %(i, bone.name) )
		
		"""co = bone.head["ARMATURESPACE"]
		file.write( "\t\tHEAD_ARMATURESPACE %f %f %f\n" %(co.x, co.y, co.z) )
		
		co = bone.head["BONESPACE"]
		file.write( "\t\tHEAD_BONESPACE     %f %f %f\n" %(co.x, co.y, co.z) )
		
		co = bone.tail["ARMATURESPACE"]
		file.write( "\t\tTAIL_ARMATURESPACE %f %f %f\n" %(co.x, co.y, co.z) )
		
		co = bone.tail["BONESPACE"]
		file.write( "\t\tTAIL_BONESPACE     %f %f %f\n" %(co.x, co.y, co.z) )
		
		file.write( "\t\tMATRIX4_ARMATURESPACE " )
		for i_ in range(0, 4):
			for j_ in range(0, 4):
				file.write( "%f " % bone.matrix["ARMATURESPACE"][j_][i_] )
		file.write( "\n" )

		file.write( "\t\tMATRIX3_BONESPACE     " )
		for i_ in range(0, 3):
			for j_ in range(0, 3):
				file.write( "%f " % bone.matrix["BONESPACE"][j_][i_] )
		file.write( "\n" )
		
		file.write("\t\tLENGTH %f\n" % bone.length)"""
		
		co = bone.head["ARMATURESPACE"]
		file.write( "\t\tHEAD %f %f %f\n" %(co.x, co.y, co.z) )
		
		co = bone.tail["ARMATURESPACE"]
		file.write( "\t\tTAIL %f %f %f\n" %(co.x, co.y, co.z) )
		
		
		# write the parent
		bpar = bone.parent
		if not bpar:
			file.write( "\t\tPARENT -1\n" )
		else:
			bpar_index = bones_d[ bpar.name ]
			file.write( "\t\tPARENT %d\n" %(bpar_index) )
			
		# write the childs
		file.write( "\t\tCHILDS_NUM %d CHILD_INDECES " % len(bone.children) )
		for bch in bone.children:
			file.write( "%d " % bones_d[bch.name] )
		file.write( "\n" )
		
		i = i+1
	

	print "Done! File \"%s\" created" % filename
	file.close()
main()