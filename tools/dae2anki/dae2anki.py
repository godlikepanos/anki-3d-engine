#!/usr/bin/python3

import optparse
import xml.etree.cElementTree as xml
from struct import pack

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

	def __eq__(self, b):
		return self.position == b.position and self.uv == b.uv

	def __ne__(self, b):
		return not self.__eq__(b)

	def __srt__(self):
		return "pos: %s, uv: %s" % (self.position.__str__(), self.uv.__str__())

class Mesh:
	def __init__(self):
		self.name = ""
		self.vertices = []
		self.indices = []

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

	geometry_el_arr = el.findall("geometry")
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
	source_elarr = mesh_el.findall("source")
	all_float_array = {}
	for source_el in source_elarr:
		source_id = source_el.get("id")
	
		float_array_el = source_el.find("float_array")
		all_float_array[source_id] = parse_float_array(float_array_el)

	# Create a link between vertices:id and vertices/input:source
	vertices_id_to_source = {}
	vertices_el = mesh_el.find("vertices")
	vertices_id = vertices_el.get("id")
	input_el = vertices_el.find("input")
	vertices_input_source = input_el.get("source")
	vertices_id_to_source[vertices_id] = vertices_input_source[1:]

	# Now find what it is what
	input_elarr = mesh_el.findall("polylist/input")
	for input_el in input_elarr:
		semantic = input_el.get("semantic")
		source = input_el.get("source")
		offset = input_el.get("offset")

		if semantic == "VERTEX":
			print("-- Found positions float_array")
			positions_float_array = \
				all_float_array[vertices_id_to_source[source[1:]]]
			positions_offset = offset
		elif semantic == "TEXCOORD":
			print("-- Found uvs float_array")
			uvs_float_array = all_float_array[source[1:]]
			uvs_offset = offset

	if positions_float_array == None:
		raise Exception("Positions not found")
	if uvs_float_array == None:
		raise Exception("UVs not found")

	return (positions_float_array, int(positions_offset), 
		uvs_float_array, int(uvs_offset))

def search_in_array(arr, el):
	""" XXX """
	for i in range(0, len(arr)):
		if arr[i] == el:
			return i
	return -1

def parse_geometry(geometry_el):
	""" XXX """

	geom_name = geometry_el.get("name")
	print("-- geometry: %s" % geom_name)

	mesh_el = geometry_el.find("mesh")

	# Get positions and UVs
	(positions_float_array, positions_offset, uvs_float_array, uvs_offset) = \
		get_positions_and_uvs(mesh_el)

	# get polylist
	polylist_el = mesh_el.find("polylist")
	inputs_count = len(polylist_el.findall("input"))

	# Make sure that we are dealing with triangles
	tokens = [x.strip() for x in 
		polylist_el.find("vcount").text.split(" ")]

	for token in tokens:
		if token:
			if int(token) != 3:
				raise Exception("Only triangles are alowed")	

	# Get face number
	face_count = int(mesh_el.find("polylist").get("count"))

	# Get p. p is a 3D array where the 1st dim is the face, 2nd is the vertex
	# and the 3rd is 0 for position, 1 for normal and 2 for text coord
	p = []
	tokens = [x.strip() for x in 
		polylist_el.find("p").text.split(" ")]

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
			index = p[fi * 3 * inputs_count + vi * inputs_count 
				+ positions_offset]
			
			pos = Vector(positions_float_array[index * 3], 
				positions_float_array[index * 3 + 1], 
				positions_float_array[index * 3 + 2])

			index = p[fi * 3 * inputs_count + vi * inputs_count + uvs_offset]

			uv = Vector(uvs_float_array[index * 2], 
				uvs_float_array[index * 2 + 1], 0.0)
			
			vert = Vertex()
			vert.position = pos
			vert.uv = uv

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
	return geom

def write_mesh(mesh, directory, flip):
	""" XXX """
	f = open(directory + "/" + mesh.name + ".mesh", "wb")
	
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
	buff += pack("I", 0)

	f.write(buff)
	f.close()

def main():
	(infile, outdir, flip) = parse_commandline()

	print("-- Begin...")
	xml.register_namespace("", "http://www.collada.org/2005/11/COLLADASchema")
	tree = xml.parse(infile)

	el_arr = tree.findall("library_geometries")
	for el in el_arr:
		geometries = parse_library_geometries(el)

		for geom in geometries:
			write_mesh(geom, outdir, flip)

if __name__ == "__main__":
	main()
