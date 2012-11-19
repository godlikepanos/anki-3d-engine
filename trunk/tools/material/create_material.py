#!/usr/bin/python3

import optparse

def main():
	# Command line args
	parser = optparse.OptionParser("usage: %prog [options]")

	parser.add_option("-t", "--template", dest="template",
		type="string", help="specify material template")
	parser.add_option("-o", "--output", dest="out",
		type="string", help="specify the output filename")
	parser.add_option("-v", "--vars", dest="vars",
		type="string", help="specify the variables to replace. "
		"Format var:val,var1:val1")

	(options, args) = parser.parse_args()

	if not options.template or not options.out:
		parser.error("argument is missing")

	# Open template
	ftempl = open(options.template, "r")
	templ = ftempl.read()
	ftempl.close()
	
	# Parse vars
	if options.vars:
		varvals = options.vars.split(",")
		for varval in varvals:
			(var, val) = varval.split(":")
			print("-- Replacing %%%s%% with %s" % (var, val))

			templ = templ.replace("%" + var + "%", val)

	# Write out file
	fout = open(options.out, "w")
	fout.write(templ)
	fout.close()

if __name__ == "__main__":
	main()
