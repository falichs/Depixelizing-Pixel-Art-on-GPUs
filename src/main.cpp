// Copyright (c) 2015 Felix Kreuzer
//
// This source is subject to the MIT License.
// Please see the LICENSE.txt file for more information.
// All other rights reserved.
//
// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include "PixelArtRenderer.h"

#include <boost/program_options.hpp>
using namespace boost;
namespace po = boost::program_options;

#include <algorithm>
#include <iterator>
using namespace std;

/** @file main.cpp */ 

/**
* Prints command line options to console
*/
void printUsage() {
	std::cout << "Usage is -in <infile>\n";
}

/**
* An operator function for using BOOST::PROGRAM_OPTIONS.
*/
template<class T>
ostream& operator<<(ostream& os, const vector<T>& v)
{
    copy(v.begin(), v.end(), ostream_iterator<T>(os, " ")); 
    return os;
}
/**
* Handles input parameters and controls the main-loop.
* Command-line parameters are parsed using BOOST::PROGRAM_OPTIONS. 
* After that the PIXELARTRENDERER class gets instantiated and initialized. 
* The main-loop orders the Renderer to draw frames and measures the time between them. 
* The only way to break the loop is by pressing the ESC key or closing the OS-window.
* @see PixelArtRenderer
*/
int main( int argc, char* argv[] )
{
	int height;
	string inputPath;
	string sequence_name; //if fed with sequence
	int sequence_count = 0; //if fed with sequence
	float sequence_fps = 1; //if fed with sequence
	bool useSequence = false;
	po::options_description desc("Allowed options");
	try {
        desc.add_options()
            ("help", "produce help message")
            //("input-file", po::value< vector<string> >(), "input file")
			("input-file,I", po::value<string>(&inputPath), "input file")
			("input-sequence,S", po::value<string>(&sequence_name), "input seqence name")
			("sequence-count", po::value<int>(&sequence_count), "input seqence count")
			("sequence-fps", po::value<float>(&sequence_fps)->default_value(15.0f), "input seqence frames per second")
			("height", po::value<int>(&height)->default_value(600), 
                  "output vertical size in pixel")
        ;
        po::positional_options_description p;
        p.add("input-file", -1);
		p.add("input-sequence", 3);
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).
                  options(desc).positional(p).run(), vm);
        po::notify(vm);
    
        if (vm.count("help")) {
            cout << "Usage: GPUPixelArt.exe [options]\n";
            cout << desc;
            return 0;
        } else if (vm.count("input-file"))
        {
			//cout << "Input files are: " << vm["input-file"].as< vector<string> >() << "\n";
			
			cout << "INPUT: " << inputPath << "\n";
        }  else if (vm.count("input-sequence"))
        {
			//cout << "Input files are: " << vm["input-file"].as< vector<string> >() << "\n";
			useSequence = true;
			cout << "INPUT: sequence:" << sequence_name << ", frame count:" << sequence_count << ", fps:" << sequence_fps <<".\n";
        } else {
			cout << "Usage: GPUPixelArt.exe [options]\n";
            cout << desc;
            return 0;
		}

        cout << "Height is " << height << "\n"; 
		       
	}
    catch(std::exception& e)
    {
        cout << e.what() << "\n";
		cout << "Usage: GPUPixelArt.exe [options]\n";
            cout << desc;
        return 1;
    }
	    
    PixelArtRenderer* renderer = PixelArtRenderer::getInstance();
		
	int errorcode = renderer->initGraphics();
	if (errorcode != 0) {
		return 1;
	}
	renderer->resizeFun(renderer->getWindow(),0,height);
	if(useSequence) {
		if( !(renderer->loadPixelArtSequence(sequence_name, sequence_count, sequence_fps))) {
			return 1;
		}

	} else {
		if(!(renderer->loadPixelArt(inputPath.c_str())))
			return 1;
	}
		
	if(!renderer->initConstructionContent()) {
		return 1;
	}

	//glfwEnable(GLFW_STICKY_KEYS);
	double lastTime = glfwGetTime();
	int nbFrames = 0;
	double sequence_last_frame_time = lastTime;     
	do{
		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1 sec ago
			// printf and reset timer
			char title[100];
			snprintf(title,100, "Depixelizing PixelArt On GPU - %f ms/frame - ", 1000.0/double(nbFrames) );
			glfwSetWindowTitle(renderer->getWindow(), title);
			nbFrames = 0;
			lastTime += 1.0;
		}
		
		if(useSequence) {
			
			renderer->sequenceLoadFrame(currentTime);
		}	
		renderer->drawFrame(currentTime);
		
	} // Check if the ESC key was pressed or the window was closed
	while( !glfwWindowShouldClose(renderer->getWindow()));

		

	// Cleanup
	renderer->~PixelArtRenderer();
	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	
	return 0;
}

