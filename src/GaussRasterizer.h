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

#ifndef GAUSSRASTERIZER_H
#define GAUSSRASTERIZER_H

/** 
 * @file GaussRasterizer.h 
 * Header file defining the GaussRasterizer class.
 */

#include "Drawable.h"

class Image;
class CellGraphBuilder;
/**
* This class is used to rasterize the Depixelized Pixel Art.
*/
class GaussRasterizer : public Drawable {
	public:
		GaussRasterizer(Image* pixelArt, GLenum* DrawBuffers, CellGraphBuilder* cellGraph);
		~GaussRasterizer();
		void draw(int windowWidth, int WindowHeight);
		void setPixelArt(Image* pixelArt);

		/**
		* sets the zoom window parameters.
		* zoom factor and window coordinates range from 0.0 to 1.0.
		* no bounds checking performed here.
		* @param zoomFactor the size of the zoom region
		* @param zoomWindow_x the x coordinate of the zoom window center
		* @param zoomWindow_y the y coordinate of the zoom window center
		*/
		void setZoom(float zoomFactor, float zoomWindow_x, float zoomWindow_y);

	protected:
		int init();
		
	private:
		GLuint m_vaoID_gaussRasterizer_UVquad;
		
		GLuint m_bufferID_gaussRasterizer_vertices;
		//GLuint m_bufferID_gaussRasterizer_UVs;

		GLuint m_programID_gaussRasterizer;
		GLuint m_uniformID_gaussRasterizer_pixelArt;
		GLuint m_uniformID_gaussRasterizer_indexedCellPositions;
		GLuint m_uniformID_gaussRasterizer_neighborIndices;
		GLuint m_uniformID_gaussRasterizer_CPflags;
		GLuint m_uniformID_gaussRasterizer_viewportDimensions;
		GLuint m_uniformID_gaussRasterizer_uf_zoomFactor;
		GLuint m_uniformID_gaussRasterizer_uv2_zoomWindowCenter;

		GLenum* m_DrawBuffers;

		CellGraphBuilder* m_cellGraph;
		Image* m_pixelArtImage;

		float m_zoomFactor;
		float m_zoomWindow_x;
		float m_zoomWindow_y;
};

#endif