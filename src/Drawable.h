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

#ifndef DRAWABLE_H
#define DRAWABLE_H

/** 
 * @file Drawable.h 
 * Header file defining the Drawable interface.
 */

#include <GL\glew.h>
/**
* The Drawable interface.
* Classes derived from this interface can be plugged into the PixelArtRenderer
* @see PixelArtRenderer
*/
class Drawable {
	public:
		/**
		* Used to draw to an opengl context
		* @param windowWidth viewport width
		* @param WindowHeight viewport height
		*/
		virtual void draw(int windowWidth, int WindowHeight) = 0;
};

#endif