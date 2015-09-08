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

#ifndef VORONOICELLGRAPH3X3_H
#define VORONOICELLGRAPH3X3_H

/** 
 * @file VoronoiCellGraph3x3.h 
 * Header file defining the VoronoiCellGraph3x3 DEBUG class.
 */

#include "Drawable.h"

class Image;
class SimilarityGraphBuilderFS;
/**
* DEBUG: This class is used to visualize the simplified Voronoi cells utilized in control-point generation.
*/
class VoronoiCellGraph3x3 : public Drawable {
	public:
		VoronoiCellGraph3x3(Image* pixelArt, GLenum* DrawBuffers,SimilarityGraphBuilderFS* simGraph);
		~VoronoiCellGraph3x3();
		void draw(int windowWidth, int WindowHeight);
		void drawOverlay(int windowWidth, int WindowHeight);
		void setPixelArt(Image* pixelArt);
		void setSimilarityGraph(SimilarityGraphBuilderFS* simGraph);

		/**
		* sets the zoom window parameters.
		* zoom factor and window coordinates range from 0.0 to 1.0.
		* no bounds checking performed here.
		* @param zoomFactor the size of the zoom region
		* @param zoomWindow_x the x coordinate of the zoom window center
		* @param zoomWindow_y the y coordinate of the zoom window center
		*/
		void setZoom(float zoomFactor, float zoomWindow_x, float zoomWindow_y) {
			m_zoomFactor_inv = 1.0f/zoomFactor;
			m_zoomWindow_x = -(zoomWindow_x - 0.5f) * 2.0f * m_zoomFactor_inv; 
			m_zoomWindow_y = -(zoomWindow_y - 0.5f) * 2.0f * m_zoomFactor_inv;
		};
	protected:
		int init();
		int initOverlay();

	private:
		
		GLuint m_vaoID_dbgDraw_voronoiCellGraph3x3;

		GLuint m_bufferID_dbgDraw_voronoiCellGraph3x3;

		GLuint m_programID_debugDrawVoronoiCellGraph3x3;
		GLuint m_uniformID_debugDrawVoronoiCellGraph3x3_pixelArt;
		GLuint m_uniformID_debugDrawVoronoiCellGraph3x3_similaryityGraph;

		GLuint m_programID_VoronoiCellOverlay;
		GLuint m_uniformID_VoronoiCellOverlay_pixelArt;
		GLuint m_uniformID_VoronoiCellOverlay_similaryityGraph;

		SimilarityGraphBuilderFS* m_simGraph;

		GLenum* m_DrawBuffers;

		Image* m_pixelArtImage;

		GLuint m_uniformID_debugDrawVoronoiCellGraph3x3_uf_zoomFactor;
		GLuint m_uniformID_debugDrawVoronoiCellGraph3x3_uv2_zoomWindowCenter;
		GLuint m_uniformID_VoronoiCellOverlay_uf_zoomFactor;
		GLuint m_uniformID_VoronoiCellOverlay_uv2_zoomWindowCenter;
		float m_zoomFactor_inv;
		float m_zoomWindow_x;
		float m_zoomWindow_y;
};

#endif