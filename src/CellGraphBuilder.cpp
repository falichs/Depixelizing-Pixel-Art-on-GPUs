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

#include "CellGraphBuilder.h"
#include "SimilarityGraphBuilderFS.h"
#include "Image.h"
#include "Shader.h"
#include <cstdio>
#include <iostream>

#define TWOPASS 0

CellGraphBuilder::CellGraphBuilder(Image* pixelArt, GLenum* DrawBuffers, SimilarityGraphBuilderFS* simGraph, GLuint TexUnitCP, GLuint TexUnitFlags, GLuint TexUnitNeighbors, GLuint texUnitOptimized, GLuint texUnitCorrected) {
	m_pixelArtImage = pixelArt;
	m_DrawBuffers = DrawBuffers;
	m_simGraph = simGraph;
	m_texUnit_indexedCellPositions = TexUnitCP;
	m_texUnit_CellFlags = TexUnitFlags;
	m_texUnit_knotNeighbors = TexUnitNeighbors;
	m_texUnit_optimizedPositions = texUnitOptimized;
	m_texUnit_correctPositions = texUnitCorrected;
	//m_optimizationOffset = 0.1;
	m_optimize = true;
	//m_positional_penalty = 2;
	init();
	initOptimizer();
	initPositionalCorrection();
	initDebug();
	setZoom(1.0f, 0.5f, 0.5f);
}

CellGraphBuilder::~CellGraphBuilder() {
	glDeleteBuffers(1, &m_bufferID_fullCellGraphConstruction);
	glDeleteBuffers(1, &m_bufferID_fullCellGraphConstruction_feedback_pos);
	glDeleteBuffers(1, &m_bufferID_fullCellGraphConstruction_feedback_neighbors);
	glDeleteBuffers(1, &m_bufferID_fullCellGraphConstruction_feedback_flags);
	glDeleteBuffers(1, &m_bufferID_fullCellGraphConstruction_feedback_colorIndicesInterleaved);
	glDeleteTextures(1, &m_texID_fullCellGraph_indexedCellPositions);
	glDeleteTextures(1, &m_texID_fullCellGraph_flags);
	glDeleteTextures(1, &m_texID_fullCellGraph_knotNeighbors);
	glDeleteProgram(m_programID_fullCellGraphConstruction);
	glDeleteProgram(m_programID_dbgDraw_fullCellGraph);

	glDeleteProgram(m_programID_optimizeEnergy);
	glDeleteBuffers(1, &m_bufferID_optimizedPositionFeedback);
	glDeleteTextures(1, &m_texID_optimizedPositions);

	glDeleteProgram(m_programID_correctPositions);

}

int CellGraphBuilder::init() {
	//SETUP VERTEX INPUT VAO
	int w = m_pixelArtImage->getWidth();
	int h = m_pixelArtImage->getHeight();
	GLfloat* vertexData_pixelPoints = new GLfloat[(w-1)*(h-1)*3];
	for(int y = 0 ; y < (h-1) ; y++) {
		int x = 0;
		for(int ix = 0 ; ix < 3*(w-1) ; ix=ix+3) {
			vertexData_pixelPoints[y*(w-1)*3 + ix] = x; // x-Coordinate
			vertexData_pixelPoints[y*(w-1)*3 + ix+1] = y; // y-Coordinate
			vertexData_pixelPoints[y*(w-1)*3 + ix+2] = 0.0f; // z-Coordinate
			x++;
		}
	}
	glGenVertexArrays(1, &m_vaoID_fullCellGraphConstruction);
	glBindVertexArray(m_vaoID_fullCellGraphConstruction);
		glGenBuffers(1, &m_bufferID_fullCellGraphConstruction);
		glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_fullCellGraphConstruction);
		glBufferData(GL_ARRAY_BUFFER, (w-1)*(h-1)*3*sizeof(GLint), vertexData_pixelPoints, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*3, (const GLvoid*)0 );//vertex positions
		glEnableVertexAttribArray(0);
		//unbind array buffer
		glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindVertexArray(0);
	delete [] vertexData_pixelPoints;

	//SETUP SHADER
	m_programID_fullCellGraphConstruction = LoadCellGraphShader(PA_SHADER_VS_CELLGRAPH, PA_SHADER_GS_CELLGRAPH);

	m_uniformID_fullCellGraphConstruction_pixelArt = glGetUniformLocation(m_programID_fullCellGraphConstruction, "pixelArt");
	m_uniformID_fullCellGraphConstruction_similaryityGraph = glGetUniformLocation(m_programID_fullCellGraphConstruction, "similarityGraph");

	//SETUP FEEDBACK BUFFERS
	glGenBuffers(1, &m_bufferID_fullCellGraphConstruction_feedback_pos);
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_fullCellGraphConstruction_feedback_pos);
	glBufferData(GL_ARRAY_BUFFER, (w-1)*(h-1)*2*2*sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m_bufferID_fullCellGraphConstruction_feedback_neighbors);
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_fullCellGraphConstruction_feedback_neighbors);
	glBufferData(GL_ARRAY_BUFFER, (w-1)*(h-1)*4*2*sizeof(GLint), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m_bufferID_fullCellGraphConstruction_feedback_flags);
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_fullCellGraphConstruction_feedback_flags);
	glBufferData(GL_ARRAY_BUFFER, (w-1)*(h-1)*2*sizeof(GLint), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m_bufferID_fullCellGraphConstruction_feedback_colorIndicesInterleaved);
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_fullCellGraphConstruction_feedback_colorIndicesInterleaved);
	glBufferData(GL_ARRAY_BUFFER, (w-1)*(h-1)*2*16*sizeof(GLint), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Textures
	glGenTextures(1,&m_texID_fullCellGraph_indexedCellPositions);
	glActiveTexture(GL_TEXTURE0 + m_texUnit_indexedCellPositions);
	glBindBuffer(GL_TEXTURE_BUFFER, m_bufferID_fullCellGraphConstruction_feedback_pos);
	glBindTexture(GL_TEXTURE_BUFFER,m_texID_fullCellGraph_indexedCellPositions);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RG32F , m_bufferID_fullCellGraphConstruction_feedback_pos);
	glTexParameteri(GL_TEXTURE_BUFFER, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_BUFFER, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glGenTextures(1,&m_texID_fullCellGraph_flags);
	glActiveTexture(GL_TEXTURE0 + m_texUnit_CellFlags);
	glBindBuffer(GL_TEXTURE_BUFFER, m_bufferID_fullCellGraphConstruction_feedback_flags);
	glBindTexture(GL_TEXTURE_BUFFER, m_texID_fullCellGraph_flags);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I , m_bufferID_fullCellGraphConstruction_feedback_flags);
	glTexParameteri(GL_TEXTURE_BUFFER, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_BUFFER, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glGenTextures(1,&m_texID_fullCellGraph_knotNeighbors);
	glActiveTexture(GL_TEXTURE0 + m_texUnit_knotNeighbors);
	glBindBuffer(GL_TEXTURE_BUFFER, m_bufferID_fullCellGraphConstruction_feedback_neighbors);
	glBindTexture(GL_TEXTURE_BUFFER, m_texID_fullCellGraph_knotNeighbors);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32I , m_bufferID_fullCellGraphConstruction_feedback_neighbors);
	glTexParameteri(GL_TEXTURE_BUFFER, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_BUFFER, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	return 0;
}

void CellGraphBuilder::draw() {
	//BUILD THE CELL GRAPH
	GLuint Query1;
	glGenQueries(1, &Query1);

	glEnable(GL_RASTERIZER_DISCARD);
	glUseProgram(m_programID_fullCellGraphConstruction);
		glActiveTexture(GL_TEXTURE0 + m_simGraph->getTexUnit());
			glBindTexture(GL_TEXTURE_2D, m_simGraph->getTexID());
			glUniform1i(m_uniformID_fullCellGraphConstruction_similaryityGraph, m_simGraph->getTexUnit());
		glActiveTexture(GL_TEXTURE0 + m_pixelArtImage->getTextureUnit());
			glBindTexture(GL_TEXTURE_2D, m_pixelArtImage->getTextureHandle());
			glUniform1i(m_uniformID_fullCellGraphConstruction_pixelArt, m_pixelArtImage->getTextureUnit());

	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_bufferID_fullCellGraphConstruction_feedback_pos);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, m_bufferID_fullCellGraphConstruction_feedback_neighbors);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, m_bufferID_fullCellGraphConstruction_feedback_flags);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 3, m_bufferID_fullCellGraphConstruction_feedback_colorIndicesInterleaved);

	glBindVertexArray(m_vaoID_fullCellGraphConstruction);
		glBeginQueryIndexed(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, 0, Query1);
			glBeginTransformFeedback(GL_POINTS);
				glDrawArrays(GL_POINTS, 0, (m_pixelArtImage->getWidth()-1)*(m_pixelArtImage->getHeight()-1));
			glEndTransformFeedback();
		glEndQueryIndexed(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN,0);
		//glEndQueryIndexed(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN,1);
	//glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	glBindVertexArray(0);
	glDisable(GL_RASTERIZER_DISCARD);
	m_primitives = 0;
	glGetQueryObjectuiv(Query1, GL_QUERY_RESULT, &m_primitives);

	//OPTIMIZE ENERGY OF CELL VERTICES
	if(m_optimize) {
		computeOptimizedPositions();
		computeCorrectedPositions();
	}
}

void CellGraphBuilder::initOptimizer() {
	m_programID_optimizeEnergy = LoadOptimizationShader(PA_SHADER_VS_OPTIMIZE_ENERGY);
	m_uniformID_optimizeEnergy_pixelArtDimensions = glGetUniformLocation(m_programID_optimizeEnergy,"pixelArtDimensions");
	m_uniformID_optimizeEnergy_indexedCellPositions = glGetUniformLocation(m_programID_optimizeEnergy,"indexedCellPositions");
	m_uniformID_optimizeEnergy_CPflags = glGetUniformLocation(m_programID_optimizeEnergy,"CPflags");
	//m_uniformID_optimizeEnergy_offsetAmount = glGetUniformLocation(m_programID_optimizeEnergy,"offsetAmount");
	m_uniformID_optimizeEnergy_knotNeighbors = glGetUniformLocation(m_programID_optimizeEnergy,"knotNeighbors");
	m_uniformID_optimizeEnergy_passNum = glGetUniformLocation(m_programID_optimizeEnergy,"passNum");
	//m_uniformID_optimizeEnergy_positionalPenalty = glGetUniformLocation(m_programID_optimizeEnergy,"positionalPenalty");
	//Buffers

	glGenBuffers(1,&m_bufferID_optimizedPositionFeedback);
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_optimizedPositionFeedback);
	glBufferData(GL_ARRAY_BUFFER, (m_pixelArtImage->getWidth()-1)*(m_pixelArtImage->getHeight()-1)*2*2*sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//VAOs
	glGenVertexArrays(1,&m_vaoID_optimizeEnergy_pass1);
	glBindVertexArray(m_vaoID_optimizeEnergy_pass1);
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_fullCellGraphConstruction_feedback_pos);
	glVertexAttribPointer(0, 2, GL_FLOAT,GL_FALSE, 0, (void*)0 );
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_fullCellGraphConstruction_feedback_neighbors);
	glVertexAttribIPointer(1, 4, GL_INT, 0, (void*)0 );
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_fullCellGraphConstruction_feedback_flags);
	glVertexAttribIPointer(2, 1, GL_INT, 0, (void*)0 );
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindVertexArray(0);

	glGenVertexArrays(1,&m_vaoID_optimizeEnergy_pass2);
	glBindVertexArray(m_vaoID_optimizeEnergy_pass2);
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_optimizedPositionFeedback);
	glVertexAttribPointer(0, 2, GL_FLOAT,GL_FALSE, 0, (void*)0 );
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_fullCellGraphConstruction_feedback_neighbors);
	glVertexAttribIPointer(1, 4, GL_INT, 0, (void*)0 );
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_fullCellGraphConstruction_feedback_flags);
	glVertexAttribIPointer(2, 1, GL_INT, 0, (void*)0 );
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindVertexArray(0);

	glGenTextures(1,&m_texID_optimizedPositions);
	glActiveTexture(GL_TEXTURE0 + m_texUnit_optimizedPositions);
	glBindBuffer(GL_TEXTURE_BUFFER, m_bufferID_optimizedPositionFeedback);
	glBindTexture(GL_TEXTURE_BUFFER, m_texID_optimizedPositions);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RG32F , m_bufferID_optimizedPositionFeedback);

}

void CellGraphBuilder::initPositionalCorrection() {
	m_programID_correctPositions = LoadOptimizationShader(PA_SHADER_VS_UPDATE_POSITIONS);
	m_uniformID_correctPositions_indexedCellPositions = glGetUniformLocation(m_programID_correctPositions,"indexedCellPositions");
	m_uniformID_correctPositions_CPflags = glGetUniformLocation(m_programID_correctPositions,"CPflags");
	m_uniformID_correctPositions_knotNeighbors = glGetUniformLocation(m_programID_correctPositions,"knotNeighbors");
	//Buffers

	glGenBuffers(1,&m_bufferID_correctPositionsFeedback);
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_correctPositionsFeedback);
	glBufferData(GL_ARRAY_BUFFER, (m_pixelArtImage->getWidth()-1)*(m_pixelArtImage->getHeight()-1)*2*2*sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenTextures(1,&m_texID_correctPositions);
	glActiveTexture(GL_TEXTURE0 + m_texUnit_correctPositions);
	glBindBuffer(GL_TEXTURE_BUFFER, m_bufferID_correctPositionsFeedback);
	glBindTexture(GL_TEXTURE_BUFFER, m_texID_correctPositions);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RG32F , m_bufferID_correctPositionsFeedback);

	//VAOs
	glGenVertexArrays(1,&m_vaoID_correctPositions);
	glBindVertexArray(m_vaoID_correctPositions);
#ifdef TWOPASS
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_fullCellGraphConstruction_feedback_pos);
#else
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_optimizedPositionFeedback);
#endif
	glVertexAttribPointer(0, 2, GL_FLOAT,GL_FALSE, 0, (void*)0 );
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_fullCellGraphConstruction_feedback_flags);
	glVertexAttribIPointer(1, 1, GL_INT, 0, (void*)0 );
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindVertexArray(0);
}

void CellGraphBuilder::computeCorrectedPositions() {
	glEnable(GL_RASTERIZER_DISCARD);
	glUseProgram(m_programID_correctPositions);
	glActiveTexture(GL_TEXTURE0 + m_texUnit_indexedCellPositions);
		glBindTexture(GL_TEXTURE_BUFFER, m_texID_fullCellGraph_indexedCellPositions);
		glUniform1i(m_uniformID_correctPositions_indexedCellPositions, m_texUnit_indexedCellPositions);
		
	glActiveTexture(GL_TEXTURE0 + m_texUnit_CellFlags);
		glBindTexture(GL_TEXTURE_BUFFER, m_texID_fullCellGraph_flags);
		glUniform1i(m_uniformID_correctPositions_CPflags, m_texUnit_CellFlags);

	glActiveTexture(GL_TEXTURE0 + m_texUnit_knotNeighbors);
		glBindTexture(GL_TEXTURE_BUFFER, m_texID_fullCellGraph_knotNeighbors);
		glUniform1i(m_uniformID_correctPositions_knotNeighbors, m_texUnit_knotNeighbors);

	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_bufferID_correctPositionsFeedback);
		glBindVertexArray(m_vaoID_correctPositions);
		glBeginTransformFeedback(GL_POINTS);
		glDrawArrays(GL_POINTS, 0, m_primitives);
		glEndTransformFeedback();

	glBindVertexArray(0);

	glDisable(GL_RASTERIZER_DISCARD);
	glBindTexture(GL_TEXTURE_BUFFER,0);
}

void CellGraphBuilder::computeOptimizedPositions() {
	glEnable(GL_RASTERIZER_DISCARD);

	glUseProgram(m_programID_optimizeEnergy);

		glActiveTexture(GL_TEXTURE0 + m_texUnit_indexedCellPositions);
			glBindTexture(GL_TEXTURE_BUFFER, m_texID_fullCellGraph_indexedCellPositions);
			glUniform1i(m_uniformID_optimizeEnergy_indexedCellPositions, m_texUnit_indexedCellPositions);
		
		glActiveTexture(GL_TEXTURE0 + m_texUnit_CellFlags);
			glBindTexture(GL_TEXTURE_BUFFER, m_texID_fullCellGraph_flags);
			glUniform1i(m_uniformID_optimizeEnergy_CPflags, m_texUnit_CellFlags);

		glActiveTexture(GL_TEXTURE0 + m_texUnit_knotNeighbors);
			glBindTexture(GL_TEXTURE_BUFFER, m_texID_fullCellGraph_knotNeighbors);
			glUniform1i(m_uniformID_optimizeEnergy_knotNeighbors, m_texUnit_knotNeighbors);

		glUniform2i(m_uniformID_optimizeEnergy_pixelArtDimensions,m_pixelArtImage->getWidth(),m_pixelArtImage->getHeight());
		//glUniform1f(m_uniformID_optimizeEnergy_offsetAmount, m_optimizationOffset);
		//glUniform1i(m_uniformID_optimizeEnergy_positionalPenalty, m_positional_penalty);
		//first pass
		glUniform1i(m_uniformID_optimizeEnergy_passNum, 0);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_bufferID_optimizedPositionFeedback);
		glBindVertexArray(m_vaoID_optimizeEnergy_pass1);
		glBeginTransformFeedback(GL_POINTS);
		glDrawArrays(GL_POINTS, 0, m_primitives);
		glEndTransformFeedback();

#ifdef TWOPASS

		glActiveTexture(GL_TEXTURE0 + m_texUnit_optimizedPositions);
			glBindTexture(GL_TEXTURE_BUFFER, m_texID_optimizedPositions);
			glUniform1i(m_uniformID_optimizeEnergy_indexedCellPositions, m_texUnit_optimizedPositions);


		//second pass
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_bufferID_fullCellGraphConstruction_feedback_pos);
		glUniform1i(m_uniformID_optimizeEnergy_passNum, 1);
		glBindVertexArray(m_vaoID_optimizeEnergy_pass2);
		glBeginTransformFeedback(GL_POINTS);
		glDrawArrays(GL_POINTS, 0, m_primitives);
		glEndTransformFeedback();

#endif
		/*
		glUniform1f(m_uniformID_optimizeEnergy_offsetAmount, m_optimizationOffset*0.5);

		//third pass
		glActiveTexture(GL_TEXTURE0 + m_texUnit_indexedCellPositions);
			glBindTexture(GL_TEXTURE_BUFFER, m_texID_fullCellGraph_indexedCellPositions);
			glUniform1i(m_uniformID_optimizeEnergy_indexedCellPositions, m_texUnit_indexedCellPositions);

		glUniform1i(m_uniformID_optimizeEnergy_passNum, 0);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_bufferID_optimizedPositionFeedback);
		glBindVertexArray(m_vaoID_optimizeEnergy_pass1);
		glBeginTransformFeedback(GL_POINTS);
		glDrawArrays(GL_POINTS, 0, m_primitives);
		glEndTransformFeedback();
		
		//fourth pass
		glActiveTexture(GL_TEXTURE0 + m_texUnit_optimizedPositions);
			glBindTexture(GL_TEXTURE_BUFFER, m_texID_optimizedPositions);
			glUniform1i(m_uniformID_optimizeEnergy_indexedCellPositions, m_texUnit_optimizedPositions);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_bufferID_fullCellGraphConstruction_feedback_pos);
		glUniform1i(m_uniformID_optimizeEnergy_passNum, 1);
		glBindVertexArray(m_vaoID_optimizeEnergy_pass2);
		glBeginTransformFeedback(GL_POINTS);
		glDrawArrays(GL_POINTS, 0, m_primitives);
		glEndTransformFeedback();
		*/

	glBindVertexArray(0);

	glDisable(GL_RASTERIZER_DISCARD);
	glBindTexture(GL_TEXTURE_BUFFER,0);
}

int CellGraphBuilder::initDebug() {
	//shader
	m_programID_dbgDraw_fullCellGraph = LoadShaders(PA_SHADER_VS_CELLGRAPH_DRAW, PA_SHADER_GS_CELLGRAPH_DRAW, PA_SHADER_FS_CELLGRAPH_DRAW);
	if (!m_programID_dbgDraw_fullCellGraph)
		return -1;

	//uniforms
	m_uniformID_dbgDraw_fullCellGraph_indexedCellPositions = glGetUniformLocation(m_programID_dbgDraw_fullCellGraph, "indexedCellPositions");
	m_uniformID_dbgDraw_fullCellGraph_pixelArtDimensions = glGetUniformLocation(m_programID_dbgDraw_fullCellGraph, "pixelArtDimensions");
	m_uniformID_dbgDraw_fullCellGraph_CPflags = glGetUniformLocation(m_programID_dbgDraw_fullCellGraph, "CPflags");
	m_uniformID_dbgDraw_fullCellGraph_neighbors = glGetUniformLocation(m_programID_dbgDraw_fullCellGraph, "knotNeighbors");
	m_uniformID_dbgDraw_fullCellGraph_uf_zoomFactor = glGetUniformLocation(m_programID_dbgDraw_fullCellGraph, "uf_zoomFactor");
	m_uniformID_dbgDraw_fullCellGraph_uv2_zoomWindowCenter = glGetUniformLocation(m_programID_dbgDraw_fullCellGraph, "uv2_zoomWindowCenter");	

	//VAO
	glGenVertexArrays(1,&m_vaoID_dbgDraw_FullCellGraph);
	glBindVertexArray(m_vaoID_dbgDraw_FullCellGraph);
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_fullCellGraphConstruction_feedback_pos);
	glVertexAttribPointer(0, 2, GL_FLOAT,GL_FALSE, 0, (void*)0 );
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_fullCellGraphConstruction_feedback_neighbors);
	glVertexAttribIPointer(1, 4, GL_INT, 0, (void*)0 );
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_fullCellGraphConstruction_feedback_flags);
	glVertexAttribIPointer(2, 1, GL_INT, 0, (void*)0 );
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindVertexArray(0);
	
	return 0;
}

void CellGraphBuilder::drawDebug(int windowWidth, int WindowHeight) {
	if(m_optimize) {
		glBindVertexArray(m_vaoID_dbgDraw_FullCellGraph);
		glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_correctPositionsFeedback);
		glVertexAttribPointer(0, 2, GL_FLOAT,GL_FALSE, 0, (void*)0 );
		glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_fullCellGraphConstruction_feedback_neighbors);
		glVertexAttribIPointer(1, 4, GL_INT, 0, (void*)0 );
		glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_fullCellGraphConstruction_feedback_flags);
		glVertexAttribIPointer(2, 1, GL_INT, 0, (void*)0 );
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER,0);
		glBindVertexArray(0);

		glUseProgram(m_programID_dbgDraw_fullCellGraph);
		glActiveTexture(GL_TEXTURE0 + m_texUnit_correctPositions);
		glBindTexture(GL_TEXTURE_BUFFER, m_texID_correctPositions);
		glUniform1i(m_uniformID_dbgDraw_fullCellGraph_indexedCellPositions, m_texUnit_correctPositions);

	} else {

		glBindVertexArray(m_vaoID_dbgDraw_FullCellGraph);
		glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_fullCellGraphConstruction_feedback_pos);
		glVertexAttribPointer(0, 2, GL_FLOAT,GL_FALSE, 0, (void*)0 );
		glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_fullCellGraphConstruction_feedback_neighbors);
		glVertexAttribIPointer(1, 4, GL_INT, 0, (void*)0 );
		glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_fullCellGraphConstruction_feedback_flags);
		glVertexAttribIPointer(2, 1, GL_INT, 0, (void*)0 );
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER,0);
		glBindVertexArray(0);

		glUseProgram(m_programID_dbgDraw_fullCellGraph);
		glActiveTexture(GL_TEXTURE0 + m_texUnit_indexedCellPositions);
		glBindTexture(GL_TEXTURE_BUFFER, m_texID_fullCellGraph_indexedCellPositions);
		glUniform1i(m_uniformID_dbgDraw_fullCellGraph_indexedCellPositions, m_texUnit_indexedCellPositions);
	}
	

	glActiveTexture(GL_TEXTURE0 + m_texUnit_CellFlags);
	glBindTexture(GL_TEXTURE_BUFFER, m_texID_fullCellGraph_flags);
	glUniform1i(m_uniformID_dbgDraw_fullCellGraph_CPflags, m_texUnit_CellFlags);
	
	glActiveTexture(GL_TEXTURE0 + m_texUnit_knotNeighbors);
	glBindTexture(GL_TEXTURE_BUFFER, m_texID_fullCellGraph_knotNeighbors);
	glUniform1i(m_uniformID_dbgDraw_fullCellGraph_neighbors, m_texUnit_knotNeighbors);

	glUniform2i(m_uniformID_dbgDraw_fullCellGraph_pixelArtDimensions,m_pixelArtImage->getWidth(),m_pixelArtImage->getHeight());
	glUniform1f(m_uniformID_dbgDraw_fullCellGraph_uf_zoomFactor, m_zoomFactor_inv);
	glUniform2f(m_uniformID_dbgDraw_fullCellGraph_uv2_zoomWindowCenter, m_zoomWindow_x, m_zoomWindow_y);

	glBindVertexArray(m_vaoID_dbgDraw_FullCellGraph);
		glLineWidth(4.0);
		glDrawBuffers(1, &m_DrawBuffers[2]);
		glViewport(0,0,windowWidth, WindowHeight);
		glDrawArrays(GL_POINTS, 0, m_primitives);
		glLineWidth(1.0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_BUFFER,0);
	glBindBuffer(GL_TEXTURE_BUFFER,0);
	glBindBuffer(GL_ARRAY_BUFFER,0);
}

void CellGraphBuilder::optimize(bool value) {
	m_optimize = value;
}

GLuint CellGraphBuilder::getPrimitivesWritten() {
	return m_primitives;
}
GLuint CellGraphBuilder::getPositionsBufferID(){
	if(m_optimize) {
		return m_bufferID_correctPositionsFeedback;
	}
	return m_bufferID_fullCellGraphConstruction_feedback_pos;
}
GLuint CellGraphBuilder::getNeighborsBufferID(){
	return m_bufferID_fullCellGraphConstruction_feedback_neighbors;
}
GLuint CellGraphBuilder::getFlagsBufferID(){
	return m_bufferID_fullCellGraphConstruction_feedback_flags;
}
GLuint CellGraphBuilder::getColorIndicesBufferID(){
	return m_bufferID_fullCellGraphConstruction_feedback_colorIndicesInterleaved;
}

GLuint CellGraphBuilder::getIndexedCellPositionsTextureID(){
	if(m_optimize) {
		return m_texID_correctPositions;
	}
	return m_texID_fullCellGraph_indexedCellPositions;
}
GLuint CellGraphBuilder::getIndexedCellPositionsTextureUnit(){
	if(m_optimize) {
		return m_texUnit_correctPositions;
	}
	return m_texUnit_indexedCellPositions;
}
GLuint CellGraphBuilder::getIndexedCellFlagsTextureID(){
	return m_texID_fullCellGraph_flags;
}
GLuint CellGraphBuilder::getIndexedCellFlagsTextureUnit(){
	return m_texUnit_CellFlags;
}
GLuint CellGraphBuilder::getIndexedCellNeighborsTextureID(){
	return m_texID_fullCellGraph_knotNeighbors;
}
GLuint CellGraphBuilder::getIndexedCellNeighborsTextureUnit(){
	return m_texUnit_knotNeighbors;
}
/*
void CellGraphBuilder::changeOptimizationOffset(float delta) {
	float value = m_optimizationOffset + delta;
	if(value > 0.01 && value < 0.5) {
		m_optimizationOffset = value;
	}
}

void CellGraphBuilder::changePositionalPenalty(int delta){
	int value = m_positional_penalty + delta;
	//if(value > 0 && value < 17) {
		m_positional_penalty = value;
	//}
	std::cout << "penalty = "<< m_positional_penalty <<".\n";
}
*/
void CellGraphBuilder::setPixelArt(Image* pixelArt) {
	m_pixelArtImage = pixelArt;
}