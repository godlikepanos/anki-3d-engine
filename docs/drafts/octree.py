from math import *

octree_node_size = 112

def recurse(depth):
	if depth == 0:
		return 1
	else:
		return pow(8, depth) + recurse(depth - 1)

def octree_size(depth):
	return recurse(depth) * octree_node_size

print("Size %d" % (octree_size(3)))
