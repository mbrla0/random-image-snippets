# Random Image Editing Snippets
Some random quickly-written programs that change pictures in a variety of ways

# The base library
All of the programs in this collection use a custom-made image format called GLT, a format that represents the
raw data used in composing a texture, along with a header, to specify the information related to the image.


The spec for the format, along with the C++ headers for manipulating it is located under the folder ```GLT/```

# Building the programs
All of the utilities/programs bundle the GLT headers with themselves, so, build them with their respective library folder.

# GLT Utilies
Utility programs for handling images in the GLT format:

  * glt-show: Displays a GLT image
  
  * glt-make: Converts an image from a format such as PNG or JPG into GLT
  
  * glt-get: Converts an image in GLT format to one in PNG

# Boundary Tracer
Traces the boundaries of an image in GLT format into white lines.

# Dismantler
A program for scrambling image data based on a given password, to the point where it becomes unidentifiable.
Along with another program, which reverses the process.
