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

#ifndef CELLGRAPHBUILDER_H
#define CELLGRAPHBUILDER_H

/** 
 * @file CellGraphBuilder.h 
 * Header file defining the CellGraphBuilder class.
 */

#include "Drawable.h"

class Image;
class SimilarityGraphBuilderFS;

/**
* This class is used to create a full Cell Graph which represents (optimized) B-Spline control points.
*/
class CellGraphBuilder : public Drawable {
	public:
		
		/**
		* Constructor
		* @param pixelArt the source image
		* @param DrawBuffers This GLenum manages our color attachments.
		* @param simGraph 
		* @param TexUnitCP this texture unit will store the spline controlpoints
		* @param TexUnitFlags this unit will store each control points' flags
		* @param TexUnitNeighbors this unit will store each control points' neighborhood connectivity information
		* @param texUnitOptimized 
		* @param texUnitCorrected 
		*/
		CellGraphBuilder(Image* pixelArt, GLenum* DrawBuffers, SimilarityGraphBuilderFS* simGraph, GLuint TexUnitCP, GLuint TexUnitFlags, GLuint TexUnitNeighbors, GLuint texUnitOptimized, GLuint texUnitCorrected);
		
		/**
		* Destructor.
		* cleans up allocated opengl objects.
		*/
		~CellGraphBuilder();

		/**
		* Use this function to compute the Cell Graph.
		* This will be called if you call the Drawable::Draw(int, int) member.
		* Viewport dimensions are not relevant for Cell Graph construction.
		* The draw method does the following:
		* 1st: Compute spline control points positions aligned to the cell graph using the FullCellGraphConstruction.geom Geometry shader.
		* <BR> This will create 
		* <BR> a controlPoint Position buffer texture,
		* <BR> a controlPoint Flags buffer texture,
		* <BR> a controlPoint Neighborhood buffer texture.
		*/
		void draw();

		void draw(int windowWidth, int WindowHeight){draw();};

		/**
		* Draw debug output representing the B-Spline curves to backbuffer. Note that you need to use draw() first!
		* @param windowWidth the viewport width
		* @param WindowHeight the viewport height
		*/
		void drawDebug(int windowWidth, int WindowHeight);

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

		/**
		* Allows to set the input Pixel Art image.
		* @param pixelArt the source image
		*/
		void setPixelArt(Image* pixelArt);

		/**
		* Tells the CellGraphBuilder wheather it should optimize spline energy.
		* @param value if TRUE control point positions will be optimized.
		*/
		void optimize(bool value);
/*
		void changeOptimizationOffset(float delta);

		void changePositionalPenalty(int delta);
*/
		GLuint getPrimitivesWritten();
		GLuint getPositionsBufferID();
		GLuint getNeighborsBufferID();
		GLuint getFlagsBufferID();
		GLuint getColorIndicesBufferID();

		GLuint getIndexedCellPositionsTextureID();
		GLuint getIndexedCellPositionsTextureUnit();
		GLuint getIndexedCellFlagsTextureID();
		GLuint getIndexedCellFlagsTextureUnit();
		GLuint getIndexedCellNeighborsTextureID();
		GLuint getIndexedCellNeighborsTextureUnit();

	protected:
		int init();
		int initDebug();
		void initOptimizer();
		void initPositionalCorrection();
		void computeOptimizedPositions();
		void computeCorrectedPositions();

	private:

		GLuint m_programID_correctPositions;
		GLuint m_uniformID_correctPositions_indexedCellPositions;
		GLuint m_uniformID_correctPositions_CPflags;
		GLuint m_uniformID_correctPositions_knotNeighbors;
		GLuint m_vaoID_correctPositions;
		GLuint m_bufferID_correctPositionsFeedback;
		GLuint m_texUnit_correctPositions;
		GLuint m_texID_correctPositions;

		GLuint m_programID_optimizeEnergy;
		GLuint m_uniformID_optimizeEnergy_pixelArtDimensions;
		GLuint m_uniformID_optimizeEnergy_indexedCellPositions;
		GLuint m_uniformID_optimizeEnergy_CPflags;
		//GLuint m_uniformID_optimizeEnergy_offsetAmount;
		GLuint m_uniformID_optimizeEnergy_knotNeighbors;
		GLuint m_uniformID_optimizeEnergy_passNum;
		//GLuint m_uniformID_optimizeEnergy_positionalPenalty;

		GLuint m_vaoID_optimizeEnergy_pass1;
		GLuint m_vaoID_optimizeEnergy_pass2;
		
		GLuint m_bufferID_optimizedPositionFeedback;

		GLuint m_texID_optimizedPositions;
		GLuint m_texUnit_optimizedPositions;

		GLuint m_vaoID_fullCellGraphConstruction;
		GLuint m_vaoID_dbgDraw_FullCellGraph;
		
		//Transform Buffer
		GLuint m_bufferID_fullCellGraphConstruction;

		//Feedback Buffers
		GLuint m_bufferID_fullCellGraphConstruction_feedback_pos;
		GLuint m_bufferID_fullCellGraphConstruction_feedback_neighbors;
		GLuint m_bufferID_fullCellGraphConstruction_feedback_flags;
		GLuint m_bufferID_fullCellGraphConstruction_feedback_colorIndicesInterleaved;

		//Shader
		GLuint m_programID_fullCellGraphConstruction;
		GLuint m_uniformID_fullCellGraphConstruction_pixelArt;
		GLuint m_uniformID_fullCellGraphConstruction_similaryityGraph;

		GLuint m_programID_dbgDraw_fullCellGraph;
		GLuint m_uniformID_dbgDraw_fullCellGraph_indexedCellPositions;
		GLuint m_uniformID_dbgDraw_fullCellGraph_pixelArtDimensions;
		GLuint m_uniformID_dbgDraw_fullCellGraph_CPflags;
		GLuint m_uniformID_dbgDraw_fullCellGraph_neighbors;

		GLuint m_texUnit_indexedCellPositions;
		GLuint m_texID_fullCellGraph_indexedCellPositions;

		GLuint m_texUnit_CellFlags;
		GLuint m_texID_fullCellGraph_flags;

		GLuint m_texUnit_knotNeighbors;
		GLuint m_texID_fullCellGraph_knotNeighbors;

		GLuint m_primitives;

		//float m_optimizationOffset;
		//int m_positional_penalty;

		GLenum* m_DrawBuffers;
		bool m_optimize;
		SimilarityGraphBuilderFS* m_simGraph;
		Image* m_pixelArtImage;

		//DEBUG DRAW ZOOM
		GLuint m_uniformID_dbgDraw_fullCellGraph_uf_zoomFactor;
		GLuint m_uniformID_dbgDraw_fullCellGraph_uv2_zoomWindowCenter;
		float m_zoomFactor_inv;
		float m_zoomWindow_x;
		float m_zoomWindow_y;
};

#endif