#!/usr/bin/python

# Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
# All rights reserved.
# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE


def gen_vec_class_name(n):
	return "TVec<T, " + str(n) + ">"


# Index to component string
def index_to_component(idx):
	arr = ["x", "y", "z", "w"]
	return arr[idx]


def gen_accessors():
	h = ""
	scalar = "T"

	# All swizzle accessors
	for vec_components in range(2, 4 + 1):
		for a in range(0, 4):
			for b in range(0, 4 if vec_components > 1 else 1):
				for c in range(0, 4 if vec_components > 2 else 1):
					for d in range(0, 4 if vec_components > 3 else 1):
						arr = [a, b, c, d]

						# Enable it
						max_comp = max(max(max(a, b), c), d)
						h += "\n\tANKI_ENABLE_IF_EXPRESSION(N > " + str(max_comp) + ")\n"

						# Return value
						h += "\t"
						if vec_components == 1:
							h += scalar + " "
						else:
							ret_type = gen_vec_class_name(vec_components)
							h += ret_type + " "

						# Method name
						method_name = ""
						for i in range(0, vec_components):
							method_name += index_to_component(arr[i])
						h += method_name

						# Write the body
						h += "() const\n"
						h += "\t{\n"
						h += "\t\treturn " + gen_vec_class_name(vec_components) + "("
						for i in range(0, vec_components):
							h += "m_carr[" + str(arr[i]) + ("], " if i < vec_components - 1 else "]);\n")
						h += "\t}\n"

	return h


def main():
	print(gen_accessors())


if __name__ == "__main__":
	main()
