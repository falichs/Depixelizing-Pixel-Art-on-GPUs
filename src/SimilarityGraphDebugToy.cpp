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

#include "SimilarityGraphDebugToy.h"
#include "SimilarityGraphBuilder.h"
#include "Shader.h"
#include "Image.h"

SimilarityGraphDebugToy::SimilarityGraphDebugToy(Image* pixelArt, SimilarityGraphBuilder* simGraph, GLenum* DrawBuffers){
	m_simGraph = simGraph;
	m_DrawBuffers = DrawBuffers;
	m_pixelArtImage = pixelArt;
	m_zoomFactor = 1.0f;
	m_zoomFactor_inv = 1.0f;
	m_zoomWindow_x = 0.5f;
	m_zoomWindow_y = 0.5f;
	init();
}

SimilarityGraphDebugToy::~SimilarityGraphDebugToy() {
	glDeleteBuffers(1, &m_bufferID_dbgDraw_similarityGraph);
	glDeleteBuffers(1, &m_bufferID_similarityGraph_vertices);
	glDeleteBuffers(1, &m_bufferID_similarityGraph_UVs);
	glDeleteProgram(m_programID_debugDrawPixels);
	glDeleteProgram(m_programID_debugDrawNodes);
	glDeleteProgram(m_programID_debugDrawEdges);
}

void SimilarityGraphDebugToy::setPixelArt(Image* pixelArt) {
	m_pixelArtImage = pixelArt;
}

int SimilarityGraphDebugToy::init() {
	
	// Generate, bind and fill a VBO with WxH points with interval 1.0
	// this one will be used for rendering the similarityGraph
	int w = m_pixelArtImage->getWidth();
	int h = m_pixelArtImage->getHeight();
	GLfloat* vertexData_pixelPoints = new GLfloat[w*h*3];
	for(int iy = 0 ; iy < h ; iy++) {
		for(int ix = 0 ; ix < 3*w ; ix=ix+3) {
			vertexData_pixelPoints[iy*w*3 + ix] = (GLfloat)ix/3;
			vertexData_pixelPoints[iy*w*3 + ix+1] = (GLfloat)iy;
			vertexData_pixelPoints[iy*w*3 + ix+2] = 0.0f;
		}
	}
	glGenVertexArrays(1, &m_vaoID_dbgDraw_similarityGraph);
	glBindVertexArray(m_vaoID_dbgDraw_similarityGraph);
		glGenBuffers(1, &m_bufferID_dbgDraw_similarityGraph);
		glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_dbgDraw_similarityGraph);
		glBufferData(GL_ARRAY_BUFFER, w*h*12, vertexData_pixelPoints, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );
		glEnableVertexAttribArray(0);

		//unbind array buffer
		glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindVertexArray(0);

	delete [] vertexData_pixelPoints;

	//Shaders
	m_programID_debugDrawPixels = LoadShaders(PA_SHADER_VS_QUAD, PA_SHADER_FS_PIXELS );
	m_uniformID_debugDrawPixels_pixelArt = glGetUniformLocation(m_programID_debugDrawPixels, "pixelArt");
	m_uniformID_debugDrawPixels_uf_zoomFactor = glGetUniformLocation(m_programID_debugDrawPixels, "uf_zoomFactor");
	m_uniformID_debugDrawPixels_uv2_zoomWindowCenter = glGetUniformLocation(m_programID_debugDrawPixels, "uv2_zoomWindowCenter");

	m_programID_debugDrawEdges = LoadShaders(PA_SHADER_VS_SIMGRAPH_DRAW, PA_SHADER_GS_SIMGRAPH_DRAW, PA_SHADER_FS_SIMGRAPH_DRAW);
	m_uniformID_debugDrawEdges_similarityGraph = glGetUniformLocation(m_programID_debugDrawEdges, "similarityGraph");
	m_uniformID_debugDrawEdges_uf_zoomFactor = glGetUniformLocation(m_programID_debugDrawEdges, "uf_zoomFactor");
	m_uniformID_debugDrawEdges_uv2_zoomWindowCenter = glGetUniformLocation(m_programID_debugDrawEdges, "uv2_zoomWindowCenter");

	m_programID_debugDrawNodes = LoadShaders(PA_SHADER_VS_SIMGRAPH_DRAW_NODES, PA_SHADER_FS_SIMGRAPH_DRAW);
	m_uniformID_debugDrawNodes_positionsRange = glGetUniformLocation(m_programID_debugDrawNodes, "positionsRange");
	m_uniformID_debugDrawNodes_uf_zoomFactor = glGetUniformLocation(m_programID_debugDrawNodes, "uf_zoomFactor");
	m_uniformID_debugDrawNodes_uv2_zoomWindowCenter = glGetUniformLocation(m_programID_debugDrawNodes, "uv2_zoomWindowCenter");

	//Generate and bind VAO
	glGenVertexArrays(1, &m_vaoID_similarityGraph_UVquad);
	glBindVertexArray(m_vaoID_similarityGraph_UVquad);

	//Generate, bind and fill a VBO with a triangle strip quad
	//We will use the quad to render the similarityGraph and any other texture (like the original PixelArt)
	const GLfloat g_vertex_buffer_data[] = { 
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f
	};
	glGenBuffers(1, &m_bufferID_similarityGraph_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_similarityGraph_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
	glVertexAttribPointer(
		0,                  // attribute, must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);
	//Generate, bind and fill a VBO with UV Data
	const GLfloat g_uv_buffer_data[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f
	};
	glGenBuffers(1, &m_bufferID_similarityGraph_UVs);
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferID_similarityGraph_UVs);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);
	glVertexAttribPointer(
		1,                                // attribute, must match the layout in the shader.
		2,                                // size : U+V => 2
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	//unbind array buffer and vao
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindVertexArray(0);

	return 0;
}

void SimilarityGraphDebugToy::drawPixelArt(int windowWidth, int WindowHeight) {
	//draw the pixelArt
	glUseProgram(m_programID_debugDrawPixels);
	glActiveTexture(GL_TEXTURE0 + m_pixelArtImage->getTextureUnit());
	glBindTexture(GL_TEXTURE_2D, m_pixelArtImage->getTextureHandle());
	glUniform1i(m_uniformID_debugDrawPixels_pixelArt, m_pixelArtImage->getTextureUnit());
	glUniform1f(m_uniformID_debugDrawPixels_uf_zoomFactor, m_zoomFactor);
	glUniform2f(m_uniformID_debugDrawPixels_uv2_zoomWindowCenter, m_zoomWindow_x, m_zoomWindow_y);

	glBindVertexArray(m_vaoID_similarityGraph_UVquad); //we can reuse this vao here
		glDrawBuffers(1, &m_DrawBuffers[2]);
		glViewport(0,0,windowWidth, WindowHeight);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void SimilarityGraphDebugToy::drawSimilarityGraphOverlay(int windowWidth, int WindowHeight) {
	drawPixelCenters(windowWidth, WindowHeight);
	drawPixelConnections(windowWidth, WindowHeight);
}

void SimilarityGraphDebugToy::drawPixelCenters(int windowWidth, int WindowHeight) {
	glPointSize(4.0f);
	glUseProgram(m_programID_debugDrawNodes);
		glUniform2i(m_uniformID_debugDrawNodes_positionsRange, m_pixelArtImage->getWidth(), m_pixelArtImage->getHeight());
		glUniform1f(m_uniformID_debugDrawNodes_uf_zoomFactor, m_zoomFactor_inv);
		glUniform2f(m_uniformID_debugDrawNodes_uv2_zoomWindowCenter,  -(m_zoomWindow_x-0.5f)*2.0f*m_zoomFactor_inv, -(m_zoomWindow_y-0.5f)*2.0f*m_zoomFactor_inv);
		
		glBindVertexArray(m_vaoID_dbgDraw_similarityGraph);
			glDrawBuffers(1, &m_DrawBuffers[2]);
			glViewport(0,0,windowWidth, WindowHeight);
			glDrawArrays(GL_POINTS, 0, m_pixelArtImage->getWidth()*m_pixelArtImage->getHeight());
		glBindVertexArray(0);
}

void SimilarityGraphDebugToy::drawPixelConnections(int windowWidth, int WindowHeight) {
	glUseProgram(m_programID_debugDrawEdges);
		glActiveTexture(GL_TEXTURE0 + m_simGraph->getTexUnit());
		glBindTexture(GL_TEXTURE_2D, m_simGraph->getTexID());
		glUniform1i(m_uniformID_debugDrawEdges_similarityGraph, m_simGraph->getTexUnit());
		glUniform1f(m_uniformID_debugDrawEdges_uf_zoomFactor, m_zoomFactor_inv);
		glUniform2f(m_uniformID_debugDrawEdges_uv2_zoomWindowCenter, -(m_zoomWindow_x-0.5f)*2.0f*m_zoomFactor_inv, -(m_zoomWindow_y-0.5f)*2.0f*m_zoomFactor_inv);
		glBindVertexArray(m_vaoID_dbgDraw_similarityGraph);
			glDrawBuffers(1, &m_DrawBuffers[2]);
			glViewport(0,0,windowWidth, WindowHeight);
			glDrawArrays(GL_POINTS, 0, m_pixelArtImage->getWidth()*m_pixelArtImage->getHeight());
		glBindVertexArray(0);
}
