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

#ifndef IMAGE_H
#define IMAGE_H

/** 
 * @file Image.h 
 * Header file defining the Image class.
 */

#include "GL/glew.h"

/**
* Image Class representing a Pixel Art image loaded from file.
*/
class Image {

public:
	/**
	* Image class constructor.
	* @param image_path path to Pixel Art image file
	* @param textureUnit specifies which texture unit the image is associated with.
	*/
	Image(const char* image_path, GLuint textureUnit);

	/**
	* Image class destructor.
	* calls the deleteImage member.
	* @see deleteImage
	*/
	~Image() {deleteImage();};

	/**
	* Fetch image width.
	* @return width in pixels
	*/
	int getWidth();

	/**
	* Fetch image height.
	* @return height in pixels
	*/
	int getHeight();

	//GLubyte* getImageData();

	/**
	* Returns the GPU texture handle associated with this Image.
	* @return texture handle
	*/
	GLuint getTextureHandle();

	/**
	* Returns the GPU texture unit identifier associated with this Image.
	* @return texture unit
	*/
	GLuint getTextureUnit();

	/**
	* Removes the texture from GPU memory.
	*/
	GLenum deleteImage();

private:

	GLubyte* image_texture;
	int im_width;
	int im_height;
	GLuint texture_handle;
	GLuint texture_unit;
};

#endif