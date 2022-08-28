Introduction
============

This guide provides detailed information on the Compressonator application. It lists the requirement needed for running the application and helps in installation, getting started with the tool, using the sample projects and finding specific topics of interest.

Compressonator GUI features
===========================

* The GUI interacts with Compressonator SDK for texture compression and bit format conversions; it can compress a wide range of compression formats including ASTC, ATC, ATInN, BCn, ETCn, DXTn, swizzle DXTn formats. 
* Supports conversion of textures with 32 bit fixed and float formats.
* Process multiple compression, decompression and transcode of images with a single processing action.
* Allow multiple processing interactions for a single source image
* Inspect visually and analytically compression results.
* Uses a single image viewer that supports a large number of compressed and uncompressed image formats.
  
Application Requirements
========================
  
* Application was tested on Windows 10 platform. 
* For best performance, a multi core CPU system with the latest AMD Graphics card are required.
* For ease of use, a wheel based mouse is essential.
* Use of multiple monitors is optional, and can be beneficial when examining multiple image views at the same time.
  Installation
* To install the application, download the executable file from release page to your system. Then, double click the executable to start the installation process and follow the on-screen instructions.
* Run the application in Administrator mode, recommended that UAC be turned off.

Getting started (Using sample projects)
=======================================

This section shows you how to get started with the application using the sample projects that come with the installation. Note that you can also start by creating a new project and add your image files for compression. 

1. From the start menu or desktop shortcut run the application  
2. You will see the following view

.. image:: media/compressonator_window.jpg

3. On the Welcome Page tab Window, click on “BC7_Compression”, the Project Explorer will change and show some sample images and settings

.. image:: media/compressonator_project_explorer.jpg

4. Click on Ruby.bmp file and you will see an Image View window show up on the right of the Project Explorer, tabbed with the Welcome Page window (as shown below).

.. image:: media/compressonator_properties_view.jpg

The Properties View will now display information on the selected image’s location, various sizes, dimensions, etc. 

5. Now click on the right arrow next to the Ruby.bmp.

.. image:: media/rubybmp.jpg

This expands the view and you will see a clickable “Add destination settings …” line and a BC7 pre-compressed destination sample Ruby_1

.. image:: media/ruby_1view.jpg

6. Click on Ruby_1, and notice that the Properties View changed (as shown below) to indicate what settings has been preset for Ruby_1

.. image:: media/compression_setting.jpg

Ruby_1 is set to compress the original Ruby.bmp image using BC7 compression format, the expected quality of the resulting image is shown as default 0.05,
this value ranges from 0 to 1. Note that lower quality value will have faster compression process with less amount of precision when compared to the original. 
On the other hand, higher quality value will slow down the compression process but produce better image quality.

7. To start the compression, click on “Process” button, a Progress windows and an Output window appear.

.. image:: media/compressonator_process_window.jpg

8. When the compression process done, the Project Explorer will change to indicate the status of the resulting compressed Ruby_1 image with a small green (succeeded) or red circle (failed),
and the Output window will indicate additional information on the succeeded or failed compression process.

.. image:: media/output.png
 
For this sample, we should see a green circle next to Ruby_1 (compression succeeded)

.. image:: media/compressonator_tree_view.jpg

9. Now the Properties View will indicate the time it took to compress the image.
To see the Compression Ratio, click on RUby_1 again, this will update the Compression Ratio which indicates how much the image was compressed compared to the original (typically 4x)

.. image:: media/compressonator_properties.jpg

10. To see the resulting compressed image, single click on Ruby_1 and you will see the image as shown below.

.. image:: media/ruby_compress_win_view.jpg

11. To view the difference between processed image (Ruby_1) and original image (Ruby.bmp), right click on Ruby_1 and select View Image Diff from the context menu 

.. image:: media/view_img_diff.jpg

You will now see a comparison of the original image with the compressed image

.. image:: media/image_diff_view.jpg

12. In addition, you can run analysis on the images that show various statistics such as MSE, PSNR and Similarity Indices (SSIM) by selecting 

.. image:: media/difficon.png
   
When analysis process completed, the statistics result will be shown on the Properties View:
 
.. image:: media/analysis_properties.jpg  

Closing views
=============
At any time you can close various views by selecting the close button (on tabs or windows)

Changing views
==============
Click on Image View window tab titles or click on any image on Project Explorer.

Delete or Remove Image(s)
=========================
Select the image in Project Explorer and press DEL key. A message window will pop up as shown below.

.. image:: media/del_remove_window.jpg

Remove	Will remove the selected items from the project explorer view, keeping the files on disk.

Delete	Will Remove and delete files on disk.<br>
Cancel	Will return you back to the application (similar applies when selecting the red close button)


Closing and Saving the Project 
==============================
Select File menu and then Exit or Click on the close button on the application window
Since we have no changes to the settings or added any new images, the application will simply close when exit.
If you made any changes to the sample project “BC7_Compression” the application will prompt to save the changes or discard them. 
When you select save the old settings for “BC7_Compression” will be overwritten.


.. image:: media/save_view.jpg




