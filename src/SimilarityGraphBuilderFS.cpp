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

#include "SimilarityGraphBuilderFS.h"
#include "Shader.h"
#include "Image.h"
#include <cstdio>

SimilarityGraphBuilderFS::SimilarityGraphBuilderFS(Image* pixelArt, GLenum* DrawBuffers, GLuint TextureUnit1, GLuint TextureUnit2) {
	this->m_pixelArtImage = pixelArt;
	this->m_DrawBuffers = DrawBuffers;
	this->m_texUnit_similarityGraph = TextureUnit1;
	this->m_texUnit_updatedSimilarityGraph = TextureUnit2;

	this->init();
}

SimilarityGraphBuilderFS::~SimilarityGraphBuilderFS() {
	glDeleteBuffers(1, &m_bufferID_similarityGraph_vertices);
	glDeleteBuffers(1, &m_bufferID_similarityGraph_UVs);
	

	glDeleteProgram(m_programID_indifferentColors);
	glDeleteProgram(m_programID_valenceUpdate);
	glDeleteProgram(m_programID_eliminateCrossingDiagonals);
	

	glDeleteTextures(1, &m_texID_similarityGraph);
	glDeleteTextures(1, &m_texID_updatedSimilarityGraph);

	glDeleteFramebuffers(1, &m_fbo1);
}

int SimilarityGraphBuilderFS::init() {

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

	//Setup Textures
	glGenTextures(1, &m_texID_similarityGraph);
	glActiveTexture(GL_TEXTURE0 + m_texUnit_similarityGraph);
	glBindTexture(GL_TEXTURE_2D, m_texID_similarityGraph);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB,  2*m_pixelArtImage->getWidth()+1,2*m_pixelArtImage->getHeight()+1, 0,GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glGenTextures(1, &m_texID_updatedSimilarityGraph);
	glActiveTexture(GL_TEXTURE0 + m_texUnit_updatedSimilarityGraph);
	glBindTexture(GL_TEXTURE_2D, m_texID_updatedSimilarityGraph);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB,  2*m_pixelArtImage->getWidth()+1,2*m_pixelArtImage->getHeight()+1, 0,GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	// the wrapping settings are needed for the crossing diagonals shader
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	//Shaders

	//similarityGraph construction and update
	m_programID_indifferentColors = LoadShaders(PA_SHADER_VS_QUAD, PA_SHADER_FS_DISSIMILAR);
	m_programID_valenceUpdate = LoadShaders(PA_SHADER_VS_QUAD, PA_SHADER_FS_UPDATE_VALENCE);
	m_programID_eliminateCrossingDiagonals = LoadShaders(PA_SHADER_VS_QUAD, PA_SHADER_FS_ELIMINATE_CROSSINGS);
		
	m_uniformID_indifferentColors_pixelArt = glGetUniformLocation(m_programID_indifferentColors, "pixelArt");
	m_uniformID_valenceUpdate_similarityGraph = glGetUniformLocation(m_programID_valenceUpdate, "similarityGraph");
	m_uniformID_eliminateCrossingDiagonals_similarityGraph = glGetUniformLocation(m_programID_eliminateCrossingDiagonals, "similarityGraph");
		

	//setup FBO
	m_fbo1 = 0;
	glGenFramebuffers(1, &m_fbo1);

	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo1);
	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_texID_similarityGraph, 0);
	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_texID_updatedSimilarityGraph, 0);

	// check that our framebuffer is ok
	if(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		return -1;
	}
	else return 0;

}

void SimilarityGraphBuilderFS::draw() {
	//===================================
	//Constructing the SimilarityGraph
	//===================================
	
	//1st PASS: Calculate SimilarityGraph
	//===================================
	glUseProgram(m_programID_indifferentColors);
	// pixelArt sampler
	glActiveTexture(GL_TEXTURE0 + m_pixelArtImage->getTextureUnit());
	glBindTexture(GL_TEXTURE_2D, m_pixelArtImage->getTextureHandle());
	glUniform1i(m_uniformID_indifferentColors_pixelArt, m_pixelArtImage->getTextureUnit());
	glBindVertexArray(m_vaoID_similarityGraph_UVquad);
	// Render to our framebuffers color attachment 0, which holds the texture texID_similarityGraph
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo1);
	glDrawBuffers(1, &m_DrawBuffers[0] );
	glViewport(0,0, 2 * m_pixelArtImage->getWidth() + 1, 2 * m_pixelArtImage->getHeight() + 1);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
	//2nd PASS: Update Valences
	//===================================
	glUseProgram(m_programID_valenceUpdate);
	//setup the previously rendered Similarity-graph as sampler for the shader	
	glActiveTexture(GL_TEXTURE0 + m_texUnit_similarityGraph);
	glBindTexture(GL_TEXTURE_2D, m_texID_similarityGraph);
	glUniform1i(m_uniformID_valenceUpdate_similarityGraph, m_texUnit_similarityGraph);
	// Render to our framebuffers color attachment 1, which holds the texture texID_updatedSimilarityGraph
	glDrawBuffers(1, &m_DrawBuffers[1]);
	glViewport(0,0, 2 * m_pixelArtImage->getWidth() + 1, 2 * m_pixelArtImage->getHeight() + 1);
	// Draw !
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
	//3rd PASS: Eliminate Crossing Diagonals
	//======================================
	glUseProgram(m_programID_eliminateCrossingDiagonals);
	//setup the previously rendered Updated Similarity-graph as sampler for the shader
	glActiveTexture(GL_TEXTURE0 + m_texUnit_updatedSimilarityGraph);
	glBindTexture(GL_TEXTURE_2D, m_texID_updatedSimilarityGraph);
	glUniform1i(m_uniformID_eliminateCrossingDiagonals_similarityGraph, m_texUnit_updatedSimilarityGraph);
	// Render to our framebuffers color attachment 0, which holds the texture texID_similarityGraph
	glDrawBuffers(1, &m_DrawBuffers[0]);
	glViewport(0,0, 2 * m_pixelArtImage->getWidth() + 1, 2 * m_pixelArtImage->getHeight() + 1);
	// Draw !
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
	//4th PASS: Update Valences
	//======================================
	
	glUseProgram(m_programID_valenceUpdate);
	glActiveTexture(GL_TEXTURE0 + m_texUnit_similarityGraph);
	glBindTexture(GL_TEXTURE_2D, m_texID_similarityGraph);
	glUniform1i(m_uniformID_valenceUpdate_similarityGraph, m_texUnit_similarityGraph);
	glDrawBuffers(1, &m_DrawBuffers[1]);
	glViewport(0,0, 2 * m_pixelArtImage->getWidth() + 1, 2 * m_pixelArtImage->getHeight() + 1);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	//unbind vao
	glBindVertexArray(0);
}

GLuint SimilarityGraphBuilderFS::getTexID() {
	return m_texID_updatedSimilarityGraph;
}

GLuint SimilarityGraphBuilderFS::getTexUnit() {
	return m_texUnit_updatedSimilarityGraph;
}

void SimilarityGraphBuilderFS::setPixelArt(Image* pixelArt) {
	m_pixelArtImage = pixelArt;
}