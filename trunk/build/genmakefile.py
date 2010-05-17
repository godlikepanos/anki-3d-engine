#!/usr/bin/python3.1
import sys, os, fnmatch, random, re
from threading import Thread


#======================================================================================================================================
# GLOBAL VARS                                                                                                                         =
#======================================================================================================================================
sourcePaths = []
precompiledHeaders = []
executableName = "unamed_project"
compiler = ""
commonFlags = ""
compilerFlags = ""
precompiledHeadersFlags = ""
linkerFlags = ""
sourceFilesRegExpr = r".*\.[c++|cpp|cc|cxx|C|c]"
includePaths = []


#======================================================================================================================================
# getCommandOutput                                                                                                                    =
#======================================================================================================================================
def getCommandOutput( command ):
	child = os.popen(command)
	data = child.read()
	err = child.close()
	if err:
		print( "getCommandOutput failed:\n" + command )
		exit( 0 )
	return data


#======================================================================================================================================
# Threads                                                                                                                             =
#======================================================================================================================================
threadList = []

class TargetThread( Thread ):
	def __init__( self, tid, range ):
		Thread.__init__( self )
		self.tid = tid
		self.range = range
		self.out_str = ""
		
	def run( self ):
		for i in self.range:
			source_file = sourceFiles[i]
			self.out_str += getCommandOutput( compiler + " -MM " + compilerFlags + " " + source_file.cppFile + " -MT " + source_file.objFile )
			self.out_str += "\t@echo Compiling " + source_file.cppFile + "...\n"
			self.out_str += "\t@$(CXX) $(INCPATH) $(CFLAGS) " + source_file.cppFile + " -o " + \
			                source_file.objFile + "\n\n"
			#print( "Im thread %d and I will make depends for %s" %(self.tid, source_file.fname) )
			#print( "Im thread %d and my i is %d" %(self.tid, i) )


#======================================================================================================================================
# SourceFile                                                                                                                          =
#======================================================================================================================================
class SourceFile:
	def __init__( self ):	
		self.cppFile = ""
		self.objFile = ""


#======================================================================================================================================
# main                                                                                                                                =
#======================================================================================================================================

# Read the arguments
inputCfgFile = ""
outputMakefile = ""

i = 0
while 1:
	i = i+1
	if i>=len(sys.argv): break

	arg = sys.argv[i]
	if arg == "-h" or arg == "-help" or arg == "--help":
		print( "Makefile generator by GODlike" )
		print( "usage: " + sys.argv[0] + " [options] [-i input] [-o output]" )
		print( "options:" )
		print( "-h, -help, --help  Print this text" )
		print( "-i                 Input config file. Default: gen.cfg.py" )
		print( "-o                 Output makefile. Default: Makefile" )
		exit(0)
	elif arg == "-i":
		inputCfgFile = sys.argv[i+1]
		i = i+1
	elif arg == "-o":
		outputMakefile = sys.argv[i+1]
		i = i+1
	else:
		print( "Unrecognized argument " + arg )
	

if outputMakefile == "":
	outputMakefile = "Makefile"
if inputCfgFile == "":
	inputCfgFile = "gen.cfg.py"


# Check if cfg exists
if not os.path.exists( inputCfgFile ):
	print( "File " + inputCfgFile + " doesn't exist" )
	exit(0)


# compile the cfg
source = ""
f = open( inputCfgFile, "r" )
for line in f.readlines():
	source += line
	
exec( compile( source, inputCfgFile, "exec" ) )


# find the cpp files
sourceFiles = []
regexpr = re.compile( sourceFilesRegExpr )
for sourceDir in sourcePaths:
	files = os.listdir( sourceDir )
	for file_ in files:
		if not regexpr.match( file_ ): continue
		
		sfile = SourceFile()
		
		(fname_wo_ext, ext) = os.path.splitext( file_ )
		sfile.cppFile = sourceDir + "/" + file_
		sfile.objFile = fname_wo_ext + ".o"
		
		# search all the source files and resolve conflicts in .o
		for sfile1 in sourceFiles:
			if sfile1.objFile == sfile.objFile:
				print( "There is a naming conflict between \"" + sfile1.cppFile + "\" and \"" + sfile.cppFile + "\" but dont worry." )
				random.seed()
				sfile.objFile = str(random.randint(1,99)) + "." + sfile.objFile;
	
		sourceFiles.append( sfile )
	

# now the precompiled headers
phFiles = []
for header in precompiledHeaders:
	sFile = SourceFile()
	(fnameWoExt, ext) = os.path.splitext( header )
	sFile.cppFile = header
	sFile.objFile = fnameWoExt + ".gch"
	phFiles.append( sFile )


# build the string
masterStr = ""

masterStr += "CXX = " + compiler + "\n"
masterStr += "COMMONFLAGS = " + commonFlags + "\n"
masterStr += "CFLAGS = $(COMMONFLAGS) " + compilerFlags + "\n"
masterStr += "PHFLAGS = $(CFLAGS) " + precompiledHeadersFlags + "\n"
masterStr += "LFLAGS = $(COMMONFLAGS) " + linkerFlags + "\n"
masterStr += "EXECUTABLE = " + executableName + "\n"

masterStr += "INCPATH = "
for path in includePaths:
	masterStr += "-I" + path + " "
	compilerFlags += " -I" + path + " "
masterStr += "\n"

masterStr += "SOURCES = "
for source_file in sourceFiles:
	masterStr += source_file.cppFile + " "
masterStr += "\n"

masterStr += "OBJECTS = "
for source_file in sourceFiles:
	masterStr += source_file.objFile + " "
masterStr += "\n"

masterStr += "PRECOMPILED_HEADERS = "
for header in phFiles:
	masterStr += header.objFile
masterStr += "\n\n"

masterStr += "all: $(PRECOMPILED_HEADERS) $(SOURCES) $(EXECUTABLE)\n\n"

masterStr += "$(EXECUTABLE): $(OBJECTS)\n"
masterStr += "\t@echo Linking...\n"
masterStr += "\t@$(CXX) $(OBJECTS) $(LFLAGS) -o $(EXECUTABLE)\n"
masterStr += "\t@echo All Done!\n\n"


for header in phFiles:
	dependStr = getCommandOutput( compiler + " -MM " + compilerFlags + " " + precompiledHeadersFlags + " " + header.cppFile )
	#masterStr += dependStr.replace( header.fname_wo_ext + ".o", header.fname + ".gch" )
	masterStr += "\t@echo Pre-compiling header " + header.cppFile + "...\n"
	masterStr += "\t@$(CXX) $(INCPATH) $(PHFLAGS) " + header.objFile + "\n\n"


# write source file target
threadsNum = os.sysconf('SC_NPROCESSORS_ONLN')
print( "I will invoke %d threads to make the dependencies..." % threadsNum )
num = len(sourceFiles);
itemsPerThread = num // threadsNum;

for i in range(0, threadsNum):
	begin = i*itemsPerThread
	if i == threadsNum-1:
		end = num
	else:
		end = begin + itemsPerThread	
	thread = TargetThread( i, range( int(begin), int(end) ) )
	thread.start()
	threadList.append( thread )

for thread in threadList:
	thread.join()

for thread in threadList:
	masterStr += thread.out_str

#for source_file in sourceFiles:	
	#masterStr += source_file.fname_wo_ext + ".o: " + source_file.path + source_file.fname_wo_ext + ".cpp"
	#masterStr += getCommandOutput( compiler + " -M " + compilerFlags + " " + source_file.path + "/" + source_file.fname )
	#masterStr += "\t@echo Compiling " + source_file.fname + "...\n"
	#masterStr += "\t@$(CXX) $(INCPATH) $(CFLAGS) " + source_file.path + "/" + source_file.fname + "\n\n"


masterStr += "clean:\n"
masterStr += "\trm -f *.o\n"
masterStr += "\trm -f *.gch\n"
masterStr += "\trm -f *~\n"
masterStr += "\trm -f $(EXECUTABLE)\n\n"


# write file
f = open( outputMakefile, "w" )
f.write( masterStr )
print( "File \"" + outputMakefile + "\" created!" )
