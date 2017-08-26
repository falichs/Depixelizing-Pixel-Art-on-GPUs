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

#include "GaussRasterizer.h"
#include "Image.h"
#include "Shader.h"
#include "CellGraphBuilder.h"

GaussRasterizer::GaussRasterizer(Image* pixelArt, GLenum* DrawBuffers, CellGraphBuilder* cellGraph) {
	m_pixelArtImage = pixelArt;
	m_DrawBuffers = DrawBuffers;
	m_cellGraph = cellGraph;
	m_zoomFactor = 1.0;
	m_zoomWindow_x = 0.5;
	m_zoomWindow_y = 0.5;
	init();
}

GaussRasterizer::~GaussRasterizer() {
	glDeleteBuffers(1, &m_bufferID_gaussRasterizer_vertices);
	glDeleteProgram(m_programID_gaussRasterizer);
}

int GaussRasterizer::init() {
	//Generate and bind VAO
	glGenVertexArrays(1, &m_vaoID_gaussRasterizer_UVquad);
	glBindVertexArray(m_vaoID_gaussRasterizer_UVquad);

	//Generate, bind and fill a VBO with a triangle strip quad
	//We will use the quad to render the similarityGraph and any other texture (like the original PixelArt)
	const GLfloat g_vertex_buffer_data[] = { 
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f
	};
	glGenBuffers(1, &m_bufferID_gaussRasterizer_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_gaussRasterizer_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
	glVertexAttribPointer(
		0,                  // attribute, must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	glEnableVertexAttribArray(0);

	//unbind array buffer and vao
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindVertexArray(0);

	//Shaders
	m_programID_gaussRasterizer = LoadShaders( PA_SHADER_VS_RASTERIZER, PA_SHADER_FS_RASTERIZER);
	if (!m_programID_gaussRasterizer)
		return -1;
	m_uniformID_gaussRasterizer_pixelArt = glGetUniformLocation(m_programID_gaussRasterizer, "pixelArt");
	m_uniformID_gaussRasterizer_indexedCellPositions = glGetUniformLocation(m_programID_gaussRasterizer, "indexedCellPositions");
	m_uniformID_gaussRasterizer_neighborIndices = glGetUniformLocation(m_programID_gaussRasterizer, "neighborIndices");
	m_uniformID_gaussRasterizer_CPflags = glGetUniformLocation(m_programID_gaussRasterizer, "CPflags");
	m_uniformID_gaussRasterizer_viewportDimensions = glGetUniformLocation(m_programID_gaussRasterizer, "viewportDimensions");
	m_uniformID_gaussRasterizer_uf_zoomFactor = glGetUniformLocation(m_programID_gaussRasterizer, "uf_zoomFactor");
	m_uniformID_gaussRasterizer_uv2_zoomWindowCenter = glGetUniformLocation(m_programID_gaussRasterizer, "uv2_zoomWindowCenter");
	return 0;
}

void GaussRasterizer::setPixelArt(Image* pixelArt) {
	m_pixelArtImage = pixelArt;
}

void GaussRasterizer::setZoom(float zoomFactor, float zoomWindow_x, float zoomWindow_y) {
	m_zoomFactor = zoomFactor;
	m_zoomWindow_x = zoomWindow_x;
	m_zoomWindow_y = zoomWindow_y;
}

void GaussRasterizer::draw(int windowWidth, int WindowHeight) {
	glUseProgram(m_programID_gaussRasterizer);

	glActiveTexture(GL_TEXTURE0 + m_pixelArtImage->getTextureUnit());
	glBindTexture(GL_TEXTURE_2D, m_pixelArtImage->getTextureHandle());
	glUniform1i(m_uniformID_gaussRasterizer_pixelArt, m_pixelArtImage->getTextureUnit());

	glActiveTexture(GL_TEXTURE0 + m_cellGraph->getIndexedCellPositionsTextureUnit());
	glBindTexture(GL_TEXTURE_BUFFER, m_cellGraph->getIndexedCellPositionsTextureID());
	glUniform1i(m_uniformID_gaussRasterizer_indexedCellPositions, m_cellGraph->getIndexedCellPositionsTextureUnit());

	glActiveTexture(GL_TEXTURE0 + m_cellGraph->getIndexedCellNeighborsTextureUnit());
	glBindTexture(GL_TEXTURE_BUFFER, m_cellGraph->getIndexedCellNeighborsTextureID());
	glUniform1i(m_uniformID_gaussRasterizer_neighborIndices, m_cellGraph->getIndexedCellNeighborsTextureUnit());

	glActiveTexture(GL_TEXTURE0 + m_cellGraph->getIndexedCellFlagsTextureUnit());
	glBindTexture(GL_TEXTURE_BUFFER, m_cellGraph->getIndexedCellFlagsTextureID());
	glUniform1i(m_uniformID_gaussRasterizer_CPflags, m_cellGraph->getIndexedCellFlagsTextureUnit());

	glUniform2i(m_uniformID_gaussRasterizer_viewportDimensions, windowWidth, WindowHeight);

	glUniform1f(m_uniformID_gaussRasterizer_uf_zoomFactor, m_zoomFactor);
	glUniform2f(m_uniformID_gaussRasterizer_uv2_zoomWindowCenter, m_zoomWindow_x, m_zoomWindow_y);

	glBindVertexArray(m_vaoID_gaussRasterizer_UVquad);
		glDrawBuffers(1, &m_DrawBuffers[2]);
		glViewport(0,0,windowWidth, WindowHeight);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_BUFFER,0);
	glBindBuffer(GL_ARRAY_BUFFER,0);
}