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

#include "PixelArtRenderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//#include <vector>
#include <deque>
#include <sstream>
#include <iomanip>

#include "Image.h"
#include "Shader.h"
#include "SimilarityGraphBuilderFS.h"
#include "VoronoiCellGraph3x3.h"
#include "CellGraphBuilder.h"
//#include "HoffRasterizerGS.h"
#include "SimilarityGraphDebugToy.h"
#include "GaussRasterizer.h"
#define  FREEIMAGE_LIB
#include "FreeImage.h"
//#include "CellSpaceGrid.h"


	static int window_width= 800;
	static int window_heigth= 600;
	static float aspectRatio;
	//static int s_lastMWPos = 0;
	static double s_mousewheel_diff = 0.0;
	//static double s_mwOffset = 0.0;
	static bool s_sequenceIsRunning = true;
	static float s_deltaFPS;

PixelArtRenderer* PixelArtRenderer::singletonPointer = 0;

void PixelArtRenderer::resizeFun(GLFWwindow* window, int w, int h)
{
	window_width = h*aspectRatio;
	window_heigth = h;
	glfwSetWindowSize(window, window_width, window_heigth );
}

void PixelArtRenderer::cbfun(GLFWwindow* window, int key, int scancode, int action, int mods) {
	switch(key) {
	case GLFW_KEY_F1:
		if(action == GLFW_PRESS) {
			//PixelArtRenderer::getInstance()->toggleRenderMode();
			PixelArtRenderer::getInstance()->switchRenderMode(renderMode::GAUSSRASTERIZER);
		}
		break;
	case GLFW_KEY_F2:
		if(action == GLFW_PRESS) {
			PixelArtRenderer::getInstance()->switchRenderMode(renderMode::SIMILARITYGRAPH);
		}
	break;
	case GLFW_KEY_F3:
		if(action == GLFW_PRESS) {
			PixelArtRenderer::getInstance()->switchRenderMode(renderMode::VORONOIGRAPH3x3);
		}
		break;
	case GLFW_KEY_F4:
		if(action == GLFW_PRESS) {
			PixelArtRenderer::getInstance()->switchRenderMode(renderMode::FULLCELLGRAPH);
		}
		break;
	case GLFW_KEY_F5:
		if(action == GLFW_PRESS) {
			PixelArtRenderer::getInstance()->toggleOverlay();
		}
		break;
	case GLFW_KEY_F6:
		if(action == GLFW_PRESS) {
			PixelArtRenderer::getInstance()->toggleOptimization();
		}
		break;
/*
	case GLFW_KEY_UP:
		if(action == GLFW_PRESS) {
			PixelArtRenderer::getInstance()->changePositionalPenalty(1);
		}
		break;
	case GLFW_KEY_DOWN:
		if(action == GLFW_PRESS) {
			PixelArtRenderer::getInstance()->changePositionalPenalty(-1);
		}
		break;
*/
/*
	case GLFW_KEY_LEFT:
		if(action == GLFW_PRESS) {
			s_deltaFPS -= 1.0f;
		}
		break;
	case GLFW_KEY_RIGHT:
		if(action == GLFW_PRESS) {
			s_deltaFPS += 1.0f;
		}
		break;
*/
	case GLFW_KEY_SPACE:
		if(action == GLFW_PRESS) {
			s_sequenceIsRunning = !s_sequenceIsRunning;
		}
		break;
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, GL_TRUE);
		break;
	case GLFW_KEY_S:
		if(action == GLFW_PRESS) {
			PixelArtRenderer::getInstance()->saveNextFrame();
		}
		break;
	}
	
}

void PixelArtRenderer::mwfun(GLFWwindow* window, double xoffset, double yoffset) {
	
	if(glfwGetKey(PixelArtRenderer::getInstance()->getWindow(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		s_deltaFPS += float(yoffset);
	} else {
		s_mousewheel_diff -= yoffset;
	}
}

void PixelArtRenderer::calculateZoomFactor() {
	m_zoom_factor += float(s_mousewheel_diff/20.0);
	s_mousewheel_diff = 0.0;
	if(m_zoom_factor > 1.0f) m_zoom_factor = 1.0f;
	if(m_zoom_factor < 0.1f) m_zoom_factor = 0.1f;
}

void PixelArtRenderer::calculateZoomWindowPosition() {
	//calculate shifted zoom window center
	
	if(glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) { 
		if (!m_mouse_grabbingIsActive) {
			m_mouse_grabbingIsActive = true;
			glfwGetCursorPos(m_window,&m_mouse_lastPos_x,&m_mouse_lastPos_y);
		} else {
			double x,y;
			glfwGetCursorPos(m_window,&x,&y);
		
			double zoom_window_translation_x = -0.5*(x - m_mouse_lastPos_x) / window_width;
			double zoom_window_translation_y = 0.5*(y - m_mouse_lastPos_y) / window_heigth;
			m_mouse_lastPos_x = x;
			m_mouse_lastPos_y = y;
			m_zoom_window_position_x += zoom_window_translation_x;
			m_zoom_window_position_y += zoom_window_translation_y;
		}
	} else {
		m_mouse_grabbingIsActive = false;
	}
}

void PixelArtRenderer::smoothZoom(double lastFrameTime) {
		//ody = tdy + (ody-tdy)*exp(-frameTime*springConstant);
		double factor = exp(-lastFrameTime * 6.0);
		m_zoom_factor_smoothed = m_zoom_factor + (m_zoom_factor_smoothed - m_zoom_factor) * factor;
		m_zoom_window_position_x_smoothed = m_zoom_window_position_x + (m_zoom_window_position_x_smoothed - m_zoom_window_position_x) * factor;
		m_zoom_window_position_y_smoothed = m_zoom_window_position_y + (m_zoom_window_position_y_smoothed - m_zoom_window_position_y) * factor;
	};

void PixelArtRenderer::setZoom(double lastFrameTime) {
	calculateZoomFactor();
	calculateZoomWindowPosition();

	//check if window is out of viewport bounds
	float zoom_factor_half = m_zoom_factor * 0.5f;
	if(m_zoom_window_position_x - zoom_factor_half < 0) m_zoom_window_position_x = zoom_factor_half;
	if(m_zoom_window_position_y - zoom_factor_half < 0) m_zoom_window_position_y = zoom_factor_half;
	if(m_zoom_window_position_x + zoom_factor_half > 1) m_zoom_window_position_x = 1.0f - zoom_factor_half;
	if(m_zoom_window_position_y + zoom_factor_half > 1) m_zoom_window_position_y = 1.0f - zoom_factor_half;
	
	//apply zoom
	smoothZoom(lastFrameTime);
	m_gaussRasterizer->setZoom(m_zoom_factor_smoothed, m_zoom_window_position_x_smoothed, m_zoom_window_position_y_smoothed);
	m_simGraphDebugToy->setZoom(m_zoom_factor_smoothed, m_zoom_window_position_x_smoothed, m_zoom_window_position_y_smoothed);
	m_voronoiCellGraph3x3->setZoom(m_zoom_factor_smoothed, m_zoom_window_position_x_smoothed, m_zoom_window_position_y_smoothed);
	m_cellGraphBuilder->setZoom(m_zoom_factor_smoothed, m_zoom_window_position_x_smoothed, m_zoom_window_position_y_smoothed);
	//fprintf(stdout, "ZOOM: %f,%f,%f\n",m_zoom_factor,m_zoom_window_position_x,m_zoom_window_position_y);
}

PixelArtRenderer::PixelArtRenderer() {
	//setup
	m_currentRenderMode = renderMode::GAUSSRASTERIZER;
	m_overlay = false;
	m_opt = true;
	m_isSequence=false;
	m_saveNextFrame = false;
	texUnit_pixelArt=0;
	texUnit_similarityGraph = 1;
	texUnit_updatedSimilarityGraph = 2;
	texUnit_indexedCellPositions = 3;
	texUnit_CellFlags = 4;
	texUnit_knotNeighbors = 5;
	texUnit_optimizedPositions = 6;
	texUnit_correctedPositions = 7;
	DrawBuffers = new GLenum[3];
	DrawBuffers[0] = GL_COLOR_ATTACHMENT0;
	DrawBuffers[1] = GL_COLOR_ATTACHMENT1;
	DrawBuffers[2] = GL_BACK_LEFT;
	aspectRatio = 4.0/3.0;
	//m_lastFrameMWPos = 0;
	m_zoom_factor = 1.0f;
	m_zoom_window_position_x = 0.5;
	m_zoom_window_position_y = 0.5;
	m_zoom_factor_smoothed = 1.0f;
	m_zoom_window_position_x_smoothed = 0.5;
	m_zoom_window_position_y_smoothed = 0.5;
	m_mouse_grabbingIsActive = false;
	m_lastFrameTime = 0.0;
}

PixelArtRenderer::~PixelArtRenderer() {
	delete [] DrawBuffers;
	delete pixelArtImage;
	delete m_similarityGraphBuilder;
	delete m_voronoiCellGraph3x3;
	delete m_cellGraphBuilder;
	delete m_simGraphDebugToy;
	delete m_gaussRasterizer;
}

PixelArtRenderer* PixelArtRenderer::getInstance() {
    if(singletonPointer==0)
		singletonPointer=new PixelArtRenderer();
    return singletonPointer;
}

int PixelArtRenderer::initGraphics() 
{
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RED_BITS, 0);
	glfwWindowHint(GLFW_GREEN_BITS, 0);
	glfwWindowHint(GLFW_BLUE_BITS, 0);
	glfwWindowHint(GLFW_ALPHA_BITS, 0); // try something else maybe?
	glfwWindowHint(GLFW_DEPTH_BITS , 32);
	glfwWindowHint(GLFW_STENCIL_BITS  , 0);

	// Open a window and create its OpenGL context
	m_window = glfwCreateWindow(window_width, window_heigth, "Depixelizing Pixel Art", NULL, NULL);
	if(!m_window) {
        fprintf( stderr, "Failed to open GLFW window.\n" );
        glfwTerminate();
        return 1;
    }
	
	glfwMakeContextCurrent(m_window);
	// Initialize GLEW
	glewExperimental = GL_TRUE;

	glfwSetWindowTitle(m_window, "Depixelizing PixelArt On GPU" );

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(m_window, GLFW_STICKY_KEYS, GL_TRUE );

	resizeFun(m_window, 800,600);
	glfwSetWindowSizeCallback(m_window, &resizeFun);
	glfwSetKeyCallback(m_window, &cbfun);
	glfwSetScrollCallback(m_window, &mwfun);
	glfwGetCursorPos(m_window, &m_mouse_lastPos_x, &m_mouse_lastPos_y);
	// white background
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}
	fprintf(stdout, "GL_RENDERER   = %s\n", (char *)glGetString(GL_RENDERER));
	fprintf(stdout, "GL_VERSION    = %s\n", (char *)glGetString(GL_VERSION));
	fprintf(stdout, "GL_VENDOR     = %s\n", (char *)glGetString(GL_VENDOR));
	if (GLEW_VERSION_3_3)
	{
		fprintf(stdout, "OpenGL 3.3 is supported!\n");
	}
	else
	{
		fprintf(stdout, "OpenGL 3.3 is NOT supported!\n");
		return -1;
	}
	glGetError();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	return 0;
}

bool PixelArtRenderer::initConstructionContent() {
	m_similarityGraphBuilder = new SimilarityGraphBuilderFS(pixelArtImage, DrawBuffers, texUnit_similarityGraph, texUnit_updatedSimilarityGraph);
	m_voronoiCellGraph3x3 = new VoronoiCellGraph3x3(pixelArtImage, DrawBuffers, m_similarityGraphBuilder);
	m_cellGraphBuilder = new CellGraphBuilder(pixelArtImage, DrawBuffers, m_similarityGraphBuilder, texUnit_indexedCellPositions, texUnit_CellFlags, texUnit_knotNeighbors, texUnit_optimizedPositions, texUnit_correctedPositions);
	
	m_simGraphDebugToy = new SimilarityGraphDebugToy(pixelArtImage, m_similarityGraphBuilder, DrawBuffers);
	
	m_gaussRasterizer = new GaussRasterizer(pixelArtImage, DrawBuffers, m_cellGraphBuilder);
	return true;
}

bool PixelArtRenderer::loadPixelArt(const char *filename) {
	bool success;
	pixelArtImage = new Image(filename, texUnit_pixelArt, success);
	aspectRatio = ((float)pixelArtImage->getWidth()) / ((float)pixelArtImage->getHeight());
	resizeFun(m_window, window_width,window_heigth);
	return success;
}

bool PixelArtRenderer::loadPixelArtSequence(const std::string filename, int count, float fps) {
	bool returner = true;
	for (int item = 0; item < count; item++) {
		std::stringstream ss;
		ss << std::setfill('0') << std::setw(4) << item;
		bool success;
		m_img_vector.push_back(new Image((filename + ss.str() + ".png").c_str(), texUnit_pixelArt, success));
		returner = returner && success;
	}
	
	m_isSequence = true;
	pixelArtImage = m_img_vector.front();
	aspectRatio = ((float)pixelArtImage->getWidth()) / ((float)pixelArtImage->getHeight());
	resizeFun(m_window, window_width,window_heigth);
	m_sequence_fps = fps;
	m_sequence_count = count;
	return returner;
}

void PixelArtRenderer::sequenceLoadFrame(double time) {
	m_sequence_fps += s_deltaFPS*0.5f;
	s_deltaFPS = 0.0f;
	if(m_sequence_fps < 1.0f) m_sequence_fps = 1.0f;
	int ID = int(time * m_sequence_fps) % m_sequence_count;
	if(s_sequenceIsRunning) {
		m_similarityGraphBuilder->setPixelArt(m_img_vector.at(ID));
		m_simGraphDebugToy->setPixelArt(m_img_vector.at(ID));
		m_cellGraphBuilder->setPixelArt(m_img_vector.at(ID));
		m_voronoiCellGraph3x3->setPixelArt(m_img_vector.at(ID));
		m_gaussRasterizer->setPixelArt(m_img_vector.at(ID));
	}
	//fprintf(stdout, "m_sequence_fps=%f.\n",m_sequence_fps); 
}

/*
void PixelArtRenderer::sequenceStepFrame() {
	if(s_sequenceIsRunning) {
		m_similarityGraphBuilder->setPixelArt(m_img_deque.front());
		m_simGraphDebugToy->setPixelArt(m_img_deque.front());
		m_cellGraphBuilder->setPixelArt(m_img_deque.front());
		m_voronoiCellGraph3x3->setPixelArt(m_img_deque.front());
		m_gaussRasterizer->setPixelArt(m_img_deque.front());
		m_img_deque.push_back(m_img_deque.front());
		m_img_deque.pop_front();
	}
}
*/
void PixelArtRenderer::drawFrame(double time) {
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	m_similarityGraphBuilder->draw();
	setZoom(time - m_lastFrameTime);
	switch(m_currentRenderMode) {
	case renderMode::SIMILARITYGRAPH:
		m_simGraphDebugToy->drawPixelArt(window_width, window_heigth);
		if (m_overlay) {
			m_simGraphDebugToy->drawSimilarityGraphOverlay(window_width, window_heigth);
		}
		break;
	case renderMode::VORONOIGRAPH3x3:
		m_voronoiCellGraph3x3->draw(window_width, window_heigth);
		if (m_overlay) {
			m_voronoiCellGraph3x3->drawOverlay(window_width, window_heigth);
		}
		break;
	case renderMode::FULLCELLGRAPH:
		if (m_opt) {
			m_cellGraphBuilder->optimize(true);
		} else {
			m_cellGraphBuilder->optimize(false);
		}
		m_cellGraphBuilder->draw();
		m_cellGraphBuilder->drawDebug(window_width, window_heigth);
		/*if(m_overlay){
			m_cellSpaceGrid->draw(window_width, window_heigth);
		}*/
		break;
	case renderMode::GAUSSRASTERIZER:
		if (m_opt) {
			m_cellGraphBuilder->optimize(true);
		} else {
			m_cellGraphBuilder->optimize(false);
		}
		/*if(m_overlay){
			m_cellSpaceGrid->draw(window_width, window_heigth);
		}*/
		m_cellGraphBuilder->draw();
		m_gaussRasterizer->draw(window_width, window_heigth);
		
		break;
	}
	
	// Swap buffers
	if(m_saveNextFrame) {
		saveFrame();
		m_saveNextFrame = false;
	}
	glfwSwapBuffers(m_window);
	glfwPollEvents();
	m_lastFrameTime = time;
}

void PixelArtRenderer::switchRenderMode(renderMode mode) {
	m_currentRenderMode = mode;
}

void PixelArtRenderer::toggleRenderMode() {
	m_currentRenderMode = renderMode((m_currentRenderMode+1)%renderMode::COUNT);
}
void PixelArtRenderer::toggleOverlay() {
	m_overlay = !m_overlay;
}
void PixelArtRenderer::toggleOptimization() {
	m_opt = !m_opt;
}
/*
void PixelArtRenderer::changeOffset(float delta) {
	m_cellGraphBuilder->changeOptimizationOffset(delta);
}

void PixelArtRenderer::changePositionalPenalty(int delta) {
	m_cellGraphBuilder->changePositionalPenalty(delta);
}
*/

void PixelArtRenderer::saveNextFrame() {
	m_saveNextFrame = true;
}

void PixelArtRenderer::saveFrame() {
	// Make the BYTE array, factor of 4 because it's RBGA.
	BYTE* pixels = new BYTE[ 4 * window_width * window_heigth];
	glBindFramebuffer(GL_READ_FRAMEBUFFER,0);
	glReadPixels(0, 0, window_width, window_heigth, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
	FreeImage_Initialise();
	// Convert to FreeImage format & save to file
	FIBITMAP* image = FreeImage_ConvertFromRawBits(pixels, window_width, window_heigth, 4 * window_width, 32,0x0000FF00 , 0x00FF0000, 0xFF000000 , false);
	if (!image)
	{
		fprintf(stderr, "Cannot read bits from framebuffer.\n");
	}
	else 
	{
		if (FreeImage_Save(FIF_PNG, image, "frame.png", PNG_DEFAULT))
		{
			fprintf(stdout, "Frame saved to ./frame.png\n");
		}
		else
		{
			fprintf(stderr, "Could not save frame to ./frame.png\n");
		}
	}
	
	// Free resources
	FreeImage_Unload(image);
	FreeImage_DeInitialise();
	delete [] pixels;
}
