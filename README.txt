DEPIXELIZING PIXEL ART ON GPUS - http://www.cg.tuwien.ac.at/research/publications/2014/KREUZER-2014-DPA/
based on: http://research.microsoft.com/en-us/um/people/kopf/pixelart/

THIS SOFTWARE WAS TESTED ON WINDOWS WITH NVIDIA GPUs ONLY!
IT WILL NOT RUN ON INTEL GPUs

This application is intended to be a preview for my gpu-supported implementation of the DEPIXELIZING PIXEL ART algorithm.
You can either load a single image, or a sequence of images(images in a sequence must have the same resolution).
The Window title display the current rendering speed. See section controls below for usage.

========== BUILDING FROM SOURCE: ==========

Requirements:
CMAKE(http://www.cmake.org/) to configure and generate a Visual Studio Solution.

This Project requires boost::program_options
easy way to get it:
download boost source from boost.org
set BOOST_ROOT environment variable to the path which contains the boost source.
open up a command prompt and enter the following commands:

cd %BOOST_ROOT%
bootstrap
b2 --with-program_options

...and you're done setting up boost for this project.

Then open CMAKE and set the source directory to this directory. Choose any binary directory you want and give it a go.

========== STARTUP: ==========
The main executable is located in the \bin directory

Usage: GPUPixelArt.exe [options]
Allowed options:
  --help                      produce help message
  -I [ --input-file ] arg     input file
  -S [ --input-sequence ] arg input seqence name
  --sequence-count arg        input seqence count
  --sequence-fps arg (=15)         input seqence playback fps
  --height arg (=600)         output vertical size in pixel

Use either the -I or the -S options!
If you want to use the sequence option -S then do not forget to add the --sequence-count and --sequence-fps arguments!

Make a release build and extract examples.zip to a directory above the binary directory.
Feel free to try the startXXX.bat files. You can use them to start the supplemented image sequences.
I do not own any copyrights to the pictures in examples.zip (copyrights owned by Nintendo Co., Ltd.)

========== CONTROLS: =========
SPACE 			... pause sequence
MOUSEWHEEL 		... control zoom
LEFT_SHIFT + MOUSEWHEEL	... change sequence playback speed

DEBUGGING:
F1		... draw depixelized Pixel Art (default)
F2		... draw using Nearest Neighbor Interpolation
F3		... draw Simplified Voronoi Regions
F4		... draw B-Splines
F5		... toggle Optimization (has effect on F1&F4 mode)
F6		... toggle debug graph overlays (effects F2&F3 mode)

(C) Felix Kreuzer, falichs@gmail.com