#ifndef SIMILARITYGRAPHDEBUGTOY_H
#define SIMILARITYGRAPHDEBUGTOY_H

/** 
 * @file SimilarityGraphDebugToy.h 
 * Header file defining the SimilarityGraphDebugToy DEBUG class.
 */

#include <GL\glew.h>

class Image;
class SimilarityGraphBuilder;
/**
* DEBUG: This class is used to visualize the Similarity graph.
*/
class SimilarityGraphDebugToy {
	public:
		SimilarityGraphDebugToy(Image* pixelArt, SimilarityGraphBuilder* simGraph, GLenum* DrawBuffers);
		~SimilarityGraphDebugToy();
		void drawPixelArt(int windowWidth, int WindowHeight);
		void drawSimilarityGraphOverlay(int windowWidth, int WindowHeight);
		void setPixelArt(Image* pixelArt);
		/**
		* sets the zoom window parameters.
		* zoom factor and window coordinates range from 0.0 to 1.0.
		* no bounds checking performed here.
		* @param zoomFactor the size of the zoom region
		* @param zoomWindow_x the x coordinate of the zoom window center
		* @param zoomWindow_y the y coordinate of the zoom window center
		*/
		void setZoom(float zoomFactor, float zoomWindow_x, float zoomWindow_y) {m_zoomFactor = zoomFactor; m_zoomFactor_inv = 1.0f/zoomFactor;m_zoomWindow_x = zoomWindow_x; m_zoomWindow_y = zoomWindow_y;};
	protected:
		int init();
		void drawPixelCenters(int windowWidth, int WindowHeight);
		void drawPixelConnections(int windowWidth, int WindowHeight);

	private:
		
		GLuint m_vaoID_dbgDraw_similarityGraph;
		GLuint m_vaoID_similarityGraph_UVquad;
		
		GLuint m_bufferID_dbgDraw_similarityGraph;
		GLuint m_bufferID_similarityGraph_vertices;
		GLuint m_bufferID_similarityGraph_UVs;

		GLuint m_programID_debugDrawPixels;
		GLuint m_uniformID_debugDrawPixels_pixelArt;

		GLuint m_programID_debugDrawNodes;
		GLuint m_uniformID_debugDrawNodes_positionsRange;
		
		GLuint m_programID_debugDrawEdges;
		GLuint m_uniformID_debugDrawEdges_similarityGraph;

		GLenum* m_DrawBuffers;
		SimilarityGraphBuilder* m_simGraph;
		Image* m_pixelArtImage;

		GLuint m_uniformID_debugDrawEdges_uf_zoomFactor;
		GLuint m_uniformID_debugDrawEdges_uv2_zoomWindowCenter;
		GLuint m_uniformID_debugDrawNodes_uf_zoomFactor;
		GLuint m_uniformID_debugDrawNodes_uv2_zoomWindowCenter;
		GLuint m_uniformID_debugDrawPixels_uf_zoomFactor;
		GLuint m_uniformID_debugDrawPixels_uv2_zoomWindowCenter;
		float m_zoomFactor;
		float m_zoomFactor_inv;
		float m_zoomWindow_x;
		float m_zoomWindow_y;
};

#endif