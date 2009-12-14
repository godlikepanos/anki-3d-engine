import Blender


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

obj = Blender.Object.GetSelected()[0]
arm = obj.getData(0,1)
act = obj.getAction()
ipo = act.getAllChannelIpos()["down"]

print ipo.curves
print ipo.curves[5].__getitem__(3)

print


kframes = obj.getAction().getFrameNumbers()
print kframes

for frame in kframes:
	Blender.Set( "curframe", frame )
	Blender.Redraw()

	pb_names = obj.getPose().bones.keys()
	for pb_nam in pb_names:
		pb = obj.getPose().bones[ pb_nam ]
		
		if pb_nam == "up":
			print "HEAD ", pb.head
			print "TAIL ", pb.tail
			print
			PrintMatR( pb.localMatrix )
			print