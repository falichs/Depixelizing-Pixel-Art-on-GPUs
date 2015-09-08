#ifndef SIMILARITYGRAPHBUILDER_H
#define SIMILARITYGRAPHBUILDER_H

/** 
 * @file SimilarityGraphBuilder.h 
 * Header file defining the SimilarityGraphBuilder interface.
 */

#include "Drawable.h"
/**
* The Similarity Graph builder interface.
* Classes derived from this interface are intended to create the similiarity graph buffer.
* @see PixelArtRenderer
* @see Drawable
*/
class SimilarityGraphBuilder : public Drawable {
	public:
		/**
		* Use this function to compute the Similaritygraph.
		* This will be called if you call the Drawable::Draw(int, int) member.
		* Viewport dimensions are not relevant for SimilarityGraph construction.
		*/
		virtual void draw() = 0 ;

		/**
		* Returns the Texture ID containing the resulting Similarity Graph 
		* @return texture ID
		*/
		virtual GLuint getTexID() = 0 ;

		/**
		* Returns the Texture Unit associated with resulting the Similarity Graph 
		* @return texture Unit
		*/
		virtual GLuint getTexUnit() = 0 ;
};

#endif