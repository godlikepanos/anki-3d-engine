Property View
=============
Properties such as file size and image format of a selected item on the
Project Explorer are displayed on the Property View. Content of the
Property View changes when different items are selected in the
application.


Properties
----------

The Properties View will display information on the selected image’s
location, various sizes, dimensions, etc.

|image49|

Property View Window

The Property View above shows that the selected image is set to compress
the original Ruby.bmp image using BC7 compression format, the expected
quality of the resulting image is shown as default 0.05, this value
ranges from 0 to 1. Lower quality values will have faster processing
time and less amount of precision when compared to the original image.

Warning: For some large images, setting quality values above 0.75, the
time to process it may take several hours for only a marginal increase
in overall quality when compared to the original image.

When a compression process is completed, the Property View will indicate
the time it took to compress the image and the Compression Ratio. To see
the Compression Ratio, click on compressed image in the Project
Explorer, this will update the Compression Ratio which indicates how
much the image was compressed compared to the original (typically 4x for
BC7)

|image50|

Property View Window showing Compression Ratio

.. |image49| image:: media/image48.png
.. |image50| image:: media/image52.png

