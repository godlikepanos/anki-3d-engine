#!/usr/bin/python
print( "Starting..." )
import sys, os, fnmatch, compiler
from threading import Thread


#======================================================================================================================================
# GetCommandOutput                                                                                                                    =
#======================================================================================================================================
def GetCommandOutput( command ):
	child = os.popen(command)
	data = child.read()
	err = child.close()
	if err:
		print( command + " failed" )
		exit( 0 )
	return data


#======================================================================================================================================
# Threads                                                                                                                             =
#======================================================================================================================================
THREADS_NUM = 2

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
			self.out_str += GetCommandOutput( compiler + " -M " + compiler_flags + " " + source_file.path + "/" + source_file.fname )
			self.out_str += "\t@echo Compiling " + source_file.fname + "...\n"
			self.out_str += "\t@$(CXX) $(INCPATH) $(CFLAGS) " + source_file.path + "/" + source_file.fname + "\n\n"
			#print( "Im thread %d and I will make depends for %s" %(self.tid, source_file.fname) )
			#print( "Im thread %d and my i is %d" %(self.tid, i) )


# read config
if len(sys.argv) == 2:
	f = open( sys.argv[1] )
else:
	f = open( "gen.cfg.py" )
	
str = ""
for line in f.readlines():
	str += line + "\n"
eval( compiler.compile( str, "err.log", "exec" ) )


# class source_file_t
class source_file_t:
	def __init__( self, fullfname ):
		(path, fname) = os.path.split( fullfname )
		(fname_wo_ext, ext) = os.path.splitext( fname )
		ext = ext[:-len(ext)]
		
		self.path = path
		self.fname = fname
		self.fname_wo_ext = fname_wo_ext
		self.ext = ext

source_files = []
ph_files = []


# find the cpp files
for source_dir in include_paths:
	files = os.listdir( source_dir )
	for file_ in fnmatch.filter( files, "*.cpp" ):
		source_files.append( source_file_t( source_dir + file_ ) )
		

# now the precompiled headers
for header in precompiled_headers:
	ph_files.append( source_file_t( header ) )


# build the string
master_str = ""

master_str += "CXX = " + compiler + "\n"
master_str += "COMMONFLAGS = " + common_flags + "\n"
master_str += "CFLAGS = $(COMMONFLAGS) " + compiler_flags + "\n"
master_str += "PHFLAGS = $(CFLAGS) " + precompiled_headers_flags + "\n"
master_str += "LFLAGS = $(COMMONFLAGS) " + linker_flags + "\n"
master_str += "EXECUTABLE = " + project_name + "\n"

master_str += "INCPATH = "
for path in include_paths:
	master_str += "-I" + path + " "
	compiler_flags += " -I" + path + " "
master_str += "\n"

master_str += "SOURCES = "
for source_file in source_files:
	master_str += source_file.path + "/" + source_file.fname_wo_ext + ".cpp "
master_str += "\n"

master_str += "OBJECTS = "
for source_file in source_files:
	master_str += source_file.fname_wo_ext + ".o "
master_str += "\n"

master_str += "PRECOMPILED_HEADERS = "
for header in ph_files:
	master_str += header.fname + ".gch "
master_str += "\n\n"

master_str += "all: $(PRECOMPILED_HEADERS) $(SOURCES) $(EXECUTABLE)\n\n"

master_str += "$(EXECUTABLE): $(OBJECTS)\n"
master_str += "\t@echo Linking...\n"
master_str += "\t@$(CXX) $(LFLAGS) $(OBJECTS) -o $(EXECUTABLE)\n"
master_str += "\t@echo All Done!\n\n"


for header in ph_files:
	depend_str = GetCommandOutput( compiler + " -M " + compiler_flags + " " + precompiled_headers_flags + " " + header.path + "/" + header.fname )
	master_str += depend_str.replace( header.fname_wo_ext + ".o", header.fname + ".gch" )
	master_str += "\t@echo Pre-compiling header " + header.fname + "...\n"
	master_str += "\t@$(CXX) $(INCPATH) $(PHFLAGS) " + header.path + "/" + header.fname + "\n\n"


# write source file target
print( "I will invoke %d threads to make the dependencies..." % THREADS_NUM )
num = len(source_files);
items_per_thread = num // THREADS_NUM;

for i in range(0, THREADS_NUM):
	begin = i*items_per_thread
	if i == THREADS_NUM-1:
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
f = open( "Makefile", "w" )
f.write( master_str )
print( "File \"Makefile\" created!" )
