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

#include "Image.h"
#define  FREEIMAGE_LIB
#include "FreeImage.h"
#include <iostream>

Image::Image(const char *image_path, GLuint textureUnit, bool& success) {
	success = true;
	FreeImage_Initialise();
	texture_unit = textureUnit;
	// Load Texture
	FREE_IMAGE_FORMAT image_format = FreeImage_GetFileType(image_path,0);
	FIBITMAP* image_bgr = FreeImage_Load(image_format, image_path);
	if (!image_bgr) {
		fprintf(stdout, "Failed to load image %s", image_path);
		success = false;
	}
	FIBITMAP* temp = image_bgr;
	image_bgr = FreeImage_ConvertTo32Bits(image_bgr);
	FreeImage_Unload(temp);
	im_width = FreeImage_GetWidth(image_bgr);
	im_height = FreeImage_GetHeight(image_bgr);
	GLubyte* image_texture = new GLubyte[4*im_width*im_height];
	char* image_pixeles = (char*)FreeImage_GetBits(image_bgr);
	//FreeImage loads in BGR format, so you need to swap some bytes(Or use GL_BGR).
	for(int j= 0; j<im_width*im_height; j++){
		image_texture[j*4+0]= image_pixeles[j*4+2];
		image_texture[j*4+1]= image_pixeles[j*4+1];
		image_texture[j*4+2]= image_pixeles[j*4+0];
		image_texture[j*4+3]= image_pixeles[j*4+3];
	}
	FreeImage_DeInitialise();
	// Create one OpenGL texture
	glGenTextures(1, &texture_handle);
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_2D, texture_handle);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, im_width, im_height, 0, GL_RGBA,GL_UNSIGNED_BYTE,(GLvoid*)image_texture );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,0);
}

int Image::getWidth() {
	return im_width;
}

int Image::getHeight() {
	return im_height;
}

GLuint Image::getTextureHandle() {
	return texture_handle;
}

GLuint Image::getTextureUnit() {
	return texture_unit;
}
/*
GLubyte* Image::getImageData() {
	return image_texture;
}
*/
GLenum Image::deleteImage() {
	glGetError();
	glDeleteTextures(1,&texture_handle);
	return glGetError();
}