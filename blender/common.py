import Blender
import string
import os.path
from Blender import Mathutils
from Blender.Mathutils import *


#===================================================================================================
# vars                                                                                             =
#===================================================================================================
NULL = 0
false = 0
true = 1


#===================================================================================================
# Messages                                                                                         =
#===================================================================================================
"""
Print a generic error. Used by ERROR, WARNING and INFO only
"""
def GenericError( msg0, msg1 ):
	import sys
	line = sys._getframe(2).f_lineno
	fname = sys._getframe(2).f_code.co_filename
	func_name = sys._getframe(2).f_code.co_name

	print( "-->%s (%s:%i %s): %s" % (msg0, fname, line, func_name, msg1) )


""" Prints an error """
def ERROR( msg ):
	GenericError( "ERROR", msg )


""" Prints a warning """    
def WARNING( msg ):
	GenericError( "WARNING", msg )


""" Prints some info """
def INFO( msg ):
	GenericError( "INFO", msg )
	
	
#===================================================================================================
# WriteFile                                                                                        =
#===================================================================================================
def WriteFile( filename, txt ):
	file = open( filename, "w" )
	file.write( txt )
