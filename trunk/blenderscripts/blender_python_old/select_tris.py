import Blender


print "\n---- Select Triagles ----"

obj = Blender.Object.GetSelected()[0]
mesh = obj.getData( 0, 1 )

edit_mode = Blender.Window.EditMode()
Blender.Window.EditMode(0) # exit from editmode

tris_num = 0
for face in mesh.faces:
	if len(face.verts) == 3:
		face.sel = 1
		tris_num = tris_num + 1
	else:
		face.sel = 0
	
	
Blender.Window.EditMode( edit_mode )
print "%d triangles selected" %tris_num
	