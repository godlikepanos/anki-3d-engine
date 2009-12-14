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
		self.path = "/home/godlike/"
		
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
	
	file.write( "{ " )
	
	# for every face
	for face in mesh.faces:
		for vert in face.v:
			for coord in vert.co:
				file.write( "%f, " % coord )
		
	file.write( " } " )
	


	

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

	ExportMesh()
	

	print "All Done!"
	
	
main()