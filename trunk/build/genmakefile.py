#!/usr/bin/python3.1
import sys, os, fnmatch, random
from threading import Thread


#======================================================================================================================================
# GLOBAL VARS                                                                                                                         =
#======================================================================================================================================
source_paths = []
precompiled_headers = []
executable_name = "unamed_project"
compiler = ""
common_flags = ""
compiler_flags = ""
precompiled_headers_flags = ""
linker_flags = ""


#======================================================================================================================================
# GetCommandOutput                                                                                                                    =
#======================================================================================================================================
def GetCommandOutput( command ):
	child = os.popen(command)
	data = child.read()
	err = child.close()
	if err:
		print( "GetCommandOutput failed:\n" + command )
		exit( 0 )
	return data


#======================================================================================================================================
# Threads                                                                                                                             =
#======================================================================================================================================
thread_list = []

class target_thread_t( Thread ):
	def __init__( self, tid, range ):
		Thread.__init__( self )
		self.tid = tid
		self.range = range
		self.out_str = ""
		
	def run( self ):
		for i in self.range:
			source_file = source_files[i]
			self.out_str += GetCommandOutput( compiler + " -MM " + compiler_flags + " " + source_file.cpp_file + " -MT " + source_file.obj_file )
			self.out_str += "\t@echo Compiling " + source_file.cpp_file + "...\n"
			self.out_str += "\t@$(CXX) $(INCPATH) $(CFLAGS) " + source_file.cpp_file + " -o " + \
			                source_file.obj_file + "\n\n"
			#print( "Im thread %d and I will make depends for %s" %(self.tid, source_file.fname) )
			#print( "Im thread %d and my i is %d" %(self.tid, i) )


#======================================================================================================================================
# source_file_t                                                                                                                       =
#======================================================================================================================================
class source_file_t:
	def __init__( self ):	
		self.cpp_file = ""
		self.obj_file = ""


#======================================================================================================================================
# main                                                                                                                                =
#======================================================================================================================================

# Read the arguments
input_cfgfile = ""
output_makefile = ""

i = 0
while 1:
	i = i+1
	if i>=len(sys.argv): break

	arg = sys.argv[i]
	if arg == "-h" or arg == "-help" or arg == "--help":
		print( "Makefile generator by GODlike" )
		print( "usage: genmakefile.py [options] [-i input] [-o output]" )
		print( "options:" )
		print( "-h, -help, --help  Print this text" )
		print( "-i                 Input config file. Default: gen.cfg.py" )
		print( "-o                 Output makefile. Default: Makefile" )
		exit(0)
	elif arg == "-i":
		input_cfgfile = sys.argv[i+1]
		i = i+1
	elif arg == "-o":
		output_makefile = sys.argv[i+1]
		i = i+1
	else:
		print( "Unrecognized argument " + arg )
	

if output_makefile == "":
	output_makefile = "Makefile"
if input_cfgfile == "":
	input_cfgfile = "gen.cfg.py"


# Check if cfg exists
if not os.path.exists( input_cfgfile ):
	print( "File " + input_cfgfile + " doesn't exist" )
	exit(0)


# compile the cfg
source = ""
f = open( input_cfgfile, "r" )
for line in f.readlines():
	source += line
	
exec( compile( source, input_cfgfile, "exec" ) )


# find the cpp files
source_files = []
for source_dir in source_paths:
	files = os.listdir( source_dir )
	for file_ in fnmatch.filter( files, "*.cpp" ):
		sfile = source_file_t()
		
		(fname_wo_ext, ext) = os.path.splitext( file_ )
		sfile.cpp_file = source_dir + "/" + file_
		sfile.obj_file = fname_wo_ext + ".o"
		
		# search all the source files and resolve conflicts in .o
		for sfile1 in source_files:
			if sfile1.obj_file == sfile.obj_file:
				print( "There is a conflict with \"" + sfile1.cpp_file + "\" and \"" + sfile.cpp_file + "\" but dont worry." )
				random.seed()
				sfile.obj_file = str(random.randint(1,99)) + "." + sfile.obj_file;
	
		source_files.append( sfile )
	

# now the precompiled headers
ph_files = []
for header in precompiled_headers:
	ph_files.append( source_file_t( header ) )


# build the string
master_str = ""

master_str += "CXX = " + compiler + "\n"
master_str += "COMMONFLAGS = " + common_flags + "\n"
master_str += "CFLAGS = $(COMMONFLAGS) " + compiler_flags + "\n"
master_str += "PHFLAGS = $(CFLAGS) " + precompiled_headers_flags + "\n"
master_str += "LFLAGS = $(COMMONFLAGS) " + linker_flags + "\n"
master_str += "EXECUTABLE = " + executable_name + "\n"

master_str += "INCPATH = "
for path in include_paths:
	master_str += "-I" + path + " "
	compiler_flags += " -I" + path + " "
master_str += "\n"

master_str += "SOURCES = "
for source_file in source_files:
	master_str += source_file.cpp_file + " "
master_str += "\n"

master_str += "OBJECTS = "
for source_file in source_files:
	master_str += source_file.obj_file + " "
master_str += "\n"

master_str += "PRECOMPILED_HEADERS = "
for header in ph_files:
	master_str += header.fname + ".gch "
master_str += "\n\n"

master_str += "all: $(PRECOMPILED_HEADERS) $(SOURCES) $(EXECUTABLE)\n\n"

master_str += "$(EXECUTABLE): $(OBJECTS)\n"
master_str += "\t@echo Linking...\n"
master_str += "\t@$(CXX) $(OBJECTS) $(LFLAGS) -o $(EXECUTABLE)\n"
master_str += "\t@echo All Done!\n\n"


for header in ph_files:
	depend_str = GetCommandOutput( compiler + " -MM " + compiler_flags + " " + precompiled_headers_flags + " " + header.path + "/" + 
	                               header.fname )
	master_str += depend_str.replace( header.fname_wo_ext + ".o", header.fname + ".gch" )
	master_str += "\t@echo Pre-compiling header " + header.fname + "...\n"
	master_str += "\t@$(CXX) $(INCPATH) $(PHFLAGS) " + header.path + "/" + header.fname + "\n\n"


# write source file target
threads_num = os.sysconf('SC_NPROCESSORS_ONLN')
print( "I will invoke %d threads to make the dependencies..." % threads_num )
num = len(source_files);
items_per_thread = num // threads_num;

for i in range(0, threads_num):
	begin = i*items_per_thread
	if i == threads_num-1:
		end = num
	else:
		end = begin + items_per_thread	
	thread = target_thread_t( i, range( int(begin), int(end) ) )
	thread.start()
	thread_list.append( thread )

for thread in thread_list:
	thread.join()

for thread in thread_list:
	master_str += thread.out_str

#for source_file in source_files:	
	#master_str += source_file.fname_wo_ext + ".o: " + source_file.path + source_file.fname_wo_ext + ".cpp"
	#master_str += GetCommandOutput( compiler + " -M " + compiler_flags + " " + source_file.path + "/" + source_file.fname )
	#master_str += "\t@echo Compiling " + source_file.fname + "...\n"
	#master_str += "\t@$(CXX) $(INCPATH) $(CFLAGS) " + source_file.path + "/" + source_file.fname + "\n\n"


master_str += "clean:\n"
master_str += "\trm -f *.o\n"
master_str += "\trm -f *.gch\n"
master_str += "\trm -f *~\n"
master_str += "\trm -f $(EXECUTABLE)\n\n"


# write file
f = open( output_makefile, "w" )
f.write( master_str )
print( "File \"" + output_makefile + "\" created!" )
