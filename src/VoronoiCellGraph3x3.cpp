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

#include "VoronoiCellGraph3x3.h"
#include "Image.h"
#include "Shader.h"
#include "SimilarityGraphBuilderFS.h"

VoronoiCellGraph3x3::VoronoiCellGraph3x3(Image* pixelArt, GLenum* DrawBuffers, SimilarityGraphBuilderFS* simGraph) {
	m_pixelArtImage = pixelArt;
	m_DrawBuffers = DrawBuffers;
	m_simGraph = simGraph;
	setZoom(1.0f, 0.5f, 0.5f);
	init();
	initOverlay();
}

VoronoiCellGraph3x3::~VoronoiCellGraph3x3() {
	glDeleteBuffers(1, &m_bufferID_dbgDraw_voronoiCellGraph3x3);
	glDeleteProgram(m_programID_debugDrawVoronoiCellGraph3x3);
	glDeleteProgram(m_programID_VoronoiCellOverlay);
}

int VoronoiCellGraph3x3::init() {
	int w = m_pixelArtImage->getWidth();
	int h = m_pixelArtImage->getHeight();
	GLfloat* vertexData_pixelPoints = new GLfloat[(w)*(h)*3];
	for(int y = 0 ; y < (h) ; y++) {
		int x = 0;
		for(int ix = 0 ; ix < 3*(w) ; ix=ix+3) {
			vertexData_pixelPoints[y*(w)*3 + ix] = x; // x-Coordinate
			vertexData_pixelPoints[y*(w)*3 + ix+1] = y; // y-Coordinate
			vertexData_pixelPoints[y*(w)*3 + ix+2] = 0.0f; // z-Coordinate
			x++;
		}
	}
	glGenVertexArrays(1, &m_vaoID_dbgDraw_voronoiCellGraph3x3);
	glBindVertexArray(m_vaoID_dbgDraw_voronoiCellGraph3x3);
		glGenBuffers(1, &m_bufferID_dbgDraw_voronoiCellGraph3x3);
		glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_dbgDraw_voronoiCellGraph3x3);
		glBufferData(GL_ARRAY_BUFFER, (w)*(h)*3*sizeof(GLint), vertexData_pixelPoints, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*3, (const GLvoid*)0 );//vertex positions
		glEnableVertexAttribArray(0);
		//unbind array buffer
		glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindVertexArray(0);
	delete [] vertexData_pixelPoints;

	//compile shaders
	m_programID_debugDrawVoronoiCellGraph3x3 = LoadShaders(PA_SHADER_VS_VORONOI_DRAW, PA_SHADER_GS_VORONOI_DRAW, PA_SHADER_FS_VORONOI_DRAW);
	m_uniformID_debugDrawVoronoiCellGraph3x3_pixelArt = glGetUniformLocation(m_programID_debugDrawVoronoiCellGraph3x3, "pixelArt");
	m_uniformID_debugDrawVoronoiCellGraph3x3_similaryityGraph = glGetUniformLocation(m_programID_debugDrawVoronoiCellGraph3x3, "similarityGraph");
	m_uniformID_debugDrawVoronoiCellGraph3x3_uf_zoomFactor = glGetUniformLocation(m_programID_debugDrawVoronoiCellGraph3x3, "uf_zoomFactor");
	m_uniformID_debugDrawVoronoiCellGraph3x3_uv2_zoomWindowCenter = glGetUniformLocation(m_programID_debugDrawVoronoiCellGraph3x3, "uv2_zoomWindowCenter");	
	return 0;	
}

int VoronoiCellGraph3x3::initOverlay() {
	m_programID_VoronoiCellOverlay = LoadShaders(PA_SHADER_VS_VORONOI_OVERLAY, PA_SHADER_GS_VORONOI_OVERLAY, PA_SHADER_FS_VORONOI_OVERLAY);
	m_uniformID_VoronoiCellOverlay_pixelArt = glGetUniformLocation(m_programID_VoronoiCellOverlay, "pixelArt");
	m_uniformID_VoronoiCellOverlay_similaryityGraph = glGetUniformLocation(m_programID_VoronoiCellOverlay, "similarityGraph");
	m_uniformID_VoronoiCellOverlay_uf_zoomFactor = glGetUniformLocation(m_programID_VoronoiCellOverlay, "uf_zoomFactor");
	m_uniformID_VoronoiCellOverlay_uv2_zoomWindowCenter = glGetUniformLocation(m_programID_VoronoiCellOverlay, "uv2_zoomWindowCenter");	
	return 0;
}

void VoronoiCellGraph3x3::draw(int windowWidth, int WindowHeight) {
	glUseProgram(m_programID_debugDrawVoronoiCellGraph3x3);
	glActiveTexture(GL_TEXTURE0 + m_simGraph->getTexUnit());
	glBindTexture(GL_TEXTURE_2D, m_simGraph->getTexID());
		glUniform1i(m_uniformID_debugDrawVoronoiCellGraph3x3_similaryityGraph, m_simGraph->getTexUnit());
		glActiveTexture(GL_TEXTURE0 + m_pixelArtImage->getTextureUnit());
		glBindTexture(GL_TEXTURE_2D, m_pixelArtImage->getTextureHandle());
		glUniform1i(m_uniformID_debugDrawVoronoiCellGraph3x3_pixelArt, m_pixelArtImage->getTextureUnit());
		glUniform1f(m_uniformID_debugDrawVoronoiCellGraph3x3_uf_zoomFactor, m_zoomFactor_inv);
		glUniform2f(m_uniformID_debugDrawVoronoiCellGraph3x3_uv2_zoomWindowCenter, m_zoomWindow_x, m_zoomWindow_y);
		glBindVertexArray(m_vaoID_dbgDraw_voronoiCellGraph3x3);
			glDrawBuffers(1, &m_DrawBuffers[2]);
			glViewport(0,0,windowWidth, WindowHeight);
			glDrawArrays(GL_POINTS, 0, (m_pixelArtImage->getWidth())*(m_pixelArtImage->getHeight()));
		glBindVertexArray(0);
}

void VoronoiCellGraph3x3::drawOverlay(int windowWidth, int WindowHeight) {
	glUseProgram(m_programID_VoronoiCellOverlay);
		glActiveTexture(GL_TEXTURE0 + m_simGraph->getTexUnit());
		glBindTexture(GL_TEXTURE_2D, m_simGraph->getTexID());
		glUniform1i(m_uniformID_VoronoiCellOverlay_similaryityGraph, m_simGraph->getTexUnit());
		glActiveTexture(GL_TEXTURE0 + m_pixelArtImage->getTextureUnit());
		glBindTexture(GL_TEXTURE_2D, m_pixelArtImage->getTextureHandle());
		glUniform1i(m_uniformID_VoronoiCellOverlay_pixelArt, m_pixelArtImage->getTextureUnit());
		glUniform1f(m_uniformID_VoronoiCellOverlay_uf_zoomFactor, m_zoomFactor_inv);
		glUniform2f(m_uniformID_VoronoiCellOverlay_uv2_zoomWindowCenter, m_zoomWindow_x, m_zoomWindow_y);
		
		glBindVertexArray(m_vaoID_dbgDraw_voronoiCellGraph3x3);
			glDrawBuffers(1, &m_DrawBuffers[2]);
			glViewport(0,0,windowWidth, WindowHeight);
			glDrawArrays(GL_POINTS, 0, (m_pixelArtImage->getWidth())*(m_pixelArtImage->getHeight()));
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D,0);
}

void VoronoiCellGraph3x3::setPixelArt(Image* pixelArt){ 
	m_pixelArtImage = pixelArt;
}

void VoronoiCellGraph3x3::setSimilarityGraph(SimilarityGraphBuilderFS* simGraph) {
	m_simGraph = simGraph;
}