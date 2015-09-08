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

#ifndef SIMILARITYGRAPHBUILDERFS_H
#define SIMILARITYGRAPHBUILDERFS_H

/** 
 * @file SimilarityGraphBuilderFS.h 
 * Header file defining the SimilarityGraphBuilderFS class.
 */

#include "SimilarityGraphBuilder.h"

class Image;

/**
* Fragment Shader based implementation of the SimilarityGraphBuilder interface.
* This class is used to create a full Similarity Graph from a given Pixel Art image. 
* The draw method is actually split up into four passes:
* 1st pass: create an initial similarity graph using pixel color similarity information.
* 2nd pass: compute valences based on the graph created in the 1st pass.
* 3rd pass: remove unwanted crossing edges from the similarity graph.
* 4th pass: update valences based on the graph created in the 3rd pass.
* Therefore we will use two texture units which allow us to read and write to two textures in a ping-pong fashion:
* 1st pass: writes to texture unit 1.
* 2nd pass: reads from texture unit 1, writes to texture unit 2.
* 3rd pass: reads from texture unit 2, writes to texture unit 1.
* 4th pass: reads from texture unit 1, writes to texture unit 2.
* Thus the final graph is stored in texture unit 2.
*/
class SimilarityGraphBuilderFS : public SimilarityGraphBuilder {
	public:
		/**
		* Constructor
		* @param pixelArt the source image
		* @param DrawBuffers This GLenum manages our color attachments.
		* @param TextureUnit1 the first texture unit ID (actually offset, since a texture unit is an enum).
		* @param TextureUnit2 the second texture unit ID (actually offset, since a texture unit is an enum).
		*/
		SimilarityGraphBuilderFS(Image* pixelArt, GLenum* DrawBuffers, GLuint TextureUnit1, GLuint TextureUnit2);
		
		/**
		* Destructor.
		* cleans up allocated opengl objects.
		*/
		~SimilarityGraphBuilderFS();
		
		void draw();
		
		void draw(int windowWidth, int WindowHeight){draw();};

		/**
		* Allows to set the input Pixel Art image. beware - size must not change!
		* @param pixelArt the source image
		*/
		void setPixelArt(Image* pixelArt);

		GLuint getTexID();
		GLuint getTexUnit();

	protected:
		int init();
		
	private:
		GLuint m_vaoID_similarityGraph_UVquad;
		
		GLuint m_bufferID_similarityGraph_vertices;
		GLuint m_bufferID_similarityGraph_UVs;
		
		GLuint m_texUnit_similarityGraph;
		GLuint m_texID_similarityGraph;
		
		GLuint m_texUnit_updatedSimilarityGraph;
		GLuint m_texID_updatedSimilarityGraph;

		GLuint m_programID_indifferentColors;
		GLuint m_uniformID_indifferentColors_pixelArt;

		GLuint m_programID_valenceUpdate;
		GLuint m_uniformID_valenceUpdate_similarityGraph;

		GLuint m_programID_eliminateCrossingDiagonals;
		GLuint m_uniformID_eliminateCrossingDiagonals_similarityGraph;

		GLenum* m_DrawBuffers;

		GLuint m_fbo1;

		Image* m_pixelArtImage;
};

#endif