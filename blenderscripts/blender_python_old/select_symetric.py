import Blender



print "\n---- Selecting Symetric ----"

obj = Blender.Object.GetSelected()[0]
mesh = obj.getData( 0, 1 )
edit_mode = Blender.Window.EditMode()
Blender.Window.EditMode(0) # exit from editmode


# get selected verts
sel_verts = []
for i in range( 0, len(mesh.verts) ):
	if mesh.verts[i].sel:
		sel_verts.append( mesh.verts[i] )
		

# get the x symetric
ABBERANCE = 0.005

for sel_vert in sel_verts:
	sel_x = sel_vert.co.x
	sel_y = sel_vert.co.y
	sel_z = sel_vert.co.z

	# for every vert select the symetric	
	sym_verts = []
	
	for vert in mesh.verts:
		x = -vert.co.x
		y = vert.co.y
		z = vert.co.z
		
		if ( sel_x >= x-ABBERANCE and sel_x <= x+ABBERANCE ):
			if ( sel_y >= y-ABBERANCE and sel_y <= y+ABBERANCE ):
				if ( sel_z >= z-ABBERANCE and sel_z <= z+ABBERANCE ):
					sym_verts.append( vert )
	
					
	if len( sym_verts ) > 1 :
		print "More than one symetric verteces found. Please lower the ABBERANCE"
	elif len( sym_verts ) < 1:
		print "No symetric vertex found. Please higher the ABBERANCE"
	else:
		sym_verts[0].sel = 1
		sel_vert.sel = 0


Blender.Window.EditMode( edit_mode )

print "Done!"