#!/usr/bin/python3

import optparse
import xml.etree.cElementTree as xml
from struct import pack
import os

class Vector:
	""" 3D vector """
	def __init__(self, x, y, z):
		self.x = x
		self.y = y
		self.z = z

	def __eq__(self, b):
		return self.x == b.x and self.y == b.y and self.z == b.z

	def __ne__(self, b):
		return not self.__eq__(b)

	def __str__(self):
		return "x: %f, y: %f, z: %f" %(self.x, self.y, self.z)

class Vertex:
	""" Vertex """
	def __init__(self):
		self.position = Vector(0.0, 0.0, 0.0)
		self.uv = Vector(0.0, 0.0, 0.0)
		self.bones_count = 0
		self.bone_ids = [-1, -1, -1, -1]
		self.weights = [-1.0, -1.0, -1.0, -1.0]
		self.index = -1

	def __eq__(self, b):
		return self.position == b.position and self.uv == b.uv \
			and self.bones_count == b.bones_count \
			and compare_arr(self.bone_ids, b.bone_ids) \
			and compare_arr(self.weights, b.weights)

	def __ne__(self, b):
		return not self.__eq__(b)

	def __srt__(self):
		return "pos: %s, uv: %s" % (self.position.__str__(), self.uv.__str__())

class Mesh:
	def __init__(self):
		self.name = ""
		self.vertices = []
		self.indices = []
		self.id = "" # The mesh:id needed for skinning
		# The mesh/source/float_array that includes the vertex positions. 
		# Needed for skinning
		self.vert_positions = None
		self.material_name = "unnamed.mtl"

def compare_arr(a, b):
	""" Compare 2 arrays """
	la = len(a)
	lb = len(b)
	if la != lb:
		raise Exception("Arrays don't have the same size")
	for i in range(la):
		if a[i] != b[i]:
			return False

	return True

def search_in_array(arr, el):
	""" Self explanatory. Return index in array or -1 if not found """
	for i in range(0, len(arr)):
		if arr[i] == el:
			return i
	return -1

def to_collada_tag(name):
	""" Transform a name of the tag to something that COLLADA format 
	    understands """
	collada_str = "{http://www.collada.org/2005/11/COLLADASchema}"
	s = name.replace("/", "/" + collada_str)
	return collada_str + s

def parse_commandline():
	""" Parse the command line arguments """
	parser = optparse.OptionParser("usage: %prog [options]")
	parser.add_option("-i", "--input", dest="inp",
		type="string", help="specify the .DAE file to parse")
	parser.add_option("-o", "--output", dest="out",
		type="string", help="specify the directory to save the files")
	parser.add_option("-f", "--flip-yz", dest="flip",
		type="string", default=False, 
		help="flip Y with Z (from blender to AnKi)")
	(options, args) = parser.parse_args()

	if not options.inp or not options.out:
		parser.error("argument is missing")

	return (options.inp, options.out, options.flip)

def parse_library_geometries(el):
	geometries = []

	geometry_el_arr = el.findall(to_collada_tag("geometry"))
	for geometry_el in geometry_el_arr:
		geometries.append(parse_geometry(geometry_el))

	return geometries

def parse_float_array(float_array_el):
	""" XXX """
	floats = []

	tokens = [x.strip() for x in float_array_el.text.split(' ')]

	for token in tokens:
		if token:
			floats.append(float(token))

	return floats

def get_positions_and_uvs(mesh_el):
	""" Given a <mesh> get the positions and the UVs float_array. Also return
	    the offset of the mesh/polylist/input:offset for those two arrays """
	positions_float_array = None
	uvs_float_array = None

	# First get all
	source_elarr = mesh_el.findall(to_collada_tag("source"))
	all_float_array = {}
	for source_el in source_elarr:
		source_id = source_el.get("id")
	
		float_array_el = source_el.find(to_collada_tag("float_array"))
		all_float_array[source_id] = parse_float_array(float_array_el)

	# Create a link between vertices:id and vertices/input:source
	vertices_id_to_source = {}
	vertices_el = mesh_el.find(to_collada_tag("vertices"))
	vertices_id = vertices_el.get("id")
	input_el = vertices_el.find(to_collada_tag("input"))
	vertices_input_source = input_el.get("source")
	vertices_id_to_source[vertices_id] = vertices_input_source[1:]

	# Now find what it is what
	input_elarr = mesh_el.findall(to_collada_tag("polylist/input"))
	for input_el in input_elarr:
		semantic = input_el.get("semantic")
		source = input_el.get("source")
		offset = input_el.get("offset")

		if semantic == "VERTEX":
			print("------ Found positions float_array")
			positions_float_array = \
				all_float_array[vertices_id_to_source[source[1:]]]
			positions_offset = offset
		elif semantic == "TEXCOORD":
			print("------ Found uvs float_array")
			uvs_float_array = all_float_array[source[1:]]
			uvs_offset = offset

	if positions_float_array == None:
		raise Exception("Positions not found")
	if uvs_float_array == None:
		raise Exception("UVs not found")

	# Convert the positions float array to a Vector array
	positions_vec_array = []
	for i in range(int(len(positions_float_array) / 3)):
		x = positions_float_array[i * 3]
		y = positions_float_array[i * 3 + 1]
		z = positions_float_array[i * 3 + 2]
		vec = Vector(x, y, z)
		positions_vec_array.append(vec)

	return (positions_vec_array, int(positions_offset), 
		uvs_float_array, int(uvs_offset))

def parse_geometry(geometry_el):
	""" XXX """

	geom_name = geometry_el.get("name")
	geom_id = geometry_el.get("id")
	print("---- Parsing geometry: %s" % geom_name)

	mesh_el = geometry_el.find(to_collada_tag("mesh"))

	# Get positions and UVs
	(positions_vec_array, positions_offset, uvs_float_array, uvs_offset) = \
		get_positions_and_uvs(mesh_el)

	# Get polylist
	polylist_el = mesh_el.find(to_collada_tag("polylist"))
	inputs_count = len(polylist_el.findall(to_collada_tag("input")))

	# Get material
	mtl = polylist_el.get("material")

	# Make sure that we are dealing with triangles
	tokens = [x.strip() for x in 
		polylist_el.find(to_collada_tag("vcount")).text.split(" ")]

	for token in tokens:
		if token:
			if int(token) != 3:
				raise Exception("Only triangles are alowed")	

	# Get face number
	face_count = int(mesh_el.find(to_collada_tag("polylist")).get("count"))

	# Get p. p is a 3D array where the 1st dim is the face, 2nd is the vertex
	# and the 3rd is 0 for position, 1 for normal and 2 for text coord
	p = []
	tokens = [x.strip() for x in 
		polylist_el.find(to_collada_tag("p")).text.split(" ")]

	for token in tokens:
		if token:
			p.append(int(token))

	# For every face
	verts = []
	indices = []
	for fi in range(0, face_count):
		# Get the positions for each vertex
		for vi in range(0, 3):
			# index = p[fi][vi][positions_offset]
			pindex = p[fi * 3 * inputs_count + vi * inputs_count 
				+ positions_offset]
			
			pos = positions_vec_array[pindex]

			uvindex = p[fi * 3 * inputs_count + vi * inputs_count + uvs_offset]

			uv = Vector(uvs_float_array[uvindex * 2], 
				uvs_float_array[uvindex * 2 + 1], 0.0)
			
			vert = Vertex()
			vert.position = pos
			vert.uv = uv
			vert.index = pindex

			i = search_in_array(verts, vert)
			if i == -1:
				verts.append(vert)
				indices.append(len(verts) - 1)
			else:
				indices.append(i)

	# Create geometry
	geom = Mesh()
	geom.vertices = verts
	geom.indices = indices
	geom.name = geom_name
	geom.id = geom_id
	geom.vert_positions = positions_vec_array
	if mtl:
		geom.material_name = mtl

	print("------ Number of verts: %d" % len(geom.vertices))
	print("------ Number of faces: %d" % (len(geom.indices) / 3))
	return geom

def update_mesh_with_vertex_weights(mesh, skin_el):
	""" XXX """
	print("---- Updating with skin information: %s" % mesh.name)

	# Get all <source>
	source_data = {}
	source_elarr = skin_el.findall(to_collada_tag("source"))
	for source_el in source_elarr:
		source_id = source_el.get("id")
	
		float_array_el = source_el.find(to_collada_tag("float_array"))
		name_array_el = source_el.find(to_collada_tag("Name_array"))

		if float_array_el != None:
			source_data[source_id] = parse_float_array(float_array_el)
		elif name_array_el != None:
			tokens = [x.strip() for x in name_array_el.text.split(' ')]
			array = []
			for token in tokens:
				if token:
					array.append(token)
			source_data[source_id] = array
		else:
			raise Exception("Expected float_array or name_array")

	# Now find the joint names and the weights
	joint_names = None
	joint_names_offset = -1
	weight_arr = None
	weight_arr_offset = -1

	vertex_weights_el = skin_el.find(to_collada_tag("vertex_weights"))
	input_elarr = vertex_weights_el.findall(to_collada_tag("input"))
	for input_el in input_elarr:
		semantic = input_el.get("semantic")
		source = input_el.get("source")
		source = source[1:]
		offset = int(input_el.get("offset"))

		source_array = source_data[source]

		if semantic == "JOINT":
			joint_names = source_array
			joint_names_offset = offset
			print("------ Found bone names")
		elif semantic == "WEIGHT":
			weight_arr = source_array
			weight_arr_offset = offset
			print("------ Found weights")
		else:
			raise Exception("Unrecognized semantic: %s" % semantic)
			
	# Get <vcount>
	vcount = []
	tokens = [x.strip() for x in 
		vertex_weights_el.find(to_collada_tag("vcount")).text.split(" ")]

	for token in tokens:
		if token:
			vcount.append(int(token))

	# Get <v>
	v = []
	tokens = [x.strip() for x in 
		vertex_weights_el.find(to_collada_tag("v")).text.split(" ")]

	for token in tokens:
		if token:
			v.append(int(token))

	# Do a sanity check because we made an assumption
	if len(vcount) != len(mesh.vert_positions):
		raise Exception("Wrong assumption made")

	# Now that you have all do some magic... connect them
	#
	
	# For every vert
	other_index = 0
	for vert_id in range(len(vcount)):
		bones_in_vert = vcount[vert_id]

		if bones_in_vert > 4:
			raise Exception("You cannot have more than 4 bones per vertex")

		if bones_in_vert == 0:
			print("------ *WARNING* Vertex does not have bones: %d" % vert_id)

		# Get weigths and ids for the vertex
		bone_ids = [-1, -1, -1, -1]
		vweights = [0.0, 0.0, 0.0, 0.0]
		for i in range(bones_in_vert):
			bone_name_index = v[other_index * 2 + joint_names_offset]
			weight_index = v[other_index * 2 + weight_arr_offset]

			bone_ids[i] = bone_name_index
			vweights[i] = weight_arr[weight_index]

			other_index += 1

		# Update the vertex and the duplicates
		for vert in mesh.vertices:
			if vert.index != vert_id:
				continue
			vert.bones_count = bones_in_vert
			vert.bone_ids = bone_ids
			vert.weights = vweights

	# Do a sanity check. Go to all verts and check if bones_count is set
	for vert in mesh.vertices:
		if vert.bones_count == -1:
			raise Exception("Vertex skining information not set for vertex")

def write_mesh(mesh, directory, flip):
	""" Write mesh to file """
	filename = directory + "/" + mesh.name + ".mesh"
	print("---- Writing file: %s" % filename)
	f = open(filename, "wb")
	
	# Magic
	buff = pack("8s", b"ANKIMESH")

	# Mesh name
	buff += pack("I" + str(len(mesh.name)) + "s", len(mesh.name), 
		mesh.name.encode("utf-8"))

	# Verts num
	buff += pack("I", len(mesh.vertices))

	# Verts
	if flip:
		for vert in mesh.vertices:
			buff += pack("fff", vert.position.x, vert.position.z, 
				-vert.position.y)
	else:
		for vert in mesh.vertices:
			buff += pack("fff", vert.position.x, vert.position.y, 
				vert.position.z)

	# Tris num
	buff += pack("I", int(len(mesh.indices) / 3))

	# Indices
	for i in mesh.indices:
		buff += pack("I", int(i))

	# Tex coords
	buff += pack("I", len(mesh.vertices))
	for vert in mesh.vertices:
		buff += pack("ff", vert.uv.x, vert.uv.y)

	# Vert weight
	if mesh.vertices[0].bones_count > 0:
		buff += pack("I", len(mesh.vertices))

		for vert in mesh.vertices:
			buff += pack("I", vert.bones_count)
			for i in range(vert.bones_count):
				buff += pack("If", vert.bone_ids[i], vert.weights[i])
	else:
		buff += pack("I", 0)

	f.write(buff)
	f.close()

def write_mesh_v2(mesh, directory, flip):
	noop

def write_model(meshes, directory, mdl_name):
	""" Write the .model XML file """
	filename = directory + "/" + mdl_name + ".mdl"
	print("---- Writing file: %s" % filename)
	f = open(filename, "w")

	f.write("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n")
	f.write("<model>\n")
	f.write("\t<modelPatches>\n")

	for mesh in meshes:
		f.write("\t\t<modelPatch>\n")
		f.write("\t\t\t<mesh>%s</mesh>\n" 
			% os.path.abspath(directory + "/" + mesh.name + ".mesh"))
		f.write("\t\t\t<material>%s</material>\n" % "unnamed.mtl")
		f.write("\t\t</modelPatch>\n")

	f.write("\t</modelPatches>\n")
	f.write("</model>\n")

	f.close()

def main():
	(infile, outdir, flip) = parse_commandline()

	print("-- Begin...")
	xml.register_namespace("", "http://www.collada.org/2005/11/COLLADASchema")
	tree = xml.parse(infile)

	# Get meshes
	el_arr = tree.findall(to_collada_tag("library_geometries"))
	for el in el_arr:
		meshes = parse_library_geometries(el)

	# Update with skin info
	skin_elarr = tree.findall(to_collada_tag(
		"library_controllers/controller/skin"))
	for skin_el in skin_elarr:
		source = skin_el.get("source")
		source = source[1:]

		for mesh in meshes:
			if mesh.id == source:
				update_mesh_with_vertex_weights(mesh, skin_el)

	# Now write meshes
	for mesh in meshes:
		write_mesh(mesh, outdir, flip)

	# Write the model
	mdl_name = os.path.splitext(os.path.basename(infile))[0]
	write_model(meshes, outdir, mdl_name)

	print("-- Bye!")

if __name__ == "__main__":
	main()
