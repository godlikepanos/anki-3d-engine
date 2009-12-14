import Blender
import os.path


def PrintMatR( matrix ):
	for i in range(0, 4):
		print "[",
		for j in range(0, 4):
			f = round( matrix[j][i], 5)
			if f >= 0 :
				print " %04f" % f,
			else:
				print "%04f" % f,
		print "]"


print "\n\n----"

path = "c:\\src\\VisualC++\\gmdl_3d_format\\models\\"			
if not os.path.exists( path ):
	path = "c:\\data\\projects\\VisualC++\\gmdl_3d_format\\models\\"
file = open( path+"matrices.txt", "w" )


obj = Blender.Object.GetSelected()[0]

frames_num = obj.getAction().getFrameNumbers()[ len(obj.getAction().getFrameNumbers())-1 ]

for frame in (1, 20, 27):
	Blender.Set( "curframe", frame )
	Blender.Redraw()
	print "FRAME ", frame

	file.write( "------------------- FRAME %d ----------------\n" % frame )

	pb_names = obj.getPose().bones.keys()
	for pb_nam in pb_names:
		pb = obj.getPose().bones[ pb_nam ]
		
		
		file.write( "BONE \"%s\"\n" % pb_nam )
		
		file.write( "HEAD " )
		for i in range( 3 ):
			file.write( "%f " % pb.head[i] )
		file.write( "\n" )
		
		file.write( "TAIL " )
		for i in range( 3 ):
			file.write( "%f " % pb.tail[i] )
		file.write( "\n" )
	
		file.write( "LOC  " )
		for i in range( 3 ):
			file.write( "%f " % pb.loc[i] )
		file.write( "\n" )
		
	
		for i in range(0, 4):
			for j in range(0, 4):
				file.write( "%f " % pb.localMatrix[j][i] )
			file.write( "\n" )
		file.write( "\n" )
			
file.close()