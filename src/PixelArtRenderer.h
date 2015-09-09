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

#ifndef PIXELARTRENDERER_H
#define PIXELARTRENDERER_H

/** 
 * @file PixelArtRenderer.h 
 * Header file defining the PixelArtRenderer class.
 */

#include <GL/glew.h>
#include <GLFW/glfw3.h>
//#include <deque>
#include <vector>
#include <string>
/**
 * Switch Rendering Mode.
 * This enum is used to switch between the rendering modes. 
 * Except the GAUSSRASTERIZER these modes are mainly used for debugging.
 */
enum renderMode { 
	SIMILARITYGRAPH, /**< enum value SIMILARITYGRAPH. Tells the PixelArtRenderer to draw the similarity graph*/
	VORONOIGRAPH3x3, /**< enum value VORONOIGRAPH3x3. Tells the PixelArtRenderer to draw the voronoi-cell graph*/
	FULLCELLGRAPH, /**< enum value FULLCELLGRAPH. Tells the PixelArtRenderer to draw the spline-graph graph*/
	GAUSSRASTERIZER, /**< enum value GAUSSRASTERIZER. Tells the PixelArtRenderer to draw the depixelized image*/
	COUNT = 4
	};

class SimilarityGraphBuilderFS;
class VoronoiCellGraph3x3;
class CellGraphBuilder;
//class HoffRasterizerGS;
class SimilarityGraphDebugToy;
class GaussRasterizer;
//class CellSpaceGrid;
class Image;

/**
 * PixelArtRenderer SINGLETON.
 * This class represents the heart of the rendering client. 
 * It’s main responsibilities are to initialize the graphics context and to draw each frame to the backbuffer.
 */
class PixelArtRenderer {
	
	
public:
	/**
    * Use this to access the object instance.
    * @return The singleton object instance
    */
	static PixelArtRenderer* getInstance();

	/**
     * PixelArtRenderer class destructor 
	 */
	~PixelArtRenderer();

	/**
    * Call this once im order to initialize the graphics context.
	* @return 0 if initialization was successfull, -1 if not.
    */
	int initGraphics();

	/**
    * get the current context window
	* @return the GLFW window.
    */
	GLFWwindow* getWindow(){return m_window;}

	/**
    * Instruct PixelArtRenderer to draw a frame and switch buffers. 
	* @param lastFrameTime GLFW time
    */
	void drawFrame(double time);

	/**
    * Use this to load a Pixel Art image from file.
    * @param filename Path to file.
    */
	bool loadPixelArt(const char* filename);

	bool loadPixelArtSequence(const std::string filename, int count, float fps);
	//void sequenceStepFrame();
	void sequenceLoadFrame(double time);

	/**
    * Use this static member to change the viewport
	* @param window The window that received the event.
    * @param w Viewport Width.
	* @param h Viewport Height.
    */
	static void resizeFun(GLFWwindow* window, int w, int h);

	/**
    * Keyboard callback registered with GLFW
    * @param keyID See GLFW Documentation for details.
	* @param keyState See GLFW Documentation for details.
    */
	static void cbfun(GLFWwindow* window, int key, int scancode, int action, int mods);

	/**
    * Mousewheel callback registered with GLFW.
    * @param window The window that received the event.
	* @param xoffset The scroll offset along the x-axis.
	* @param yoffset The scroll offset along the y-axis.
    */
	static void mwfun(GLFWwindow* window, double xoffset, double yoffset);

	/**
    * Allows to switch the renderMode.
	* @see renderMode
    * @param mode renderMode.
    */
	void switchRenderMode(renderMode mode);

	/**
    * Switches trough the rendermodes in a circular fashion.
	* @see renderMode
    */
	void toggleRenderMode();

	/**
    * Toggles debug overlays where available.
    */
	void toggleOverlay();

	/**
    * Toggles whether to optimize splines or not.
    */
	void toggleOptimization();

	/**
    * Call this once after loading the first image.
	* @return true if everything went well.
    */
	bool initConstructionContent();

	/**
    * DEBUG: Allows to change the spline optimization offset-parameter
    * @param delta @see CellGraphBuilder.
    */
	void changeOffset(float delta);

	/**
    * DEBUG: Allows to change the spline optimization penalty-parameter
    * @param delta @see CellGraphBuilder.
    */
	void changePositionalPenalty(int delta);

	/**
    * Calling this will write the next frame to disk. 
    */
	void saveNextFrame();

private:
	/**
     * PixelArtRenderer class constructor.
     * This constructor will be called once by the singleton.
	 * It initializes parameters like aspect-ratio and resolution with default parameters.
     */
	PixelArtRenderer();

	/**
    * Reads the Backbuffer and saves it as a bitmap to disk.
    */
	void saveFrame();

	/** 
       * The singleton pointer.
       */
	static PixelArtRenderer* singletonPointer;

	
	GLFWwindow* m_window;
	//texture units
		GLuint texUnit_pixelArt;
		GLuint texUnit_similarityGraph;
		GLuint texUnit_updatedSimilarityGraph;
		GLuint texUnit_indexedCellPositions;
		GLuint texUnit_CellFlags;
		GLuint texUnit_knotNeighbors;
		GLuint texUnit_optimizedPositions;
		GLuint texUnit_correctedPositions;
	//images
		Image* pixelArtImage;
	//drawBuffers
		GLenum* DrawBuffers;
	//Switches
		renderMode m_currentRenderMode;
		bool m_overlay;
		bool m_opt;
		bool m_isSequence;
		
	//others

		SimilarityGraphBuilderFS* m_similarityGraphBuilder;
		VoronoiCellGraph3x3* m_voronoiCellGraph3x3;
		CellGraphBuilder* m_cellGraphBuilder;
		//HoffRasterizerGS* m_hoffRasterizer;
		GaussRasterizer* m_gaussRasterizer;
		SimilarityGraphDebugToy* m_simGraphDebugToy;
		//CellSpaceGrid* m_cellSpaceGrid;

		//std::deque<Image*> m_img_deque;
		std::vector<Image*> m_img_vector;
		float m_sequence_fps;
		int m_sequence_count;
		//int m_mousewheel_diff;
	//ZOOM related
		void calculateZoomFactor();
		void calculateZoomWindowPosition();
		void setZoom(double lastFrameTime);
		void smoothZoom(double lastFrameTime);
		float m_zoom_factor;
		float m_zoom_factor_smoothed;
		double m_zoom_window_position_x; //holds the zoom windows position
		double m_zoom_window_position_x_smoothed;
		double m_zoom_window_position_y; //holds the zoom windows position
		double m_zoom_window_position_y_smoothed;
		double m_mouse_lastPos_x; //last frame's mouse position
		double m_mouse_lastPos_y; //last frame's mouse position
		bool m_mouse_grabbingIsActive;
	//TIMING
		double m_lastFrameTime;

	//save frame
		bool m_saveNextFrame;
};

#endif