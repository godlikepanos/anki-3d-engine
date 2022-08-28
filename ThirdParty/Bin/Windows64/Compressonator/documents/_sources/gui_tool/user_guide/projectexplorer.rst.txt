Project Explorer
================

Images and models are added to the application using the Project Explorer.

2D texture image files are displayed in two levels, original source
image is shown on the left (branch item) and destination image(s) (sub
items) are shown on the right using a tree view outline.

3D model files (only support .obj and .gltf with .bin) are displayed in
three levels, original model is shown on the left (branch item) and
destination model is shown as 2\ :sup:`nd` level sub item while the
mesh/textures within the model is shown as 3\ :sup:`rd` level item of
the tree view.


Projects
--------

The application uses a project based concept, where 2D texture images
are added to the Project Explorer tree view as original image items in
which settings are applied using a destination item. Each original 2D
texture image item can have multiple destination items. A destination
item can be set to generate a file with a specified format (compressed,
decompressed or transcoded) and extension (DDS, KTX, BMP, etc.)

While for 3D model items, they are added to the Project Explorer tree
view as original model items, in which multiple model destination
settings can be added as 2\ :sup:`nd` level tree which create multiple
resulted model items, the 3\ :sup:`rd` level destination setting which
applied to the mesh/texture items within the model can only be added
once per mesh/texture item.

Multiple destination items can be processed at the same time.

Projects can be loaded, created and saved to disk at any time.

Sample Projects
~~~~~~~~~~~~~~~

These samples can be accessed either from the Welcome Page or from the
sample projects folder

Compressonator\\Projects

+--------------------------+----------------------------------------------------------------------------------------------------------------------------+
| BC7\_Compression.cprj    | Project file demonstrating compression using BC7 on images with source extension formats of BMP, PNG, TGA and TIF          |
+==========================+============================================================================================================================+
| BC6H\_Compression.cprj   | Project file demonstrating compression using BC6H on a high dynamic range image (OpenEXR) file extension format of (EXR)   |
+--------------------------+----------------------------------------------------------------------------------------------------------------------------+

**Processing Ruby.bmp sample using BC7 Compression**

1. On the Welcome Page tab window as shown in the view below, click on
   “BC7\_Compression”

\ |image40|

The Project Explorer will change and show some sample images and
settings from the BC7 Compression sample project:

|image41|

2. Select the image by clicking on the name (for example, Ruby.bmp), the
   Properties View will now display information on the selected image’s
   location, various sizes, dimensions, etc.

|image42|

3. Now click on the right arrow next to the Ruby.bmp.

|image43|

This expands the view and you will see a clickable “Add destination
settings …” line and a BC7 pre-compressed destination sample
Ruby\_bmp\_1.

|image44|

4. Click on Ruby\_bmp\_1, and notice that the Property View changed (as
   shown below) to indicate what settings has been preset for
   Ruby\_bmp\_1

|image45|

Note that Compression Ratio and Compression Time both show “Unknown” and
“Not Processed”. These values will be updated when the destination file
is created during processing.

5. Click on the Process button located in the Properties View. Two new
   windows will open a Progress Window and a Message Output window. When
   processing is complete the progress window will close and the Output
   window will show a result.

|image46|

Notice also that there is a green circle next to Ruby\_bmp\_1,
indicating that a compressed file has been created and the process was
successful.

|image47|

6. To view the resulting file, double click on Ruby\_bmp\_1

|image48|


Add Destination Settings
------------------------

To add new destination settings a for specific original image (branch
item), expand its branch and select Add destination settings… by double
clicking on it.

|image59|

A new window will be displayed

|image60|

Add Destination Settings Window

Once you have set the desired options, the destination file name and
folder; select save. This will now add the new item to the Project
Explorer view.

|image61|

**Note:** In some cases, a red circle with a cross is displayed
indicating that a file already exists and will be overwritten with the
new settings. The current release does not check for duplications during
setting.

Using Codec Quality Settings
----------------------------
BC1, BC2, and BC3 have discrete quality settings, These settings are available in the following ranges (varying the q setting in these ranges will have no new effects, q is a discrete coarse setting)

.. code-block:: c

   q = 0.0     to 0.01 sets lowest quality and fast compression
   q = 0.101 to 0.6 sets mid-quality
   q = 0.601 to 1,0 set the best quality and low performance  
   BC4 and BC5 have no quality settings, no changes in quality will occur if set.
   BC6 & BC7 have full q ranges from 0 to 1.0


Setting Global Quality Settings
-------------------------------

Users can override all individual destination compression settings, using a globally set value before processing

Currently, only the quality settings can be overwritten with a new global setting.

**The process is as follows:**

On the project explorer click on "Double Click Here to add files"

|QualitySetting1|

A new property view will be displayed

|QualitySetting2|

Set a new Quality value to override all existing quality settings for textures in the project explorer, a value of 0 with restore the old values and disable the global settings

|QualitySetting3|

When an override is set the textures will display the new override setting and disable its editing features.

Notice also that the "Double Click Here to add files" background color has also changed to indicate that an override setting is in effect, it will return to a white background if the override settings are turned off.

.. |image40| image:: media/image43.png
.. |image41| image:: media/image44.png
.. |image42| image:: media/image45.png
.. |image43| image:: media/image46.png
.. |image44| image:: media/image47.png
.. |image45| image:: media/image48.png
.. |image46| image:: media/image49.png
.. |image47| image:: media/image50.png
.. |image48| image:: media/image51.png
.. |image59| image:: media/image65.png
.. |image60| image:: media/image66.png
.. |image61| image:: media/image67.png
.. |QualitySetting1| image:: media/globalsettings2.png
.. |QualitySetting2| image:: media/globalsettings3.png
.. |QualitySetting3| image:: media/globalsettings5.png

