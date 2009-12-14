import Blender

print "\n\n----"


Blender.Window.EditMode(0)


objs = Blender.Object.GetSelected()
mesh = objs[0].getData( 0, 1 )


for face in mesh.faces:
	face.sel = 0

	
for face in mesh.faces:
	found = 0
	for uv in face.uv:
		x = uv.x
		y = uv.y 
		if (x==0.0 or x==1.0) and (y==0.0 or y==1.0):
			found = found + 1
	if found:
		face.sel = 1

Blender.Window.EditMode(1)