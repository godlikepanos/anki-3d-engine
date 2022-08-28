The Main Window
===============

.. toctree::
   :maxdepth: 4
   :name: toc-GUI_TheMainWindow

   imageview
   toolbars
   applicationoptions
   propertyview
   projectexplorer
   messageoutput

|image9|

Detaching Windows from the Application
--------------------------------------

With the mouse position next to any gray shaded area next to the title
of the window, press down the left mouse key and at the same time slide
the mouse cursor away from the application. This will detach the window
and allow you to position it at a new location, either inside the
application or at a new desktop screen location. Once you have
positioned the window to where it is desired, simply release the left
mouse key.

Above are illustrations of where the mouse can be positioned for moving
the window.

Note: The Project View window is not moveable.

When the window is moved “Un-Docked” around the application a blue
shaded area will be displayed showing what new places the moved window
can be placed.

Window with Blue shaded application section.

Example of Window outside of the application.

The advantage of moving windows outside of the application is to provide
a larger viewing area for the image and allow side by side comparisons
when using multiple monitors.

Batch Processing from Command Line Tool
---------------------------------------

Once a project has been setup, just like processing from the GUI you can
also setup processing using the CompressonatorCLI command line tool. The
Command line tool has more options that are currently not supported on
the GUI application. While 3D mesh process is not supported on Command
line tool yet.

Once the Batch file are generated from the GUI, it can be edited to
include more options used by the command line tool. This also
facilitates automated generation of compressed files from many source
textures.

Steps to generate a batch file:

-  Creating a project file and set up the desired destination settings.

-  Open the file menu and select “Export to batch file…”

-  Select a destination file name and location to generate a batch file
   that can be run from command line.

-  Select Save.

The batch file requires the CompressonatorCLI.exe, support file folders
and DLL to be present on the same location.

The following files are required to run CompressonatorCLI.exe with the
batch file:

… Include all files and subfolders under that folder

+-------------------------+---------------------------------------+
| \\plugins\\...          | Qt Windows DLL and Qt Image Plugins   |
+=========================+=======================================+
| CompressonatorCLI.exe   | Command line Application              |
+-------------------------+---------------------------------------+
| qt.conf                 | Specifies the plugin folder           |
+-------------------------+---------------------------------------+
| Qt5Gui.dll              | Qt Run time DLL’s                     |
+-------------------------+---------------------------------------+
| Qt5Core.dll             |                                       |
+-------------------------+---------------------------------------+
| libGLESv2.dll           |                                       |
+-------------------------+---------------------------------------+
| libEGL.dll              |                                       |
+-------------------------+---------------------------------------+

\`

.. |image9| image:: media/image9.png
